#include "Assignment03.hh"

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/camera/SmoothedCamera.hh>

// imgui
#include <imgui/imgui.h>

// GLFW
#include <GLFW/glfw3.h>

// extra helper
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/timing/CpuTimer.hh>

#include <cstdint>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

void Assignment03::update(float elapsedSeconds)
{
    mAccumSeconds += elapsedSeconds;

    // update sphere
    if (mSphereMoving)
    {
        mSpherePos = tg::pos3(mSphereMovingRadius * tg::cos(tg::radians(mAccumSeconds)), //
                              -1 - mSphereRadius,                                        //
                              mSphereMovingRadius * tg::sin(tg::radians(mAccumSeconds)));
    }
    else
    {
        mSpherePos = {0, -1 - mSphereRadius, 0};
    }

    static auto fixedParticles = mCloth.fixedParticles;
    if (mCloth.fixedParticles != fixedParticles)
    {
        fixedParticles = mCloth.fixedParticles;
        mCloth.resetMotionState();
    }

    // update cloth
    mCloth.resetForces();
    mCloth.updateForces();
    mCloth.updateMotion(elapsedSeconds);
    mCloth.addSphereCollision(mSpherePos, mSphereRadius);
}

void Assignment03::render(float elapsedSeconds)
{
    // Functions called with this macro are undone at the end of the scope
    GLOW_SCOPED(enable, GL_DEPTH_TEST);

    // clear the screen
    glClearColor(0.30f, 0.3f, 0.3f, 1.00f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Get camera matrices
    auto view = getCamera()->getViewMatrix();
    auto proj = getCamera()->getProjectionMatrix();

    // set up shader
    auto shader = mShaderObj->use();
    shader.setUniform("uViewMatrix", view);
    shader.setUniform("uProjectionMatrix", proj);
    shader.setUniform("uLightPos", mLightPos);
    shader.setUniform("uCamPos", getCamera()->getPosition());
    shader.setUniform("uShowNormals", mCloth.coloring == Coloring::Normals);

    // draw sphere
    {
        GLOW_SCOPED(wireframe, mWireframeSphere);
        auto modelMatrix = tg::translation(mSpherePos) * tg::scaling(tg::size3(mSphereRadius - mSphereOffset));
        shader.setUniform("uModelMatrix", modelMatrix);
        shader.setUniform("uTranslucency", 0.0f);

        mSphere->bind().draw();
    }

    // draw cloth
    {
        GLOW_SCOPED(wireframe, mWireframeCloth);
        tg::mat4 modelMatrix = tg::mat4::identity;
        shader.setUniform("uModelMatrix", modelMatrix);
        shader.setUniform("uTranslucency", 0.3f);

        mCloth.createVAO()->bind().draw();
    }
    (void)elapsedSeconds;
}

void Assignment03::onGui()
{
    if (ImGui::Begin("RTG Cloth Simulation"))
    {
        if (ImGui::CollapsingHeader("Cloth Rendering", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::RadioButton("Unicolored", mCloth.coloring == Coloring::Unicolored))
                mCloth.coloring = Coloring::Unicolored;
            if (ImGui::RadioButton("Checker", mCloth.coloring == Coloring::Checker))
                mCloth.coloring = Coloring::Checker;
            if (ImGui::RadioButton("Stress", mCloth.coloring == Coloring::Stress))
                mCloth.coloring = Coloring::Stress;
            if (ImGui::RadioButton("Normals", mCloth.coloring == Coloring::Normals))
                mCloth.coloring = Coloring::Normals;
            ImGui::Checkbox("Smooth Normals", &mCloth.smoothShading);
            ImGui::PushID("wireframecloth"); // needed to use same caption twice
            ImGui::Checkbox("Render Wireframe", &mWireframeCloth);
            ImGui::PopID();
        }
        ImGui::NewLine();
        ImGui::NewLine();
        if (ImGui::CollapsingHeader("Cloth Simulation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Reset Cloth"))
                mCloth.resetMotionState();

            if (ImGui::RadioButton("One Side", mCloth.fixedParticles == FixedParticles::OneSide))
                mCloth.fixedParticles = FixedParticles::OneSide;
            if (ImGui::RadioButton("Two Sides", mCloth.fixedParticles == FixedParticles::TwoSides))
                mCloth.fixedParticles = FixedParticles::TwoSides;
            if (ImGui::RadioButton("Four Sides", mCloth.fixedParticles == FixedParticles::FourSides))
                mCloth.fixedParticles = FixedParticles::FourSides;
            if (ImGui::RadioButton("Four Corners", mCloth.fixedParticles == FixedParticles::FourCorners))
                mCloth.fixedParticles = FixedParticles::FourCorners;

            ImGui::SliderFloat("k (stiffness)", &mCloth.springK, 0.01f, 10.0f);
            ImGui::SliderFloat("d (damping)", &mCloth.dampingD, 0.0f, 0.99f);
            ImGui::SliderFloat("g (gravity)", &mCloth.gravity, -100.0f, 100.0f);
        }
        ImGui::NewLine();
        ImGui::NewLine();
        if (ImGui::CollapsingHeader("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Radius", &mSphereRadius, 0.2f, 5.0f);
            ImGui::Checkbox("Movement", &mSphereMoving);
            ImGui::Checkbox("Render Wireframe", &mWireframeSphere);
        }
    }
    ImGui::End();
}

bool Assignment03::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    return onMousePosition(x, y);
}

bool Assignment03::onMousePosition(double x, double y)
{
    if (isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        isMousePressed = true;

        auto cam = getCamera();
        tg::vec3 ps[2];
        auto i = 0;
        for (auto d : {0.5f, -0.5f})
        {
            tg::vec4 v{(float)x / float(getWindowWidth()) * 2 - 1, 1 - (float)y / float(getWindowHeight()) * 2, d * 2 - 1, 1.0f};

            v = tg::inverse(cam->getProjectionMatrix()) * v;
            v /= v.w;
            v = tg::inverse(cam->getViewMatrix()) * v;
            ps[i++] = tg::vec3(v);
        }

        auto pos = cam->getPosition();
        auto dir = normalize(ps[0] - ps[1]);

        bool dragged = mCloth.drag(pos, dir, cam->handle.getForward());
        GlfwApp::setUseDefaultCameraHandling(!dragged);

        return true;
    }
    else if (isMousePressed)
    {
        isMousePressed = false;
        mCloth.releaseDrag();
        GlfwApp::setUseDefaultCameraHandling(true);
        return true;
    }

    return GlfwApp::onMousePosition(x, y);
}

void Assignment03::init()
{
    setGui(GlfwApp::Gui::ImGui);

    GlfwApp::init(); // Call to base GlfwApp

    setTitle("RTG Assignment03 - Cloth Simulation");

    // set up camera
    auto cam = getCamera();
    cam->setPosition({2, 2, 2});
    cam->handle.setTarget({0, 0, 0});

    // load resources
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shaderObj");
    mSphere = geometry::UVSphere<Vertex>().generate([](tg::pos3 position, tg::vec3 normal, tg::vec3 tangent, tg::pos2 texCoord) {
        (void)tangent;
        tg::vec3 sphereCol0 = {0.66f, 0.00, 0.33f};
        tg::vec3 sphereCol1 = sphereCol0;

        return Vertex{position, //
                      normal,   //
                      (int(std::fmod(6.0f * texCoord.x, 1.0f) > 0.5f) % 2) ? sphereCol0 : sphereCol1};
    });

    // create cloth object
    mCloth = Cloth(30, 10.0f, 10);
}
