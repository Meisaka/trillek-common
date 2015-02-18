#include "components/component.hpp"
#include "property.hpp"
#include "components/component-container.hpp"
#include "physics/collidable.hpp"
#include "interaction.hpp"
#include "trillek-game.hpp"
#include "components/system-component-value.hpp"
#include "components/system-component.hpp"
#include "components/shared-component.hpp"

namespace trillek { namespace component {

template<>
struct ContainerRef<System> {
    static System& container;
};

System& ContainerRef<System>::container = game.GetSystemComponent();

template<>
struct ContainerRef<SystemValue> {
    static SystemValue& container;
};

SystemValue& ContainerRef<SystemValue>::container = game.GetSystemValueComponent();

template<>
struct ContainerRef<Shared> {
    static Shared& container;
};

Shared& ContainerRef<Shared>::container = game.GetSharedComponent();

template<>
std::shared_ptr<Container> Initialize<Component::VelocityMax>(const id_t entity_id, const std::vector<Property> &properties) {
    glm::vec3 lmax(0.0f,0.0f,0.0f), amax(0.0f,0.0f,0.0f);
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "max_horizontal") {
            auto tlmax = (float) p.Get<double>();
            lmax.x = tlmax;
            lmax.z = tlmax;
        }
        else if (name == "max_vertical") {
            lmax.y = (float) p.Get<double>();
        }
        else if (name == "max_angular") {
            auto tamax = (float) p.Get<double>();
            amax.x = tamax;
            amax.y = tamax;
            amax.z = tamax;
        }
        else {
            LOGMSG(ERROR) << "VelocityMax: Unknown property: " << name;
            return nullptr;
        }
    }
    return component::Create<Component::VelocityMax>(VelocityMax_type(std::move(lmax), std::move(amax)));
}

template<>
id_t Initialize<Component::ReferenceFrame>(bool& result, const id_t entity_id, const std::vector<Property> &properties) {
    id_t reference_id;
    Velocity_type ref_velocity;
    result = false;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "entity") {
            reference_id = p.Get<id_t>();
            if (! game.GetSharedComponent().Has<Component::Velocity>(reference_id)) {
                LOGMSG(ERROR) << "ReferenceFrame: entity #" << reference_id << "does not have velocity";
                return 0;
            }
            ref_velocity = game.GetSharedComponent().Get<Component::Velocity>(reference_id);
            result = true;
        }
        else {
            LOGMSG(ERROR) << "ReferenceFrame: Unknown property: " << name;
        }
    }
    if (result) {
        // create IsReferenceFrame and CombinedVelocity components
        game.GetSystemValueComponent().Insert<Component::IsReferenceFrame>(reference_id, true);
        game.GetSystemComponent().Insert<Component::CombinedVelocity>(entity_id, ref_velocity);
        return reference_id;
    }
    return 0;
}

template<>
float_t Initialize<Component::OxygenRate>(bool& result, const id_t entity_id, const std::vector<Property> &properties) {
    auto oxygen_rate = 20.0f;       // default value;
    result = false;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "rate") {
            oxygen_rate = p.Get<float>();
        }
        else {
            LOGMSG(ERROR) << "OxygenRate: Unknown property: " << name;
        }
    }
    if (entity_id > 0) {
        result = true;
    }
    return oxygen_rate;
}

template<>
uint32_t Initialize<Component::Health>(bool& result, const id_t entity_id, const std::vector<Property> &properties) {
    uint32_t health = 100;       // default value;
    result = false;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "health") {
            health = p.Get<uint32_t>();
        }
        else {
            LOGMSG(ERROR) << "Health: Unknown property: " << name;
        }
    }
    if (entity_id > 0) {
        result = true;
    }
    return health;
}

template<>
bool Initialize<Component::Movable>(bool& result, const id_t entity_id, const std::vector<Property> &properties) {
    result = false;
    bool movable = false;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "movable") {
            movable = p.Get<bool>();
        }
        else {
            LOGMSG(ERROR) << "Movable: Unknown property: " << name;
        }
    }
    if (entity_id > 0) {
        result = true;
    }
    if(movable) {
        if(!Has<Component::Interactable>(entity_id)) {
            game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
        }
        auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
        act.AddAction(Action::IA_MOVE);
    }
    return movable;
}

} // namespace component
} // namespace trillek
