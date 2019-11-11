#pragma once

#include <vector>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "Cloth.hh"
/**
 * Assignment03: A cloth simulation assignment to cover the deformable bodies topic (in two dimensions)
 */
class Assignment03 : public glow::glfw::GlfwApp
{
private: // input and camera
    tg::vec3 mLightPos = {-1, 10, 5};

    bool isMousePressed = false;

private: // scene
    float mAccumSeconds = 0;

    Cloth mCloth;

    tg::pos3 mSpherePos;
    float mSphereRadius = 2.0f;
    bool mSphereMoving = false;
    float mSphereMovingRadius = 2.5f;
    bool mWireframeCloth = false;
    bool mWireframeSphere = false;

    // Small rendering bias to prevent sphere from piercing through cloth
    const float mSphereOffset = 0.1f;

private: // graphics
    glow::SharedVertexArray mPlane;
    glow::SharedVertexArray mSphere;

    glow::SharedProgram mShaderObj;

    // bool mShowNormals = false;

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;
    void onGui() override;

    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;
};
