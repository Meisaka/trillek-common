#include "components/component.hpp"
#include "property.hpp"
#include "transform.hpp"

namespace trillek { namespace component {

template<>
std::shared_ptr<Container> Initialize<Component::GameTransform>(const std::vector<Property> &properties) {
    glm::vec3 translation(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(0.0f, 0.0f, 0.0f);
    bool radians = true;
    id_t entity_id;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if (name == "tx") {
            translation.x = (float)p.Get<double>();
        }
        else if (name == "ty") {
            translation.y = (float)p.Get<double>();
        }
        else if (name == "tz") {
            translation.z = (float)p.Get<double>();
        }
        else if (name == "rx") {
            rotation.x = (float)p.Get<double>();
        }
        else if (name == "ry") {
            rotation.y = (float)p.Get<double>();
        }
        else if (name == "rz") {
            rotation.z = (float)p.Get<double>();
        }
        else if (name == "sx") {
            scale.x = (float)p.Get<double>();
        }
        else if (name == "sy") {
            scale.y = (float)p.Get<double>();
        }
        else if (name == "sz") {
            scale.z = (float)p.Get<double>();
        }
        else if (name == "entity_id") {
            entity_id = p.Get<id_t>();
        }
        else if (name == "radians") {
            radians = p.Get<bool>();
        }
        else {
            LOGMSG(ERROR) << "GameTransform: Unknown property: " << name;
        }
    }
    if (!radians) {
        glm::radians(rotation);
    }
    return component::Create<Component::GameTransform>(
        Transform(translation, rotation, scale));
}

} // End of component
} // End of trillek
