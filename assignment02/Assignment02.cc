#include "Assignment02.hh"

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/VertexArray.hh>

// GLFW
#include <GLFW/glfw3.h>

// imgui
#include <imgui/imgui.h>

// extra helper
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/timing/CpuTimer.hh>
#include "Helper.hh"

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

// returns a uniform random float within min and max
static float random(float min, float max);

///
/// tg Primer:
/// tg:: contains basic vector math functions and classes
/// it closely mimics the naming of GLSL
///
/// Positions:
///     tg::pos3 p = {1.0f, 0.0f, 0.0f};
///
/// Vectors:
///     tg::vec2 v = {1.0f, 0.5f};
///
/// Normal math:
///     tg::vec2 a, b;
///     float f;
///     a + b
///     a * f
///
/// Component access:
///     v.x = 3.0f;
///
/// Useful functions:
///     dot(a, b)       // inner product
///     length(a)       // 2-norm
///     distance(a, b)  // euclidean distance
///     normalize(a)    // a / |a|
///     auto a = tg::radians(3.1415f)   // creates an angle from radians
///     auto a = tg::degree(180)        // creates an angle from degrees
///     tg::cos(a), tg::sin(a)          // sin/cos of a tg::angle
///
///
/// For further information, have a look at the tg primer PDF
/// in the RWTHmoodle under "learning materials"

void Assignment02::initGame()
{
    // Ball entity
    spawnBall();

    // Paddles have
    // - a transform component
    // - a render component
    // - a box shape component (20 x 120)
    // - a static collision component
    // - a paddle component (owned by the respective player)
    for (auto player : {Player::Left, Player::Right})
    {
        const int paddleMargin = 50;
        auto paddle = std::make_shared<Entity>(player == Player::Left ? "Left Paddle" : "Right Paddle");

        auto tc = paddle->addComponent<TransformComponent>();
        tc->position = tg::pos2(player == Player::Left ? paddleMargin : mFieldWidth - paddleMargin, mFieldHeight / 2.0f);
        mTransformComps.push_back(tc);

        auto rc = paddle->addComponent<RenderComponent>();
        rc->color = tg::vec3(1, 1, 1);
        mRenderComps.push_back(rc);

        auto sc = paddle->addComponent<BoxShapeComponent>();
        sc->halfExtent = {10, 60};
        mShapeComps.push_back(sc);

        auto cc = paddle->addComponent<CollisionComponent>();
        cc->dynamic = false;
        mCollisionComps.push_back(cc);

        auto pc = paddle->addComponent<PaddleComponent>();
        pc->owner = player;
        mPaddleComps.push_back(pc);

        mEntities.push_back(paddle);
    }

    // Game borders have
    // - a transform component
    // - a half plane shape
    // - a static collision component
    for (auto isTop : {true, false})
    {
        auto border = std::make_shared<Entity>(isTop ? "Top Border" : "Bottom Border");

        auto tc = border->addComponent<TransformComponent>();
        tc->position = tg::pos2(mFieldWidth / 2, isTop ? 0 : mFieldHeight);
        mTransformComps.push_back(tc);

        auto cc = border->addComponent<CollisionComponent>();
        cc->dynamic = false;
        mCollisionComps.push_back(cc);

        auto sc = border->addComponent<HalfPlaneShapeComponent>();
        sc->normal = tg::vec2(0, isTop ? 1 : -1);
        mShapeComps.push_back(sc);

        mEntities.push_back(border);
    }

    // Region detector have
    // - a transform component
    // - a half plane shape
    // - a region detector component (owned by the respective player)
    for (auto player : {Player::Left, Player::Right})
    {
        auto detector = std::make_shared<Entity>(player == Player::Left ? "Left Detector" : "Right Detector");

        auto tc = detector->addComponent<TransformComponent>();
        tc->position = tg::pos2(player == Player::Left ? 0.0f : mFieldWidth, mFieldHeight / 2.0f);
        mTransformComps.push_back(tc);

        auto rdc = detector->addComponent<RegionDetectorComponent>();
        rdc->owner = player;
        mRegionDetectorComps.push_back(rdc);

        auto sc = detector->addComponent<HalfPlaneShapeComponent>();
        sc->normal = tg::vec2(player == Player::Left ? 1 : -1, 0);
        mShapeComps.push_back(sc);

        mEntities.push_back(detector);
    }
}

