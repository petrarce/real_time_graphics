#include "Character.hh"

#include <glow/common/log.hh>
#include <typed-geometry/tg.hh>

#include "World.hh"

namespace
{
tg::vec3 getXZPartOfVec3(tg::vec3 const& vec)
{
    auto vecLen = tg::length(vec);
    return vecLen * tg::normalize(tg::vec3(vec.x, 0, vec.z));
}
} // namespace
tg::pos3 Character::update(World& world, float elapsedSeconds, tg::vec3 const& movement, const tg::mat3& camRot)
{
    // ensure chunk exists
    {
        auto ip = tg::ipos3(tg::floor(mPosition));
        world.ensureChunkAt(ip);

        if (!world.queryChunk(ip)->isGenerated())
            return mPosition + tg::vec3(0, height, 0); // chunk not generated, do nothing
    }

    RayHit result = world.rayCast(mPosition + tg::vec3(0, mSwimming ? height - swimHeight + 0.1f : maxStepHeight, 0),
                                  tg::vec3(0, -1, 0), mSwimming ? 0.1f : maxStepHeight + 1e-3f);
    bool touchesGround = result.hasHit;

    tg::vec2 move(movement.x, movement.z);
    if (tg::length_sqr(move) > 1e-12)
    {
        auto worldSpaceMovement = getXZPartOfVec3(tg::inverse(camRot) * tg::vec3(move.x, 0, move.y));

        // Slower movement when swimming or jumping
        if (mSwimming)
            worldSpaceMovement *= 0.3f;

        mVelocity = tg::vec3(0, mVelocity.y, 0) + worldSpaceMovement;


        // do not let the character run into walls
        // we actually need to rays (one a little to the left and one to the right)
        // so that we can detect if we walk by a close wall (on the left or right)
        auto leftRay = tg::rotate_y(tg::normalize(mVelocity), tg::degree(-25.0f));
        auto rightRay = tg::rotate_y(tg::normalize(mVelocity), tg::degree(+25.0f));

        RayHit result = world.rayCast(mPosition + tg::vec3(0, maxStepHeight, 0), getXZPartOfVec3(leftRay), 0.7f);
        if (!result.hasHit) // Check the other side
            result = world.rayCast(mPosition + tg::vec3(0, maxStepHeight, 0), getXZPartOfVec3(rightRay), 0.7f);

        if (result.hasHit)
        {
            // (solid) block in front of character
            auto prohibitedDir = -tg::vec3(result.hitNormal);
            auto normalAmount = dot(prohibitedDir, mVelocity);
            if (normalAmount > 0)
                mVelocity -= dot(prohibitedDir, mVelocity) * prohibitedDir;
        }
    }


    if (touchesGround)
    {
        auto hitPos = result.hitPos;
        auto inWater = result.block.mat == world.getMaterialFromName("water")->index;
        mSwimming = false;
        if (inWater)
        {
            mSwimming = true;
            hitPos.y += -height + swimHeight; // eyes 30cm above water surface
        }

        // Prevent player from running into hills
        if (hitPos.y - 0.1 > mPosition.y)
            mVelocity *= 0;

        // Time in seconds until 80% of the position is updated
        const auto tau = tg::clamp(1.0f / tg::length(move), 0.05f, 0.25f);
        mPosition = tg::mix(hitPos, mPosition, tg::pow(0.2f, elapsedSeconds / tau));
        mVelocity.y = movement.y;
    }
    else
    {
        // Character is not touching ground
        auto acceleration = tg::vec3(0, -9.80665f, 0);
        mVelocity += elapsedSeconds * acceleration;
    }

    // update position
    mPosition += elapsedSeconds * mVelocity;

    // remove velocity within xz plane;
    mVelocity.x = 0;
    mVelocity.z = 0;

    // get camera position from character position
    return mPosition + tg::vec3(0, height, 0);
}
