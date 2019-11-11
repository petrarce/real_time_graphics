#include "CameraController.hh"

#include <glow/common/log.hh>
#include <glow/objects/Framebuffer.hh>

#include <typed-geometry/tg.hh>

namespace
{
tg::vec3 fwd_from_azi_alti(tg::angle azi, tg::angle alt)
{
    auto caz = cos(-azi);
    auto saz = sin(-azi);
    auto cal = cos(-alt);
    auto sal = sin(-alt);
    return tg::vec3(caz * cal, //
                    sal,       //
                    saz * cal);
}
}


namespace glow::viewer
{
void CameraController::setupMesh(float size, tg::pos3 center)
{
    mMeshSize = size;
    mMeshCenter = center;

    s.TargetMinDistance = mMeshSize / 10000.f;
    s.DefaultTargetDistance = mMeshSize * 1.0f;
    s.MoveSpeedStart = mMeshSize;
    s.MoveSpeedMin = mMeshSize / 1000.f;
    s.MoveSpeedMax = mMeshSize * 1000.f;
    s.FocusRefDistance = mMeshSize / 3.f;

    // Near plane must not be zero
    auto constexpr floatEps = 1e-12f;
    static_assert(floatEps > 0.f, "Float epsilon too small");
    s.NearPlane = tg::max(mMeshSize / 100000.f, floatEps);

    resetView();
}

void CameraController::resetView()
{
    mTargetPos = mMeshCenter;
    mTargetDistance = s.DefaultTargetDistance;
    mAltitude = 30_deg;
    mAzimuth = 45_deg;
    mFwd = fwd_from_azi_alti(mAzimuth, mAltitude);
    mUp = {0, 1, 0};
    mRight = normalize(cross(mFwd, mUp));
    mPos = mTargetPos - mFwd * mTargetDistance;
    mMode = Mode::Targeted;
    mMoveSpeed = s.MoveSpeedStart;
}

tg::pos3 CameraController::getPosition() const { return (mSmoothedTargetPos - mSmoothedFwd * mSmoothedTargetDistance); }

tg::mat4 CameraController::computeViewMatrix() const
{
    // tg::look_at crashes if position and target are too close, as floating precision ruins the normalize
    if (TG_LIKELY(tg::distance_sqr(mSmoothedPos, mSmoothedTargetPos) > 1e-36f))
        return tg::look_at(mSmoothedPos, mSmoothedTargetPos, mSmoothedUp);
    else
        return tg::look_at(mSmoothedPos, tg::dir3(0, 0, -1), tg::vec3(0, 1, 0));
}

tg::mat4 CameraController::computeProjMatrix() const
{
    return tg::perspective_reverse_z(s.HorizontalFov, mWindowWidth / float(mWindowHeight), s.NearPlane);
}

CameraController::CameraController() { resetView(); }

void CameraController::clipCamera()
{
    mSmoothedTargetPos = mTargetPos;
    mSmoothedTargetDistance = mTargetDistance;

    mSmoothedUp = mUp;
    mSmoothedRight = mRight;
    mSmoothedFwd = mFwd;
}

void CameraController::focusOnSelectedPoint(int x, int y, glow::SharedFramebuffer const& framebuffer)
{
    if (x < 0 || x >= mWindowWidth)
        return;
    if (y < 0 || y >= mWindowHeight)
        return;

    // read back depth

    auto proj = computeProjMatrix();
    auto view = computeViewMatrix();

    float d;
    {
        auto fb = framebuffer->bind();
        glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);
    }

    if (d == 0.0f)
        return; // background (reverse Z -> depth at 0 means far)

    auto posNdc = tg::pos3(x / (mWindowWidth - 1.0f) * 2.0f - 1.0f,  //
                           y / (mWindowHeight - 1.0f) * 2.0f - 1.0f, //
                           d /* * 2.0 - 1.0 -> reverse Z! */);

    // screen to view
    auto posView = inverse(proj) * posNdc;

    // view to world
    auto posWorld = inverse(view) * posView;

    // zoom in
    auto dir = posWorld - mPos;
    auto dis = length(dir);
    if (dis > s.FocusRefDistance * 2) // too far? clip to focus dis
        dis = s.FocusRefDistance;
    else
        dis /= 2; // otherwise half distance

    mTargetPos = posWorld;

    // reuse old target distance
    mTargetDistance = tg::clamp(dis, s.TargetMinDistance, mTargetDistance);

    mMode = Mode::Targeted;
}

void CameraController::changeCameraSpeed(int delta)
{
    mMoveSpeed *= tg::pow(s.MoveSpeedFactor, delta);
    mMoveSpeed = tg::clamp(mMoveSpeed, s.MoveSpeedMin, s.MoveSpeedMax);

    glow::info() << "Set camera speed to " << mMoveSpeed / 1000 << " km/s";
}

void CameraController::resize(int w, int h)
{
    mWindowWidth = w;
    mWindowHeight = h;
}

void CameraController::zoom(float delta)
{
    float factor = tg::pow(s.ZoomFactor, -delta);

    mTargetDistance *= factor;
    mTargetDistance = tg::max(s.TargetMinDistance, mTargetDistance);

    mMode = Mode::Targeted;
}

