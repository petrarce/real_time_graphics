#pragma once

#include <glow/common/shared.hh>

#include <glm/ext.hpp>

#include "Shapes.hh"

GLOW_SHARED(struct, Shape);

enum class Preset
{
    Pendulum,
    RolyPolyToy
};

/**
 * @brief A RigidBody
 *
 * Notes:
 *  - we ignore encapsulation (getter/setter) here to reduce boilerplate
 *  - relPos is a relative position in world space ("worldPos - linearPos")
 *
 */
struct RigidBody
{
public: // fields
    // preset
    Preset preset;

    // geometry
    std::vector<SharedShape> shapes;
    tg::pos3 pendulumPosition = {0, 10, 0};

    // motion state
    // NOTE: optimized implementations would store invMass and invInertia
    float mass = 0.0f;
    tg::mat3 inertia;

    tg::pos3 linearPosition;  // translation
    tg::mat3 angularPosition; // rotation

    tg::vec3 linearMomentum;  // mass * velocity
    tg::vec3 angularMomentum; // inertia * angular velocity

    tg::vec3 linearForces;  // force
    tg::vec3 angularForces; // torque

    float linearDamping = 0.1f;
    float angularDamping = 0.1f;

    float restitution = 0.5f;
    float friction = 0.5f;

    float gravity = -9.81f;

public: // properties
    /// returns the current transformation matrix
    tg::mat4 getTransform() const;

    /// returns the linear velocity
    tg::vec3 linearVelocity() const;

    /// returns the angular velocity
    tg::vec3 angularVelocity() const;

    /// returns the world velocity at a given point
    tg::vec3 velocityInPoint(tg::pos3 worldPos) const;

    /// converts a rigid-body local point (0 is center of gravity) to a world position
    tg::pos3 pointLocalToGlobal(tg::pos3 localPos) const;

    /// converts a global position to a local one
    tg::pos3 pointGlobalToLocal(tg::pos3 worldPos) const;

    /// converts a rigid-body local direction to a world direction
    tg::vec3 directionLocalToGlobal(tg::vec3 localDir) const;

    /// converts a global direction to a local one
    tg::vec3 directionGlobalToLocal(tg::vec3 worldDir) const;

    /// recalculates world inertia
    tg::mat3 invInertiaWorld() const;

public: // const functions
    /// performas a raycast against this rigid body
    /// resulting hitPos and hitNormal are in global space
    bool rayCast(tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3 &hitPos, tg::vec3& hitNormal, float maxRange = 10000.0f) const;

public: // setup functions
    /// calculates mass and inertia from shapes
    void calculateMassAndInertia();

    /// loads a given preset
    void loadPreset(Preset preset);

    /// moves all shapes
    void moveShapes(tg::vec3 offset);

public: // mutating functions
    /// applies equation of motion
    void update(float elapsedSeconds);

    /// clears linear and angular forces
    void clearForces();

    /// adds a force to a given relative position (0 is center of gravity)
    /// force itself is in world coords
    void addForce(tg::vec3 force, tg::pos3 worldPos);

    /// applies an impulse at a relative point
    /// impulse itself is in world coords
    void applyImpulse(tg::vec3 impulse, tg::pos3 worldPos);

    /// performs collision handling against a plane
    void checkPlaneCollision(tg::pos3 planePos, tg::vec3 planeNormal);

    /// applies collision handling for a given relPos and normal of other object
    /// does NOT correct position
    void computeCollision(tg::pos3 worldPos, tg::vec3 otherNormal);
};