void Assignment02::spawnBall()
{
    auto ball = std::make_shared<Entity>("Ball");

    // Balls have
    // - a transform component (starts in center with spawnVelocity)
    // - a render component
    // - a sphere shape component (radius 15)
    // - a dynamic collision component
    // - a ball component

    tg::vec2 spawnVelocity = {300, 0}; // dummy initial value

    /// Task 2.a
    /// Without a proper spawn velocity the ball will only move horizontally
    ///
    /// Your job is to:
    ///     - rotate the velocity by -20°..20° (uniformly, 0° is horizontal)
    ///     - use tg::cos/sin for trigonometry which take tg::angle as input type
    ///     - you can create angles by calling tg::degree/tg::radians
    ///     - choose the speed randomly between 300..400
    ///     - decide if the ball goes left or right (50/50 randomly)
    ///
    /// Note:
    ///     - see random(<min>, <max>) (in this file) - don't use RAND()
    ///
    /// ============= STUDENT CODE BEGIN =============
    auto angle = tg::degree(random(- 20.0, 20.0));

    float speed = 300 + random(0.0, 100.0);

    int direction = (random(-1.0, 1.0) <= 0) * 2 - 1;

    spawnVelocity = tg::vec2(speed * tg::cos(angle) * direction, speed * tg::sin(angle));
    /// ============= STUDENT CODE END =============

    auto tc = ball->addComponent<TransformComponent>();
    tc->position = tg::pos2(mFieldWidth, mFieldHeight) / 2.0f;
    tc->velocity = spawnVelocity;
    mTransformComps.push_back(tc);

    auto rc = ball->addComponent<RenderComponent>();
    rc->color = tg::vec3(.89f, .00f, .40f);
    mRenderComps.push_back(rc);

    auto sc = ball->addComponent<SphereShapeComponent>();
    sc->radius = 15;
    mShapeComps.push_back(sc);

    auto bc = ball->addComponent<BallComponent>();
    mBallComps.push_back(bc);

    /// Task 1.b
    /// Right now the ball is missing a dynamic collision component
    ///
    /// Your job is to:
    ///     - add the collision component to the entity
    ///     - set it to dynamic
    ///     - add it to the global list of collision components
    ///
    /// Note:
    ///     - you can take inspiration from this function and initGame()
    ///
    /// ============= STUDENT CODE BEGIN =============
    auto cc = ball->addComponent<CollisionComponent>();
    cc->dynamic = true;
    mCollisionComps.push_back(cc);
    /// ============= STUDENT CODE END =============

    mEntities.push_back(ball);
}

void Assignment02::updateMotionSystem(float elapsedSeconds)
{
    /// Task 1.a
    /// The motion system is responsible for updating the position of all transform components based on their velocity
    ///
    /// The transform components are stored in the vector 'mTransformComps' (see Assignment02.hh)
    /// For the definition of a transform component see Components.hh
    ///
    /// Your job is to:
    ///     - iterate over these components
    ///     - apply the motion update: x_{i+1} = x_i + v_i * dt
    ///
    /// Note:
    ///     - iterating over a C++ vector v can be done in two ways:
    ///         for (auto i = 0; i < v.size(); ++i) // i is an int
    ///         {
    ///             auto const& element = v[i]; // element is an immutable reference to the i'th entry
    ///         }
    ///         // or directly:
    ///         for (auto const& element : v) { ... }
    ///
    /// ============= STUDENT CODE BEGIN =============
		for (auto const& component : mTransformComps) { component->position += component->velocity * elapsedSeconds; }
    /// ============= STUDENT CODE END =============
}

