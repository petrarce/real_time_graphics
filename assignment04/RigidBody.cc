#include "RigidBody.hh"

#include <glow/common/log.hh>

#include <typed-geometry/tg-std.hh>

void RigidBody::calculateMassAndInertia()
{
    // resolution for sampling
    auto const targetVolume = 0.001f;

    // create samples
    std::vector<ShapeSample> samples;
    for (auto const& shape : shapes)
        shape->generateSamples(samples, targetVolume);

    // compute linear and rotational mass via sampling
    mass = 0.0f;
    tg::vec3 cog; // center of gravity (relative to origin)
    inertia = tg::mat3::zero;

/// Task 2.a
/// Calculate inertia and mass via sampling
///
/// Your job is to:
///     - calculate mass (write to variable 'mass')
///     - calculate center of gravity (write to variable 'cog')
///     - calculate inertia matrix (write to variable 'inertia')
///
/// Notes:
///     - sanity check for inertia: it should be roughly diagonal (sides are |.| < 0.5)
///
/// ============= STUDENT CODE BEGIN =============

    for (auto const& s : samples)
    {
        mass += s.mass;
        cog += s.mass * s.offset;

        inertia[0][0] += s.mass * (s.offset[1] * s.offset[1] + s.offset[2] * s.offset[2]);
        inertia[1][1] += s.mass * (s.offset[2] * s.offset[2] + s.offset[0] * s.offset[0]);
        inertia[2][2] += s.mass * (s.offset[0] * s.offset[0] + s.offset[1] * s.offset[1]);

        inertia[0][1] -= s.mass * (s.offset[1] * s.offset[0]);
        inertia[1][0] -= s.mass * (s.offset[0] * s.offset[1]);
        inertia[0][2] -= s.mass * (s.offset[2] * s.offset[0]);
        inertia[2][0] -= s.mass * (s.offset[0] * s.offset[2]);
        inertia[1][2] -= s.mass * (s.offset[2] * s.offset[1]);
        inertia[2][1] -= s.mass * (s.offset[1] * s.offset[2]);
    }

/// ============= STUDENT CODE END =============

    // move to center of mass
    moveShapes(-cog);
    linearPosition += cog;
    glow::info() << "Calculated mass and inertia via " << samples.size() << " samples.";
    glow::info() << " - Mass is " << mass;
    glow::info() << " - Center of Gravity is " << cog;
    glow::info() << " - Inertia is ";
    for (auto i = 0; i < 3; ++i)
        glow::info() << "    " << inertia[i];
}

void RigidBody::loadPreset(Preset preset)
{
    shapes.clear();
    this->preset = preset;

    linearPosition = tg::pos3(0.0);
    linearMomentum = tg::vec3(0.0);

    angularPosition = tg::mat3::identity;
    angularMomentum = tg::vec3(0.0);

    switch (preset)
    {
    case Preset::Pendulum:
        // Would be better to use a point constraint for the particle
        shapes.push_back(std::make_shared<BoxShape>(tg::size3(3, 1.5f, 1), 10.0f, tg::mat4::identity));
        linearPosition = {0, 3, 0};
        break;

    case Preset::RolyPolyToy:
        // bottom sphere
        shapes.push_back(std::make_shared<SphereShape>(2.0f, 3.0f, tg::mat4::identity));
        // heavy weight inside the bottom sphere (should be lower than bottom sphere center)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 15.0f, tg::translation(tg::vec3(0, -1.5f, 0))));
        // head
        shapes.push_back(std::make_shared<SphereShape>(1.3f, 2.0f, tg::translation(tg::vec3(0, 2.5f, 0))));
        // left eye (as seen from front)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 0.1f, tg::translation(tg::vec3(-0.5f, 3.0f, 1.0f))));
        // right eye (as seen from front)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 0.1f, tg::translation(tg::vec3(0.5f, 3.0f, 1.0f))));
        // right arm (as seen from front)
        shapes.push_back(std::make_shared<BoxShape>(
            tg::size3(0.3f, 0.7f, 0.3f), 1.0f,
            tg::translation(tg::pos3(-1.5f, 2.0f, 0)) * tg::rotation_around(tg::degree(45.0f), tg::dir3(0, 0, 1))));
        // left arm (as seen from front)
        shapes.push_back(std::make_shared<BoxShape>(
            tg::size3(0.3f, 0.7f, 0.3f), 1.0f,
            tg::translation(tg::pos3(1.5f, 2.0f, 0)) * tg::rotation_around(tg::degree(-45.0f), tg::dir3(0, 0, 1))));


        linearPosition = {0, 2, 0};
        break;
    }
}

