#include "Assignment04.hh"

#include <cstdint>
#include <vector>

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/camera/SmoothedCamera.hh>

// ImGui
#include <imgui/imgui.h>

// GLFW
#include <GLFW/glfw3.h>

// extra helper
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/timing/CpuTimer.hh>

#include <typed-geometry/tg-std.hh>


// in the implementation, we want to omit the glow:: prefix
using namespace glow;


void Assignment04::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    // update rigid body
    {
        // accumulate forces
        mRigidBody.clearForces();
        if (length(mThrusterDir) > 0.0f)
        {
            auto thrusterWorldPos = mRigidBody.pointLocalToGlobal(mThrusterPos);
            auto thrusterWorldDir = mRigidBody.directionLocalToGlobal(mThrusterDir);
            auto force = -thrusterWorldDir * mThrusterStrength;
            mRigidBody.addForce(force, thrusterWorldPos);
        }

        // update RB
        mRigidBody.update(elapsedSeconds);

        // perform collision checks
        mRigidBody.checkPlaneCollision({0, 0, 0}, {0, 1, 0}); // against ground plane

        // fix center of mass
        if (mFixCenterOfMass)
        {
            mRigidBody.linearMomentum = {0, 0, 0};
            if (mRigidBody.preset == Preset::RolyPolyToy)
                mRigidBody.linearPosition = {0, 1.415f, 0};
            else
                mRigidBody.linearPosition = {0, 3, 0};
        }
    }
}

void Assignment04::render(float elapsedSeconds)
{
    (void)elapsedSeconds;

    // Functions called with this macro are undone at the end of the scope
    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);

    // draw shadow map
    {
        auto fb = mFramebufferShadow->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        // render scene from light
        renderScene(true);
    }

    // draw scene
    {
        // clear the screen
        glClearColor(0.3f, 0.3f, 0.3f, 1.00f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // render scene normally
        renderScene(false);
    }
}

void Assignment04::onGui()
{
    if (ImGui::Begin("RTG Rigid Body Simulation"))
    {
        if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Pendulum"))
                loadPreset(Preset::Pendulum);
            if (ImGui::Button("Roly-poly toy"))
                loadPreset(Preset::RolyPolyToy);
        }

        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Gravity", &mRigidBody.gravity, -100, 100);
            ImGui::SliderFloat("Impulse strength", &mImpulseStrength, -100, 100);
            ImGui::SliderFloat("Thruster strength", &mThrusterStrength, -100, 100);
            ImGui::Checkbox("Fix center of mass", &mFixCenterOfMass);
        }

        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Show object frame", &mShowObjectFrame);
            ImGui::Checkbox("Show ray hit", &mShowRayHit);
        }
    }
    ImGui::End();
}

void Assignment04::renderScene(bool shadowPass)
{
    // Get camera matrices
    tg::mat4 view = getCamera()->getViewMatrix();
    tg::mat4 proj = getCamera()->getProjectionMatrix();

    tg::mat4 shadowview = tg::look_at(mLightPos, tg::pos3::zero, tg::vec3::unit_y);
    tg::mat4 shadowproj = tg::perspective_opengl(70_deg, 1.0f, 10.0f, 50.0f);
    if (shadowPass)
    {
        view = shadowview;
        proj = shadowproj;
    }

    // configure line shader
    {
        auto shader = mShaderLine->use();
        shader.setUniform("uViewMatrix", view);
        shader.setUniform("uProjectionMatrix", proj);
    }

    // draw objects
    {
        auto shader = mShaderObj->use();
        shader.setUniform("uViewMatrix", view);
        shader.setUniform("uProjectionMatrix", proj);

        // set up lighting
        {
            shader.setUniform("uLightPos", mLightPos);
            shader.setUniform("uCamPos", shadowPass ? mLightPos : getCamera()->getPosition());

            shader.setTexture("uShadowMap", mTextureShadow);
            shader.setUniform("uShadowViewProj", shadowproj * shadowview);
        }

        // draw plane
        {
            shader.setUniform("uModelMatrix", tg::mat4::identity);
            shader.setUniform("uColor", tg::color3(0.00f, 0.33f, 0.66f));
            mMeshPlane->bind().draw();
        }

        // draw rigid body
        {
            // solid color
            shader.setUniform("uColor", tg::color3(0.66f, 0.00f, 0.33f));

/// Task 1.b
/// Draw the rigid body
///
/// Your job is to:
///     - compute the model matrix of the box or sphere
///     - pass it to the shader as uniform
///     - draw the appropriate VertexArray
///
/// Notes:
///     - a rigid body can consist of several shapes with one common
///       transformation matrix (rbTransform)
///     - furthermore, every shape is transformed by its own matrix (s->transform)
///       and some scaling that is defined by the radius (sphere) or halfExtent (box)
///     - the needed VertexArrays are stored in mMeshSphere and mMeshCube
///
/// ============= STUDENT CODE BEGIN =============

            for (auto const& s : mRigidBody.shapes)
            {
                auto sphere = std::dynamic_pointer_cast<SphereShape>(s);
                auto box = std::dynamic_pointer_cast<BoxShape>(s);

                // draw spheres
                if (sphere)
                {
                    // TODO
                }

                // draw boxes
                if (box)
                {
                    // TODO
                }
            }

/// ============= STUDENT CODE END =============
        }
    }

    // draw lines
    {
        switch (mRigidBody.preset)
        {
        case Preset::Pendulum:
            // white line from rigid body to pendulum
            drawLine(mRigidBody.linearPosition, mRigidBody.pendulumPosition, {1, 1, 1});
            break;

        default: // nothing
            break;
        }

        // debug rendering
        if (!shadowPass)
        {
            GLOW_SCOPED(disable, GL_DEPTH_TEST); // draw above everything

            if (mShowObjectFrame)
            {
                auto pc = mRigidBody.pointLocalToGlobal({0, 0, 0});
                auto px = mRigidBody.pointLocalToGlobal({1, 0, 0});
                auto py = mRigidBody.pointLocalToGlobal({0, 1, 0});
                auto pz = mRigidBody.pointLocalToGlobal({0, 0, 1});
                drawLine(pc, px, {1, 0, 0});
                drawLine(pc, py, {0, 1, 0});
                drawLine(pc, pz, {0, 0, 1});
            }

            if (mShowRayHit)
            {
                tg::pos3 pos;
                tg::vec3 dir;
                getMouseRay(pos, dir);

                tg::pos3 hit;
                tg::vec3 normal;
                if (mRigidBody.rayCast(pos, dir, hit, normal))
                    drawLine(hit, hit + normal, {1, 0, 1});
            }

            // draw thruster
            if (length(mThrusterDir) > 0.0f)
            {
                auto pos = mRigidBody.pointLocalToGlobal(mThrusterPos);
                auto dir = mRigidBody.directionLocalToGlobal(mThrusterDir);
                drawLine(pos, pos + dir, {1, 1, 0});
            }
        }
    }
}

