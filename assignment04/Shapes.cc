#include "Shapes.hh"

#include "RigidBody.hh"

#include <glow/common/log.hh>

#include <typed-geometry/tg-std.hh>

#include <random>


static tg::rng rng;

void SphereShape::generateSamples(std::vector<ShapeSample> &samples, float targetVolume) const
{
    auto volume = radius * radius * radius * 4.0f / 3.0f * tg::pi<float>.radians();
    auto n = (int)tg::max(1.0f, tg::round(volume / targetVolume));

    for (auto i = 0; i < n; ++i)
    {
        auto offset = tg::uniform(rng, tg::ball3::unit) * radius;

        ShapeSample sample;
        sample.mass = mass / n;
        sample.offset = tg::vec3(transform * offset);
        samples.push_back(sample);
    }
}

bool SphereShape::rayCast(tg::mat4 const &rbTransform, tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3 &hitPos, tg::vec3 &hitNormal, float maxRange) const
{
    // center in world space
    auto worldCenter = tg::pos3(rbTransform * transform * tg::vec4(0, 0, 0, 1));
    auto d = worldCenter - rayPos;
    auto closestP = rayPos + rayDir * dot(d, rayDir);
    auto dis = closestP - worldCenter;

    auto r = length(dis);
    if (r < radius)
    {
        auto delta = tg::sqrt(radius * radius - r * r);
        if (delta > distance(closestP, rayPos))
            delta *= -1.0f; // inside sphere

        hitPos = closestP - rayDir * delta;
        hitNormal = normalize(hitPos - worldCenter);
        return true;
    }

    return false;
}

void SphereShape::checkPlaneCollision(RigidBody &body, tg::pos3 planePos, tg::vec3 planeNormal) const
{
    // check if hit
    auto m = body.getTransform() * transform;
    auto worldCenter = tg::pos3(m * tg::vec4(0, 0, 0, 1));
    auto dis = dot(worldCenter - planePos, planeNormal);
    if (dis < radius) // collision
    {
        // handle collision
        auto planeHit = worldCenter - planeNormal * dis;
        body.computeCollision(planeHit, planeNormal);

        // fix position
        body.linearPosition += planeNormal * (radius - dis);
    }
}

void BoxShape::generateSamples(std::vector<ShapeSample> &samples, float targetVolume) const
{
    auto volume = halfExtent.width * halfExtent.height * halfExtent.depth * 8.0f;
    auto n = (int)tg::max(1.0f, tg::round(volume / targetVolume));

    for (auto i = 0; i < n; ++i)
    {
        auto offset = tg::vec3(tg::uniform(rng, -halfExtent.width, halfExtent.width),   //
                               tg::uniform(rng, -halfExtent.height, halfExtent.height), //
                               tg::uniform(rng, -halfExtent.depth, halfExtent.depth)    //
        );

        ShapeSample sample;
        sample.mass = mass / n;
        sample.offset = transform * offset;
        samples.push_back(sample);
    }
}

bool BoxShape::rayCast(tg::mat4 const &rbTransform, tg::pos3 rayPos, tg::vec3 rayDir, tg::pos3 &hitPos, tg::vec3 &hitNormal, float maxRange) const
{
    auto m4 = rbTransform * transform;
    auto m3 = tg::mat3(m4);

    hitPos = rayPos + rayDir * maxRange;
    auto hasHit = false;

    for (auto s : {-1.0f, 1.0f})
        for (auto d : {0, 1, 2})
        {
            auto normal = tg::vec3(d == 0, d == 1, d == 2);
            auto tangent0 = tg::vec3(d == 1, d == 2, d == 0);
            auto tangent1 = tg::vec3(d == 2, d == 0, d == 1);

            auto c = normal * dot(normal, tg::vec3(halfExtent)) * s;
            auto d0 = dot(tangent0, tg::vec3(halfExtent));
            auto d1 = dot(tangent1, tg::vec3(halfExtent));

            auto worldNormal = m3 * normal * s;
            auto worldTangent0 = m3 * tangent0 * s;
            auto worldTangent1 = m3 * tangent1 * s;
            auto worldCenter = tg::vec3(m4 * tg::vec4(c, 1));

            if (tg::abs(dot(worldNormal, rayDir)) < 0.0001)
                continue; // near parallel

            // project to plane
            auto toRayPos = rayPos - worldCenter;
            auto planePos = rayPos - rayDir * dot(toRayPos, worldNormal) / dot(rayDir, worldNormal);

            // check if inside
            auto planeDir = planePos - worldCenter;
            if (tg::abs(dot(worldTangent0, planeDir)) < d0 && tg::abs(dot(worldTangent1, planeDir)) < d1)
            {
                // TODO: handle inside-box scenario

                // check if closer
                if (distance_sqr(rayPos, planePos) < distance_sqr(rayPos, hitPos))
                {
                    hitPos = planePos;
                    hitNormal = worldNormal;
                    hasHit = true;
                }
            }
        }

    return hasHit;
}

void BoxShape::checkPlaneCollision(RigidBody &body, tg::pos3 planePos, tg::vec3 planeNormal) const
{
    auto m = body.getTransform() * transform;

    for (auto dx : {-1, 1})
        for (auto dy : {-1, 1})
            for (auto dz : {-1, 1})
            {
                auto localPos = halfExtent * tg::comp3(dx, dy, dz);
                auto worldPos = tg::pos3(m * tg::pos3(localPos));

                auto dis = dot(worldPos - planePos, planeNormal);
                if (dis < 0)
                {
                    // handle collision
                    body.computeCollision(worldPos, planeNormal);

                    // fix position
                    body.linearPosition += planeNormal * (-dis);
                }
            }
}


Shape::~Shape() = default;
