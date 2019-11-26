#pragma once

#include <vector>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "RigidBody.hh"
#include "Shapes.hh"

#include <typed-geometry/tg-lean.hh>

/**
 * Assignment04: A rigid body simulation without collisions
 */
class Assignment04 : public glow::glfw::GlfwApp
{
private: // input and camera
    tg::pos3 mLightPos = {-10, 30, 5};

private: // scene
    RigidBody mRigidBody;

    float mImpulseStrength = 10.0f;
    float mThrusterStrength = 10.0f;

    // thruster is in rigidbody-local space
    tg::pos3 mThrusterPos;
    tg::vec3 mThrusterDir;

    // if true, we fix the center of mass
    bool mFixCenterOfMass = false;

private: // graphics
    glow::SharedVertexArray mMeshSphere;
    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mMeshPlane;
    glow::SharedVertexArray mMeshLine;

    glow::SharedProgram mShaderObj;
    glow::SharedProgram mShaderLine;

    const int mShadowMapSize = 2048;
    glow::SharedTexture2D mTextureShadow;
    glow::SharedFramebuffer mFramebufferShadow;

    bool mShowObjectFrame = !false;
    bool mShowRayHit = !false;

private: // helper
    void buildLineMesh();
    void drawLine(tg::pos3 from, tg::pos3 to, tg::color3 color);

    void getMouseRay(tg::pos3& pos, tg::vec3& dir);

public:
    void loadPreset(Preset preset);

    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;
    void onGui() override;

    void renderScene(bool shadowPass);

    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
};