void RigidBody::update(float elapsedSeconds)
{
    auto pendulumDistance = distance(linearPosition, pendulumPosition);

    auto omega = angularVelocity();

    // clamp if rotating too fast
    auto const maxOmega = 100.0f;
    if (length(omega) > maxOmega)
    {
        omega = normalize(omega) * maxOmega;
        glow::warning() << "Angular velocity too high, clamping! (should not happen in correct solutions)";
    }

    // motion equations
    {
/// Task 2.b
///
/// Your job is to:
///     - update the linear momentum and position
///     - update the angular momentum and position
///
/// Notes:
///     - you can test your angular velocity code with the following settings for omega:
///       (be sure to remove test code before uploading)
///         omega = tg::vec3(0, 1, 0) // slow counter-clockwise rotation around Y
///         omega = tg::vec3(5, 0, 0) // faster rotation around X (the long side for the pendulum)
///
///
/// ============= STUDENT CODE BEGIN =============

        // update
        linearMomentum += linearForces * elapsedSeconds;

        linearPosition += linearVelocity() * elapsedSeconds;

        angularMomentum += angularForces * elapsedSeconds;

        angularPosition[0] += cross(omega, angularPosition[0]) * elapsedSeconds;
        angularPosition[1] += cross(omega, angularPosition[1]) * elapsedSeconds;
        angularPosition[2] += cross(omega, angularPosition[2]) * elapsedSeconds;

/// ============= STUDENT CODE END =============

        // re-orthogonalize rotation matrix
        angularPosition[1] -= angularPosition[0] * dot(angularPosition[0], angularPosition[1]);
        angularPosition[2] -= angularPosition[0] * dot(angularPosition[0], angularPosition[2]);
        angularPosition[2] -= angularPosition[1] * dot(angularPosition[1], angularPosition[2]);

        angularPosition[0] = normalize(angularPosition[0]);
        angularPosition[1] = normalize(angularPosition[1]);
        angularPosition[2] = normalize(angularPosition[2]);
    }

    // damping
    linearMomentum *= tg::pow(1.0f - linearDamping, elapsedSeconds);
    angularMomentum *= tg::pow(1.0f - angularDamping, elapsedSeconds);

    // apply pendulum constraint
    if (preset == Preset::Pendulum)
    {
        auto dir = normalize(linearPosition - pendulumPosition);

        linearMomentum -= dot(dir, linearMomentum) * dir;
        linearPosition = pendulumPosition + dir * pendulumDistance;
    }
}

void RigidBody::checkPlaneCollision(tg::pos3 planePos, tg::vec3 planeNormal)
{
    // collide shapes
    for (auto const& s : shapes)
        s->checkPlaneCollision(*this, planePos, planeNormal);
}

void RigidBody::computeCollision(tg::pos3 worldPos, tg::vec3 otherNormal)
{
    // a relative position is actually a vector
    auto relPos = worldPos - linearPosition;

    // get local velocity
    auto velo = velocityInPoint(worldPos);

    // split into tangent and normal dir
    auto vNormal = otherNormal * dot(otherNormal, velo);

    if (dot(otherNormal, vNormal) > 0)
        return; // already separating

    // see http://www.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf p.18
    auto vrel = vNormal;
    auto num = -(1 + restitution) * vrel;
    auto t1 = 1.0f / mass;
    auto t2 = 0.0f; // inf mass
    auto t3 = dot(otherNormal, cross(invInertiaWorld() * cross(relPos, otherNormal), relPos));
    auto t4 = 0.0f; // inf inertia

    auto j = tg::comp3(num) / (t1 + t2 + t3 + t4);
    auto impulse = otherNormal * j;

    // apply correcting impulse in normal dir
    applyImpulse(impulse, worldPos);
}

void RigidBody::moveShapes(tg::vec3 offset)
{
    for (auto const& s : shapes)
        s->transform = tg::translation(offset) * s->transform;
}

tg::pos3 RigidBody::pointLocalToGlobal(tg::pos3 localPos) const
{
    return linearPosition + angularPosition * tg::vec3(localPos);
}

tg::pos3 RigidBody::pointGlobalToLocal(tg::pos3 worldPos) const
{
    return tg::pos3(transpose(angularPosition) * (worldPos - linearPosition));
}

tg::vec3 RigidBody::directionLocalToGlobal(tg::vec3 localDir) const
{
    return angularPosition * localDir;
}

tg::vec3 RigidBody::directionGlobalToLocal(tg::vec3 worldDir) const
{
    return transpose(angularPosition) * worldDir;
}

tg::mat4 RigidBody::getTransform() const
{
    return tg::translation(linearPosition) * tg::mat4(angularPosition);
}

/// Task 2.c
///
/// Your job is to:
///     - implement the bodies of the six methods defined below
///
/// Notes:
///     - in some of the methods you might need to call one of the others
///     - if needed, you should use the tg functions cross, inverse and transpose
///
/// ============= STUDENT CODE BEGIN =============

tg::vec3 RigidBody::linearVelocity() const
{
    return 1.0 / mass * linearMomentum;
}

tg::vec3 RigidBody::angularVelocity() const
{
    return invInertiaWorld() * angularMomentum;
}

tg::vec3 RigidBody::velocityInPoint(tg::pos3 worldPos) const
{
    return linearVelocity() + cross(angularVelocity(), worldPos - linearPosition);
}

tg::mat3 RigidBody::invInertiaWorld() const
{
    return angularPosition * inverse(inertia) * transpose(angularPosition);
}

void RigidBody::applyImpulse(tg::vec3 impulse, tg::pos3 worldPos)
{
    linearMomentum += impulse;
    angularMomentum += cross(worldPos - linearPosition, impulse);
}

void RigidBody::addForce(tg::vec3 force, tg::pos3 worldPos)
{
    linearForces += force;
    angularForces += cross(worldPos - linearPosition, force);
}

/// ============= STUDENT CODE END =============

void RigidBody::clearForces()
{
    linearForces = tg::vec3();
    angularForces = tg::vec3();

    // gravity
    linearForces += tg::vec3(0, gravity, 0) * mass;
}

bool RigidBody::rayCast(tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3& hitPos, tg::vec3& hitNormal, float maxRange) const
{
    hitPos = rayPos + rayDir * maxRange;
    auto hasHit = false;
    auto transform = getTransform();

    for (auto const& s : shapes)
    {
        tg::pos3 pos;
        tg::vec3 normal;

        if (s->rayCast(transform, rayPos, rayDir, pos, normal, maxRange))
        {
            if (distance_sqr(rayPos, pos) < distance_sqr(rayPos, hitPos))
            {
                hitPos = pos;
                hitNormal = normal;
                hasHit = true;
            }
        }
    }

    return hasHit;
}