bool Assignment04::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // calculate mouse ray
        tg::pos3 pos;
        tg::vec3 dir;
        getMouseRay(pos, dir);

        tg::pos3 hit;
        tg::vec3 normal;
        if (mRigidBody.rayCast(pos, dir, hit, normal))
        {
            // impact or thruster
            if (mods & GLFW_MOD_SHIFT) // shift + [LMB] -> thruster
            {
                mThrusterDir = mRigidBody.directionGlobalToLocal(normal);
                mThrusterPos = mRigidBody.pointGlobalToLocal(hit);
            }
            else // [LMB] -> impact
            {
                auto impulse = mImpulseStrength * dir;
                mRigidBody.applyImpulse(impulse, hit);
            }
        }
        else
        {
            // clear thruster
            if (mods & GLFW_MOD_SHIFT)
                mThrusterDir = tg::vec3();
        }
    }

    return false;
}

void Assignment04::loadPreset(Preset preset)
{
    mThrusterDir = {0, 0, 0};
    mRigidBody.loadPreset(preset);
    mRigidBody.calculateMassAndInertia();
}


/// Task 1.a
/// Build the line mesh
/// Draw the line mesh
///
/// Your job is to:
///     - build a line mesh for one single line
///     - draw the line mesh (i.e. drawing the VertexArray after binding it)
///
/// Notes:
///     - create an ArrayBuffer, define the attribute(s), set the data, create a VertexArray
///     - store the VertexArray to the member variable mMeshLine
///     - the primitive type is GL_LINES
///     - for the drawing, you have to set some uniforms for mShaderLine
///     - uViewMatrix and uProjectionMatrix are automatically set
///
///
/// ============= STUDENT CODE BEGIN =============

void Assignment04::buildLineMesh()
{
    // TODO
}

void Assignment04::drawLine(tg::pos3 from, tg::pos3 to, tg::color3 color)
{
    // TODO
}

/// ============= STUDENT CODE END =============

void Assignment04::getMouseRay(tg::pos3& pos, tg::vec3& dir)
{
    auto mp = getMousePosition();
    auto x = mp.x;
    auto y = mp.y;

    auto cam = getCamera();
    tg::vec3 ps[2];
    auto i = 0;
    for (auto d : {0.5f, -0.5f})
    {
        tg::vec4 v{x / float(getWindowWidth()) * 2 - 1, 1 - y / float(getWindowHeight()) * 2, d * 2 - 1, 1.0};

        v = tg::inverse(cam->getProjectionMatrix()) * v;
        v /= v.w;
        v = tg::inverse(cam->getViewMatrix()) * v;
        ps[i++] = tg::vec3(v);
    }

    pos = cam->getPosition();
    dir = normalize(ps[0] - ps[1]);
}

void Assignment04::init()
{
    setGui(GlfwApp::Gui::ImGui);

    // limit GPU to 60 fps
    setVSync(true);

    GlfwApp::init(); // Call to base GlfwApp

    setTitle("RTG Assignment04 - Rigid Body Simulation");

    // set up camera
    auto cam = getCamera();
    cam->setPosition({12, 12, 12});
    cam->handle.setTarget({0, 0, 0});

    // load shaders
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shader/obj");
    mShaderLine = Program::createFromFile(util::pathOf(__FILE__) + "/shader/line");

    // shadow map
    mTextureShadow = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32F, 1);
    mTextureShadow->bind().setMinFilter(GL_LINEAR);
    mFramebufferShadow = Framebuffer::createDepthOnly(mTextureShadow); // depth-only

    // create geometry
    mMeshSphere = geometry::UVSphere<>().generate();
    mMeshCube = geometry::Cube<>().generate();
    mMeshPlane = geometry::Quad<geometry::CubeVertex>().generate([](float u, float v) {
        return geometry::CubeVertex{tg::pos3((u * 2 - 1) * 1000, 0, (v * 2 - 1) * 1000), //
                                    tg::vec3(0, 1, 0),                                   //
                                    tg::vec3(1, 0, 0),                                   //
                                    tg::pos2(u, v)};
    });
    buildLineMesh();

    // init scene
    loadPreset(Preset::Pendulum);
    // loadPreset(Preset::RolyPolyToy);
}