static bool checkSphereSegmentCollision(tg::pos2 v0, tg::pos2 v1, tg::vec2 n, tg::pos2 c, float r, TransformComponent* tc)
{
    assert(v0 != v1);

    auto isCollision = false;

    // Sphere-Line-Intersection:

    // dir = v1 - v0
    // l = v0 + dir * t
    // sdir = l - c
    // <sdir, dir> == 0
    // <v0 + dir * t - c, dir> == 0
    // <v0 - c, dir> + t * <dir, dir> == 0
    // t = - <v0 - c, dir> / <dir, dir>
    auto d10 = v1 - v0;
    auto d0c = v0 - c;
    auto t = -dot(d0c, d10) / dot(d10, d10);
    t = tg::clamp(t, 0.0f, 1.0f);
    auto np = v0 + d10 * t;
    auto dis = distance(np, c);

    if (dis < r)
        isCollision = true;

    // reflection
    if (isCollision)
    {
        auto dotVN = dot(n, tc->velocity);
        if (dotVN < 0)
            tc->velocity -= 2.0f * dotVN * n;
        else
            isCollision = false; // ignore wrong direction
    }

    return isCollision;
}

void Assignment02::updateCollisionSystem(float elapsedSeconds)
{
    for (auto const& dynamicComp : mCollisionComps)
        if (dynamicComp->dynamic)
            for (auto const& staticComp : mCollisionComps)
                if (!staticComp->dynamic)
                {
                    // sphere only
                    auto dynamicShape = dynamicComp->entity->getComponent<SphereShapeComponent>();
                    auto dynamicTransform = dynamicComp->entity->getComponent<TransformComponent>();

                    auto staticShape = staticComp->entity->getComponent<ShapeComponent>();
                    auto staticPos = staticComp->entity->getComponent<TransformComponent>()->position;
                    auto staticBoxShape = dynamic_cast<BoxShapeComponent*>(staticShape);
                    auto staticHalfPlaneShape = dynamic_cast<HalfPlaneShapeComponent*>(staticShape);
                    if (staticBoxShape != nullptr)
                    {
                        auto p0 = staticPos - staticBoxShape->halfExtent;
                        auto p1 = staticPos + staticBoxShape->halfExtent;
                        auto c = dynamicTransform->position;
                        auto r = dynamicShape->radius;
                        if (checkSphereSegmentCollision({p0.x, p0.y}, {p1.x, p0.y}, {0, -1}, c, r, dynamicTransform) || //
                            checkSphereSegmentCollision({p1.x, p0.y}, {p1.x, p1.y}, {+1, 0}, c, r, dynamicTransform) || //
                            checkSphereSegmentCollision({p1.x, p1.y}, {p0.x, p1.y}, {0, +1}, c, r, dynamicTransform) || //
                            checkSphereSegmentCollision({p0.x, p1.y}, {p0.x, p0.y}, {-1, 0}, c, r, dynamicTransform))
                        {
                            // Collision event
                            sendMessage({MessageType::Collision, staticComp, dynamicComp});

                            // Reflection is already done
                        }
                    }
                    else if (staticHalfPlaneShape != nullptr)
                    {
                        auto dis = dot(dynamicTransform->position - staticPos, staticHalfPlaneShape->normal);
                        if (dis < dynamicShape->radius)
                        {
                            // Collision event
                            sendMessage({MessageType::Collision, staticComp, dynamicComp});

                            // Reflection
                            auto dotVN = dot(staticHalfPlaneShape->normal, dynamicTransform->velocity);
                            if (dotVN < 0)
                                dynamicTransform->velocity -= 2.0f * dotVN * staticHalfPlaneShape->normal;
                        }
                    }
                    else
                        glow::error() << "Unsupported static shape";
                }
}

