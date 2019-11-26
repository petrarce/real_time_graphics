#pragma once

#include <vector>
#include <typed-geometry/tg-lean.hh>

struct ShapeSample
{
    float mass;
    tg::vec3 offset;
};

struct RigidBody;

struct Shape
{
    float mass;
    tg::mat4 transform;

    Shape(float mass, tg::mat4 transform) : mass(mass), transform(transform) {}

    virtual ~Shape(); // otherwse undefined behavior

    virtual void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const = 0;

    virtual bool rayCast(tg::mat4 const& rbTransform,
                         tg::pos3 rayPos,
                         tg::vec3 rayDir,
                         tg::pos3& hitPos,
                         tg::vec3& hitNormal,
                         float maxRange) const = 0;

    virtual void checkPlaneCollision(RigidBody& body, tg::pos3 planePos, tg::vec3 planeNormal) const = 0;
};

struct SphereShape : Shape
{
    float const radius;

    SphereShape(float r, float mass, tg::mat4 transform) : Shape(mass, transform), radius(r) {}

    void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const override;

    bool rayCast(tg::mat4 const& rbTransform, tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3& hitPos, tg::vec3& hitNormal, float maxRange) const override;

    void checkPlaneCollision(RigidBody& body, tg::pos3 planePos, tg::vec3 planeNormal) const override;
};

struct BoxShape : Shape
{
    tg::size3 const halfExtent;

    BoxShape(tg::size3 halfExtent, float mass, tg::mat4 transform) : Shape(mass, transform), halfExtent(halfExtent) {}

    void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const override;

    bool rayCast(tg::mat4 const& rbTransform, tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3& hitPos, tg::vec3& hitNormal, float maxRange) const override;

    void checkPlaneCollision(RigidBody& body, tg::pos3 planePos, tg::vec3 planeNormal) const override;
};
