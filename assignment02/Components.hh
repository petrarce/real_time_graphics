#pragma once

#include <typed-geometry/tg-lean.hh>

#include <glow/common/shared.hh>

#include "Player.hh"

// GLOW_SHARED introduces typedef's for shared pointers
// GLOW_SHARED(class, T) introduces SharedT as a typedef of std::shared_ptr<T>
// Shared pointer are the preferred way to manage objects in modern C++
// (Value types, e.g. tg::vec3 and int, are typically passed by-value - without pointers)
// (Technically, usage of unique pointers is encouraged but shared pointers are easier to handle when starting C++)
GLOW_SHARED(class, Entity);
GLOW_SHARED(struct, Component);
GLOW_SHARED(struct, TransformComponent);
GLOW_SHARED(struct, CollisionComponent);
GLOW_SHARED(struct, RegionDetectorComponent);
GLOW_SHARED(struct, ShapeComponent);
GLOW_SHARED(struct, RenderComponent);
GLOW_SHARED(struct, BallComponent);
GLOW_SHARED(struct, PaddleComponent);

/**
 * Base Component struct
 *
 * Components are Plain Old Data (POD), store only data and no code
 */
struct Component
{
    // Backreference from Component to Entity
    // The pointer is const (not the Entity) so that it cannot be changed by accident
    Entity* const entity;

    // A component can only be created for a given entity
    // No re-use of components between multiple entities (in this implementation)
    Component(Entity* e) : entity(e) {}

    // Forbid copying of components to prevent usage errors
    Component(Component const&) = delete;
    Component& operator=(Component const&) = delete;
    // Destructor must be virtual in order to safely delete subclasses
    virtual ~Component() {}
};

// A transform component stores the motion state of an entity
struct TransformComponent : Component
{
    using Component::Component; //< "import" the constructor of Component

    // current position
    tg::pos2 position;
    // current velocity
    tg::vec2 velocity;
};

// Marks this entity as "participating in collisions"
struct CollisionComponent : Component
{
    using Component::Component; //< "import" the constructor of Component

    // collision is only checked between dynamic and static (!dynamic) components
    // dynamic components change their velocity on collision
    bool dynamic = false;
};

// Entites with a region detector generate messages if a dynamic collision component is (fully) in their shape
struct RegionDetectorComponent : Component
{
    using Component::Component; //< "import" the constructor of Component

    // A region is owned by a player
    // If a ball is detected within this region, the other player scores
    Player owner;
};

// Causes this entity to be rendered
struct RenderComponent : Component
{
    using Component::Component; //< "import" the constructor of Component

    // solid fill color
    tg::vec3 color = {1, 1, 1};
};

// Marks this entity as a ball
struct BallComponent : Component
{
    using Component::Component; //< "import" the constructor of Component
};

// Marks this entity as a paddle
struct PaddleComponent : Component
{
    using Component::Component; //< "import" the constructor of Component

    // A paddle is owned by a player (e.g. used for input)
    Player owner;
};

// Describes the shape of this entity
struct ShapeComponent : Component
{
    using Component::Component; //< "import" the constructor of Component
};

// Shape: An axis-aligned box centered around transform->position
struct BoxShapeComponent : ShapeComponent
{
    using ShapeComponent::ShapeComponent; //< "import" the constructor of Component

    // half width and height
    // e.g. the unit square has halfExtent (0.5, 0.5)
    tg::vec2 halfExtent;
};

// Shape: A sphere centered around transform->position
struct SphereShapeComponent : ShapeComponent
{
    using ShapeComponent::ShapeComponent; //< "import" the constructor of Component

    // radius of the sphere
    float radius;
};

// Shape: half of the 2D plane. The dividing line goes through transform->position
struct HalfPlaneShapeComponent : ShapeComponent
{
    using ShapeComponent::ShapeComponent; //< "import" the constructor of Component

    // normal points away from colliding half plane
    // i.e. a point x is inside this shape iff: dot(x - transform->position, normal) <= 0
    tg::vec2 normal;
};