void Assignment02::updateRegionDetectorSystem(float elapsedSeconds)
{
    for (auto const& detectorComp : mRegionDetectorComps)
        for (auto const& dynamicComp : mCollisionComps)
            if (dynamicComp->dynamic)
            {
                // dynamic can be sphere only
                auto dynamicShape = dynamicComp->entity->getComponent<SphereShapeComponent>();
                auto dynamicTransform = dynamicComp->entity->getComponent<TransformComponent>();

                // detector is half plane only
                auto halfPlaneShape = detectorComp->entity->getComponent<HalfPlaneShapeComponent>();
                auto detectorPos = detectorComp->entity->getComponent<TransformComponent>()->position;

                /// Task 1.d
                /// The region detectors are HalfPlaneShapes that should send a message
                /// when a dynamic collision component is completely inside the detector
                ///
                /// Your job is to:
                ///     - check if the sphere is completely (not partially) inside the detector region
                ///     - send a region detection message if the sphere is detected
                ///
                /// Note:
                ///     - we statically assume that the dynamic shape is a SphereShape
                ///     - sending messages works via 'sendMessage({MessageType::<your type>, <sender>, <subject>})'
                ///     - check Messages.hh for details on the message struct
                ///
                /// ============= STUDENT CODE BEGIN =============
                if (dot(dynamicTransform->position - detectorPos, halfPlaneShape->normal) <= 0) { sendMessage({MessageType::RegionDetection, detectorComp, dynamicComp}); }
                /// ============= STUDENT CODE END =============
            }
}

void Assignment02::updatePaddleSystem(float elapsedSeconds)
{
    for (auto const& paddle : mPaddleComps)
    {
        auto transform = paddle->entity->getComponent<TransformComponent>();
        auto shape = paddle->entity->getComponent<BoxShapeComponent>();
        auto const speed = 1000.0f;

        /// Task 1.c
        /// The next step is to add input handling
        ///
        /// Your job is to:
        ///     - determine if the paddle should be moved up or down
        ///     - change transform->velocity of the paddle depending on the input
        ///     - guarantee that the paddle cannot leave the game field (not even partially)
        ///
        /// Note:
        ///     - Player::Left should use W/S for up/down
        ///     - Player::Right should use UP/DOWN (arrows) for up/down
        ///     - you can check if a key is pressed via 'isKeyDown(GLFW_KEY_<Name of the Key>)'
        ///     - the paddle speed is stored in 'speed'
        ///     - the game field starts at y = 0 and goes up to y = mFieldHeight
        ///     - check out Components.hh if you want to know what data you can access
        ///     - use transform->velocity to move the paddle and transform->position to prevent it from leaving
        ///
        /// ============= STUDENT CODE BEGIN =============
        if (paddle->owner == Player::Left) {
			if (isKeyDown(GLFW_KEY_W)) {
				transform->velocity.y += 10.0;
			}
			else if (isKeyDown(GLFW_KEY_S)) {
				transform->velocity.y -= 10.0;
			}
			if (transform->position.y - shape->halfExtent.y < 0) {
				transform->position.y = shape->halfExtent.y;
				transform->velocity.y = 0.0;
			}
			else if (mFieldHeight < transform->position.y + shape->halfExtent.y) {
				transform->position.y = mFieldHeight - shape->halfExtent.y;
				transform->velocity.y = 0.0;
			}
        }
        else if (paddle->owner == Player::Right) {
			if (isKeyDown(GLFW_KEY_UP)) {
				transform->velocity.y += 10.0;
			}
			else if (isKeyDown(GLFW_KEY_DOWN)) {
				transform->velocity.y -= 10.0;
			}
			if (transform->position.y - shape->halfExtent.y < 0) {
				transform->position.y = shape->halfExtent.y;
				transform->velocity.y = 0.0;
			}
			else if (mFieldHeight < transform->position.y + shape->halfExtent.y) {
				transform->position.y = mFieldHeight - shape->halfExtent.y;
				transform->velocity.y = 0.0;
			}
        }
        /// ============= STUDENT CODE END =============
    }
}

