#pragma once

#include <glow/common/property.hh>
#include <typed-geometry/tg-lean.hh>

class World;
class Character
{
public:
    /// Height of the character
    float height = 1.8f;
    /// Max step height the character can take without jumping
    float maxStepHeight = 1.1f;
    /// Character movement speed
    float movementSpeed = 4.0f;
    /// Character jump speed
    float jumpSpeed = 5.0f;
    /// Distance eyes to surface when swimming
    float swimHeight = 0.2f;

private:
    /// Character position
    tg::pos3 mPosition;
    /// Character velocity;
    tg::vec3 mVelocity;
    /// Detect whether character is currently swimming
    bool mSwimming = false;
public:
    GLOW_PROPERTY(Position);

public:
    Character() = default;

    Character(tg::pos3 const& initialPosition) : mPosition(initialPosition) {}

    // returns new character (eye) position
    tg::pos3 update(World &world, float elapsedSeconds, tg::vec3 const& movement, tg::mat3 const& camRot);

    float getMovementSpeed(bool isRunning) const { return isRunning ? 2.0f * movementSpeed : movementSpeed; }
    float getJumpSpeed(bool isRunning) const { return isRunning ? 1.0f * jumpSpeed : jumpSpeed; }
};
