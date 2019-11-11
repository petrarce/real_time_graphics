#pragma once

#include <typeinfo>
#include <vector>

#include <glow/common/log.hh>
#include <glow/common/shared.hh>

// Forward declare component struct and entity class
// See Components.hh for a explanation of GLOW_SHARED
GLOW_SHARED(class, Entity);
GLOW_SHARED(struct, Component);

/**
 * An Entity is a named object with a list of attached components
 */
class Entity
{
private:
    // All attached components
    std::vector<SharedComponent> mComponents;

    // The name of the entity
    // (it's const and cannot be changed later on)
    std::string const mName;

public:
    // An entity is constructed with a fixed name
    Entity(std::string const& name) : mName(name) {}

public: // getter
    std::string const& getName() const { return mName; }
    std::vector<SharedComponent> const& getComponents() const { return mComponents; }

public:
    // Creates a new Component of type CompT and returns a shared_ptr to it
    // Also adds it to the component list of this entity
    //
    // Usage:
    //   SharedTransformComponent tc = myEntity->addComponent<TransformComponent>();
    template <typename CompT>
    std::shared_ptr<CompT> addComponent()
    {
        auto comp = std::make_shared<CompT>(this);
        mComponents.push_back(comp);
        return comp;
    }

    // Queries an existing component by type
    // Returns a pointer to the component
    // It is an error to query a non-existing component
    // (Existence can be checked with hasComponent)
    //
    // Usage:
    //  TransformComponent* tc = myEntity->getComponent<TransformComponent>();
    template <typename CompT>
    CompT* getComponent() const
    {
        for (auto const& c : mComponents)
        {
            auto cp = dynamic_cast<CompT*>(c.get());
            if (cp)
                return cp;
        }

        glow::error() << "Could not find a component of type " << typeid(CompT).name();
        return nullptr;
    }

    // Checks if this entity has an attached component of the given type
    //
    // Usage:
    //  bool hasTc = myEntity->hasComponent<TransformComponent>();
    template <typename CompT>
    bool hasComponent() const
    {
        for (auto const& c : mComponents)
            if (dynamic_cast<CompT*>(c.get()))
                return true;

        return false;
    }
};