void Assignment02::updateGameLogic(float elapsedSeconds)
{
    // The only job of our global game logic system is to count down the cooldown for multi balls
    // No need to clamp against zero
    mMultiBallCooldown -= elapsedSeconds;
}

void Assignment02::processMessages()
{
    // receive messages
    for (auto const& msg : mMessages)
    {
        switch (msg.type)
        {
        case MessageType::Collision:
        {
            auto const speedLimit = 1000.0f;
            auto isAtSpeedLimit = false;

            /// Task 2.b
            /// Collision events are generated when a dynamic object hits a static one
            /// We want to spice up gameplay by accelerating the ball by 20% every time a _paddle_ is hit
            ///
            /// Your job is to:
            ///     - check if the collision is between a paddle and a ball
            ///     - increase the ball velocity by 20%
            ///     - if the speed is above speedLimit, set 'isAtSpeedLimit' to true and clamp the ball velocity
            ///
            /// Note:
            ///     - again, check Messages.hh and Entity.hh for useful info/functions
            ///
            /// ============= STUDENT CODE BEGIN =============
			if (!msg.sender->entity->hasComponent<PaddleComponent>())
				break;

			if (!msg.subject->entity->hasComponent<BallComponent>())
				break;

			auto ballTransform = msg.subject->entity->getComponent<TransformComponent>();

			ballTransform->velocity *= 1.2;

			if (speedLimit <= length(ballTransform->velocity)){
				isAtSpeedLimit = true;
				ballTransform->velocity = normalize(ballTransform->velocity) * speedLimit;
			}
            /// ============= STUDENT CODE END =============

            if (isAtSpeedLimit)
            {
                /// Task 2.c
                /// As a last improvement to gameplay we want to spawn additional balls
                /// if a paddle is hit at the speed limit and no more than one every 5 seconds
                ///
                /// Your job is to:
                ///     - spawn a new ball (at most one every 5s)
                ///
                /// Note:
                ///     - you may use 'mMultiBallCooldown' (a float decreasing by 1 per sec and starts at 0)
                ///
                /// ============= STUDENT CODE BEGIN =============
            	if (mMultiBallCooldown <= -5.0) { 
            		spawnBall();
            		mMultiBallCooldown = 0.0;
            	}
                /// ============= STUDENT CODE END =============
            }
        }
        break;

        case MessageType::RegionDetection:
        {
            /// Task 1.e
            /// When a ball is in the goal region, a region detection message is sent
            /// This indicates that one player scored a point
            ///
            /// Your job is to:
            ///     - check if message subject is a ball
            ///     - increase 'mScoreLeft' or 'mScoreRight' depending on the owner of the region detector
            ///     - destroy the ball
            ///     - spawn a new ball (only if there are no current balls)
            ///
            /// Note:
            ///     - an entity is a ball if it has a ball component
            ///     - see Entity.hh for how to access/check components
            ///     - see Messages.hh for how to interpret its content (the current message is 'msg')
            ///     - see Assignment02.hh for useful functions and members
            ///     - all active ball components are stored in 'mBallComps' (you can check if a vector is '.empty()')
            ///     - you can also write to the console via 'glow::info() << "...";' when a player scored
            ///
            /// ============= STUDENT CODE BEGIN =============
        	if (!msg.subject->entity->hasComponent<BallComponent>())
        		break;

    		if (msg.sender->entity->getComponent<RegionDetectorComponent>()->owner == Player::Left) {
    			mScoreRight += 1;
    			glow::info() << "player Right scores!";
    		}
    		else {
    			mScoreLeft += 1;
    			glow::info() << "player Left scores!";
    		}
    		destroyEntity(msg.subject->entity);

    		if (mBallComps.empty())
    			spawnBall();
            /// ============= STUDENT CODE END =============
        }
        break;
        }
    }
    mMessages.clear();
}

void Assignment02::update(float elapsedSeconds)
{
    updateMotionSystem(elapsedSeconds);
    updateCollisionSystem(elapsedSeconds);
    updateRegionDetectorSystem(elapsedSeconds);
    updatePaddleSystem(elapsedSeconds);
    updateGameLogic(elapsedSeconds);
    processMessages();
}