void CameraController::fpsStyleLookaround(float relDx, float relDy)
{
    auto angleX = -relDx * s.HorizontalSensitivity;
    auto angleY = relDy * s.VerticalSensitivity;

    if (s.InvertHorizontal)
        angleX *= -1.0f;
    if (s.InvertVertical)
        angleY *= -1.0f;

    mAzimuth += angleX;
    mAltitude += angleY;

    mAltitude = clamp(mAltitude, -89_deg, 89_deg);

    auto pos = mPos;

    mFwd = fwd_from_azi_alti(mAzimuth, mAltitude);
    mRight = normalize(cross(mFwd, mRefUp));
    mUp = normalize(cross(mRight, mFwd));
    mTargetPos = pos + mFwd * mTargetDistance;

    mMode = Mode::FPS;
    mTargetDistance = s.FocusRefDistance;
}

void CameraController::targetLookaround(float relDx, float relDy)
{
    auto angleX = -relDx * s.HorizontalSensitivity * 2;
    auto angleY = relDy * s.VerticalSensitivity * 2;

    if (s.InvertHorizontal)
        angleX *= -1.0f;
    if (s.InvertVertical)
        angleY *= -1.0f;

    mAzimuth += angleX;
    mAltitude += angleY;

    mAltitude = clamp(mAltitude, -89_deg, 89_deg);

    mFwd = fwd_from_azi_alti(mAzimuth, mAltitude);
    mRight = normalize(cross(mFwd, mRefUp));
    mUp = normalize(cross(mRight, mFwd));

    mMode = Mode::Targeted;
}

void CameraController::moveCamera(float dRight, float dFwd, float dUp, float elapsedSeconds)
{
    if (dRight == 0 && dUp == 0 && dFwd == 0)
        return;

    auto speed = mMoveSpeed * elapsedSeconds;

    mPos += mRight * speed * dRight;
    mPos += mUp * speed * dUp;
    mPos += mFwd * speed * dFwd;

    mMode = Mode::FPS;
    mTargetDistance = s.FocusRefDistance;
}

void CameraController::rotate(float unitsRight, float unitsUp)
{
    auto rx = unitsRight * s.NumpadRotateDegree;
    auto ry = unitsUp * s.NumpadRotateDegree;

    if (mMode == Mode::FPS)
    {
        rx = -rx;
        ry = -ry;
    }

    auto rot = tg::rotation_around(rx, normalize(mUp)) * tg::rotation_around(-ry, normalize(mRight));

    mFwd = rot * mFwd;
    mRight = normalize(cross(mFwd, mRefUp));
    mUp = normalize(cross(mRight, mFwd));
}

void CameraController::setOrbit(tg::vec3 dir, tg::vec3 up)
{
    mTargetPos = mMeshCenter;
    mTargetDistance = s.DefaultTargetDistance;
    mFwd = -normalize(dir);
    mRight = normalize(cross(mFwd, up));
    mUp = cross(mRight, mFwd);
    mMode = Mode::Targeted;
}

void CameraController::setOrbit(tg::angle azimuth, tg::angle altitude, float distance)
{
    mTargetPos = mMeshCenter;
    mTargetDistance = distance > 0 ? distance : s.DefaultTargetDistance;
    mAltitude = altitude;
    mAzimuth = azimuth;
    mFwd = fwd_from_azi_alti(mAzimuth, mAltitude);
    mUp = {0, 1, 0};
    mRight = normalize(cross(mFwd, mUp));
    mMode = Mode::Targeted;
}

void CameraController::setTransform(tg::pos3 position, tg::pos3 target)
{
    auto const dir = position - target;
    mTargetPos = target;
    mPos = position;
    mTargetDistance = tg::length(dir);
    mFwd = -normalize(dir);
    mRight = normalize(cross(mFwd, tg::vec3::unit_y));
    mUp = cross(mRight, mFwd);
    mMode = Mode::Targeted;
}

void CameraController::update(float elapsedSeconds)
{
    // update camera view system
    mRight = normalize(cross(mFwd, mUp));
    mUp = normalize(cross(mRight, mFwd));

    // smoothing
    auto alphaTranslational = tg::pow(0.5f, 1000 * elapsedSeconds / s.TranslationalSmoothingHalfTime);
    auto alphaRotational = tg::pow(0.5f, 1000 * elapsedSeconds / s.RotationalSmoothingHalfTime);
    mSmoothedTargetDistance = tg::mix(mTargetDistance, mSmoothedTargetDistance, alphaTranslational);
    mSmoothedTargetPos = mix(mTargetPos, mSmoothedTargetPos, alphaTranslational);
    mSmoothedPos = mix(mPos, mSmoothedPos, alphaTranslational);
    mSmoothedFwd = normalize(mix(mFwd, mSmoothedFwd, alphaRotational));
    mSmoothedRight = normalize(mix(mRight, mSmoothedRight, alphaRotational));
    mSmoothedUp = normalize(mix(mUp, mSmoothedUp, alphaRotational));

    // update derived properties
    switch (mMode)
    {
    case Mode::FPS:
        mTargetPos = mPos + mFwd * mTargetDistance;
        mSmoothedTargetPos = mSmoothedPos + mSmoothedFwd * mSmoothedTargetDistance;
        break;
    case Mode::Targeted:
        mPos = mTargetPos - mFwd * mTargetDistance;
        mSmoothedPos = mSmoothedTargetPos - mSmoothedFwd * mSmoothedTargetDistance;
        break;
    }

    // failsafe
    auto camOk = true;
    // TODO: check infs
    if (!camOk)
        clipCamera();
}
}
