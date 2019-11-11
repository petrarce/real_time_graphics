#pragma once

#include <glow/common/shared.hh>
#include <glow/fwd.hh>
#include <typed-geometry/tg-lean.hh>

namespace glow::viewer
{
struct CameraSettings
{
    float MoveSpeedStart = 50 * 1000.0;
    float MoveSpeedMin = 1;
    float MoveSpeedMax = 10000000.0f;
    float MoveSpeedFactor = 2.0f;

    tg::angle NumpadRotateDegree = 30_deg;

    // smoothing halftimes in ms
    float TranslationalSmoothingHalfTime = 40;
    float RotationalSmoothingHalfTime = 30;

    float FocusRefDistance = 30 * 1000;

    float DefaultTargetDistance = 60 * 1000.;

    float TargetMinDistance = 1000;
    float ZoomFactor = 1.3f;

    tg::angle HorizontalSensitivity = 360_deg;
    tg::angle VerticalSensitivity = 360_deg;
    bool InvertHorizontal = false;
    bool InvertVertical = false;

    tg::angle HorizontalFov = 90_deg;
    float NearPlane = 0.001f;
};

/**
 *
 * The CameraController is a smoothed camera with multiple transformation layers:
 *
 * - a global rotation for smoothing the orbital camera
 * - a target position/distance for target lookaroud camera
 *
 * NOTE: from outside, all camera properties are computed in global space and the smoothed versions are reported
 *
 * in FPS mode, pos is state and target pos computed. In targeted mode, it's the other way around
 *
 * NOTE: float/float mixups are because of ImGui
 */
GLOW_SHARED(class, CameraController);
class CameraController
{
    enum class Mode
    {
        Targeted, // or Orbital
        FPS       // WASD + right mouse
    };

    // "external" state
private:
    int mWindowWidth = 1;
    int mWindowHeight = 1;
    float mMeshSize = 1;
    tg::pos3 mMeshCenter;

    // camera view state
private:
    tg::pos3 mPos;             ///< in "target space", after applying planet rotation
    tg::pos3 mTargetPos;       ///< in "target space", after applying planet rotation
    float mTargetDistance = 1; ///< distance 0 means fps cam

    tg::vec3 mRight;
    tg::vec3 mFwd;
    tg::vec3 mUp;
    tg::vec3 mRefUp = tg::vec3::unit_y;

    tg::angle mAltitude;
    tg::angle mAzimuth;

    // smoothed view state
private:
    tg::pos3 mSmoothedPos;
    tg::pos3 mSmoothedTargetPos;
    float mSmoothedTargetDistance = 1;

    tg::vec3 mSmoothedRight;
    tg::vec3 mSmoothedFwd;
    tg::vec3 mSmoothedUp;

    // input settings
public:
    CameraSettings s;

    // input state
private:
    Mode mMode = Mode::Targeted;
    float mMoveSpeed = 0;

    // getter
public:
    tg::pos3 getPosition() const;
    tg::vec3 getForwardDir() const { return mSmoothedFwd; }
    tg::vec3 getUpDir() const { return mSmoothedUp; }
    tg::vec3 getRightDir() const { return mSmoothedRight; }
    float getTargetDistance() const { return mSmoothedTargetDistance; }
    tg::pos3 getTargetPos() const { return mSmoothedTargetPos; }
    float getNearPlane() const { return s.NearPlane; }
    tg::comp<2, tg::angle> getSphericalCoordinates() const { return {mAzimuth, mAltitude}; }

    // queries
public:
    tg::mat4 computeViewMatrix() const;
    tg::mat4 computeProjMatrix() const;

    // behaviors / input methods
public:
    CameraController();

    void clipCamera();
    void resetView();

    void focusOnSelectedPoint(int x, int y, glow::SharedFramebuffer const& framebuffer);

    void fpsStyleLookaround(float relDx, float relDy);
    void targetLookaround(float relDx, float relDy);

    void zoom(float delta);

    void moveCamera(float dRight, float dFwd, float dUp, float elapsedSeconds);

    void rotate(float unitsRight, float unitsUp);

    void setOrbit(tg::vec3 dir, tg::vec3 up);
    void setOrbit(tg::angle azimuth, tg::angle altitude, float distance = -1.f);
    void setOrbit(tg::comp<2, tg::angle> spherical, float distance = -1.f)
    {
        setOrbit(spherical.comp0, spherical.comp1, distance);
    }

    void setTransform(tg::pos3 position, tg::pos3 target);

    GLOW_SHARED_CREATOR(CameraController);

    // events
public:
    void update(float elapsedSeconds);
    void onGui();

    // setup / config
public:
    void resize(int w, int h);
    void setupMesh(float size, tg::pos3 center); // e.g. aabb diagonal
    void changeCameraSpeed(int delta);
};
}
