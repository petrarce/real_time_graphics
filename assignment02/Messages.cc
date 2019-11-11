#include "Messages.hh"

#include "Components.hh"
#include "Entity.hh"

std::string Message::toString() const
{
    std::string reason;
    switch (type)
    {
    case MessageType::Collision:
        reason = "Collision";
        break;
    case MessageType::RegionDetection:
        reason = "Region Detection";
        break;
    }

    return reason + " from " + sender->entity->getName() + " about " + subject->entity->getName();
}
