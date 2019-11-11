#pragma once

#include <vector>

#include <typed-geometry/tg-lean.hh>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "Components.hh"
#include "Entity.hh"
#include "Messages.hh"

/**
 * Assignment02: A relatively simple Pong Game written in with the Entity-Component-Systems approach
 */
class Assignment02 : public glow::glfw::GlfwApp
{
private: // logic
    // Points of Player::Left
    int mScoreLeft = 0;
    // Points of Player::Right
    int mScoreRight = 0;

    // Width of the game field
    int mFieldWidth = 1000;
    // Height of the game field
    int mFieldHeight = 1000;

    // Cooldown in seconds until the next multi ball can be spawned
    float mMultiBallCooldown = 0.0f;

private:
    // Initializes the game by creating all important entities
    // Also spawn a first ball
    void initGame();

    // Sends a global message to the message queue
    // Messages are handled in processMessages()
    void sendMessage(Message const& msg);

    // Spawns a new ball in the center of the field
    // Does NOT delete the old ball
    void spawnBall();

    // Destroys an entity and all attached components
    // Removes the entity (and its components) from all lists in this class
    void destroyEntity(Entity* entity);

private: // ECS
    // list of entities
    std::vector<SharedEntity> mEntities;

    // list of components
    std::vector<SharedTransformComponent> mTransformComps;
    std::vector<SharedShapeComponent> mShapeComps;
    std::vector<SharedCollisionComponent> mCollisionComps;
    std::vector<SharedRegionDetectorComponent> mRegionDetectorComps;
    std::vector<SharedRenderComponent> mRenderComps;
    std::vector<SharedBallComponent> mBallComps;
    std::vector<SharedPaddleComponent> mPaddleComps;

    // systems
    void updateMotionSystem(float elapsedSeconds);
    void updateCollisionSystem(float elapsedSeconds);
    void updateRegionDetectorSystem(float elapsedSeconds);
    void updatePaddleSystem(float elapsedSeconds);
    void updateGameLogic(float elapsedSeconds);
    void processMessages();

    // message queue
    std::vector<Message> mMessages;

private: // graphics
    glow::SharedVertexArray mQuad;
    glow::SharedProgram mShaderObj;

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float) override;
    void onGui() override;
};