void Assignment02::render(float)
{
    // clear the screen in RWTH blue
    glClearColor(0.00f, 0.00f, 0.00f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw all render components
    {
        auto shader = mShaderObj->use();
        auto quad = mQuad->bind();

        // "Zoom" mode for aspect ratio
        auto fieldSize = tg::size2(mFieldWidth, mFieldHeight);
        auto offset = tg::vec2::zero;
        auto scale = tg::size2::zero;
        if (getWindowWidth() > getWindowHeight())
        {
            offset.x = (1.0f - getWindowHeight() / (float)getWindowWidth()) / 2.0f;
            scale = tg::size2(getWindowHeight() / (float)getWindowWidth(), 1.0f) / fieldSize;
        }
        else
        {
            offset.y = (1.0f - getWindowWidth() / (float)getWindowHeight()) / 2.0f;
            scale = tg::size2(1.0f, getWindowWidth() / (float)getWindowHeight()) / fieldSize;
        }

        // blue area
        shader.setUniform("uPosition", offset);
        shader.setUniform("uSize", fieldSize * scale);
        shader.setUniform("uSphere", false);
        shader.setUniform("uPaddle", false);
        shader.setUniform("uColor", tg::vec3(0.00f, 0.33f, 0.62f));
        quad.draw();

        // render entities
        for (auto const& renderComp : mRenderComps)
        {
            auto transformComp = renderComp->entity->getComponent<TransformComponent>();
            auto shapeComp = renderComp->entity->getComponent<ShapeComponent>();
            auto boxShape = dynamic_cast<BoxShapeComponent*>(shapeComp);
            auto sphereShape = dynamic_cast<SphereShapeComponent*>(shapeComp);

            tg::vec2 halfSize;

            if (boxShape)
            {
                halfSize = boxShape->halfExtent;
                shader.setUniform("uSphere", false);
                shader.setUniform("uPaddle", true);
            }
            else if (sphereShape)
            {
                halfSize = tg::vec2(sphereShape->radius);
                shader.setUniform("uSphere", true);
                shader.setUniform("uPaddle", false);
            }
            else
                glow::error() << "Shape not supported";

            shader.setUniform("uSize", 2 * halfSize * scale);
            shader.setUniform("uPosition", (transformComp->position - halfSize) * scale + offset);

            shader.setUniform("uColor", renderComp->color);

            quad.draw();
        }
    }
}

void Assignment02::onGui()
{
    if (ImGui::Begin("RTG Pong Game"))
    {
        ImGui::Text("Score");
        ImGui::Text("%d:%d", mScoreLeft, mScoreRight);
    }
    ImGui::End();
}

void Assignment02::sendMessage(const Message& msg)
{
    glow::info() << "got message " << msg.toString();

    mMessages.push_back(msg);
}

void Assignment02::destroyEntity(Entity* entity)
{
    SharedEntity se;
    for (auto const& e : mEntities)
        if (e.get() == entity)
        {
            se = e;
            break;
        }

    removeElement(mEntities, se); // se is pinned until function exits, thus safe to use
    for (auto c : se->getComponents())
    {
        removeElement(mTransformComps, c);
        removeElement(mShapeComps, c);
        removeElement(mCollisionComps, c);
        removeElement(mRegionDetectorComps, c);
        removeElement(mRenderComps, c);
        removeElement(mBallComps, c);
        removeElement(mPaddleComps, c);
    }
}

void Assignment02::init()
{
    setGui(GlfwApp::Gui::ImGui);

    GlfwApp::init();

    // load simulation resources
    mQuad = geometry::Quad<>().generate();
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shaderObj");

    setTitle("RTG Assignment02 - Simple Pong Game");

    // create initial entities / setup game area
    initGame();
}

static float random(float min, float max)
{
    static tg::rng rng;
    return tg::uniform(rng, min, max);
}
