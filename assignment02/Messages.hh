#pragma once

#include <string>

#include <glow/common/shared.hh>

// Forward declare component struct
// See Components.hh for a explanation of GLOW_SHARED
GLOW_SHARED(struct, Component);

// We used a strongly typed enum for identifying the message type
// Usage:
//   MessageType::Collision
//   MessageType::RegionDetection
enum class MessageType
{
    // Sent when a dynamic collision component collides with a static one
    Collision,
    // Sent when a dynamic collision component collides with a region detection component
    RegionDetection
};

struct Message
{
    // Type of the message
    MessageType type;

    // The component that sent the message
    SharedComponent sender;
    // Our messages also have a "subject", which is the content of message
    // Collision:
    //   - sender is the static collision component
    //   - subject the dynamic collision component
    // RegionDetection:
    //   - sender is the region detection component
    //   - subject the dynamic collision component
    //
    // Note that even though sender and subject are components, their entities can be accessed via
    //  sender->entity and subject->entity
    SharedComponent subject;

    // Returns a string representation of the message
    std::string toString() const;
};
