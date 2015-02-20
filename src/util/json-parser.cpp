#include "util/json-parser.hpp"
#include "resources/text-file.hpp"
#include "systems/resource-system.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"
#include "logging.hpp"

#include <mutex>
#include <iostream>

namespace trillek {
namespace util {

std::string MakeString(const rapidjson::Value& v) {
    return std::string(v.GetString(), v.GetStringLength());
}

JSONParser::JSONParser() {
    static std::once_flag only_one;

    std::call_once(only_one, [this] () { RegisterTypes(); this->RegisterComponentTypes(); });
}
std::shared_ptr<JSONComponent> JSONParser::ParseJSONComponent(rapidjson::Value& node) {
    if (node.IsObject()) {
        auto component = std::make_shared<JSONComponent>();

        for (auto component_property_itr = node.MemberBegin();
            component_property_itr != node.MemberEnd(); ++component_property_itr) {
            std::string component_property_name = MakeString(component_property_itr->name);

            if (component_property_itr->value.IsString()) {
                std::string property_value = MakeString(component_property_itr->value);
                Property p(component_property_name, property_value);
                component->properties.push_back(p);
            }
            else if (component_property_itr->value.IsBool()) {
                bool property_value = component_property_itr->value.GetBool();
                Property p(component_property_name, property_value);
                component->properties.push_back(p);
            }
            else if (component_property_itr->value.IsDouble()) {
                double property_value = component_property_itr->value.GetDouble();
                Property p(component_property_name, property_value);
                component->properties.push_back(p);
            }
            else if (component_property_itr->value.IsInt()) {
                int property_value = component_property_itr->value.GetInt();
                Property p(component_property_name, property_value);
                component->properties.push_back(p);
            }
            else if (component_property_itr->value.IsUint()) {
                unsigned int property_value = component_property_itr->value.GetUint();
                Property p(component_property_name, property_value);
                component->properties.push_back(p);
            }
            else if (component_property_itr->value.IsArray()) {
                auto array_itr = component_property_itr->value.Begin();
                if (array_itr != component_property_itr->value.End()) {
                    if (array_itr->IsNumber()) {
                        std::vector<double> values;
                        for (; array_itr != component_property_itr->value.End(); array_itr++) {
                            if (array_itr->IsNumber()) {
                                values.push_back(array_itr->GetDouble());
                            }
                        }
                        if (values.size() == 2) {
                            Property p(component_property_name,
                                glm::vec2(values[0], values[1]));
                            component->properties.push_back(p);
                        }
                        else if (values.size() == 3) {
                            Property p(component_property_name,
                                glm::vec3(values[0], values[1], values[2]));
                            component->properties.push_back(p);
                        }
                        else if (values.size() == 4) {
                            Property p(component_property_name,
                                glm::vec4(values[0], values[1], values[2], values[3]));
                            component->properties.push_back(p);
                        }
                    }
                    else if (array_itr->IsBool()) {
                        std::vector<bool> values;
                        for (; array_itr != component_property_itr->value.End(); array_itr++) {
                            if (array_itr->IsBool()) {
                                values.push_back(array_itr->GetBool());
                            }
                        }
                        component->properties.push_back(Property(component_property_name, std::move(values)));
                    }
                    else if (array_itr->IsString()) {
                        std::vector<std::string> values;
                        for (; array_itr != component_property_itr->value.End(); array_itr++) {
                            if (array_itr->IsString()) {
                                values.push_back(std::string(array_itr->GetString(), array_itr->GetStringLength()));
                            }
                        }
                        component->properties.push_back(Property(component_property_name, std::move(values)));
                    }
                }
            }
            // TODO ele if (component_property_itr->value.IsObject()) {} to handle inline resource initialization, etc.
        }
        return component;
    }
    return nullptr;
}

void JSONParser::ParseJSONEntity(rapidjson::Value& node) {
    if (node.IsArray()) {
        for (auto entity_itr = node.Begin(); entity_itr != node.End(); ++entity_itr) {
            if (entity_itr->IsObject()) {
                std::string entity_name = "";
                unsigned int entity_id = 0;
                auto entity = std::make_shared<JSONEntity>();

                for (auto entity_property_itr = entity_itr->MemberBegin();
                    entity_property_itr != entity_itr->MemberEnd(); ++entity_property_itr) {
                    std::string entity_property_name = MakeString(entity_property_itr->name);
                    if (entity_property_itr->value.IsObject()) {
                        if (entity_property_itr->value.IsObject()) {
                            unsigned int component_type_id = GetTypeIDFromName(entity_property_name);
                            
                            auto component = ParseJSONComponent(entity_property_itr->value);
                            if (component) {
                                component->component_name = entity_property_name;

                                if (this->factories.count(component_type_id)) {
                                    if (!this->factories[component_type_id](entity_id, component->properties)) {
                                        LOGMSG(WARNING) << "component-factory: Creating component failed";
                                    }
                                }
                                else {
                                    LOGMSG(WARNING) << "component-factory: Unknown component type: " << component->component_name;
                                }
                                entity->components.push_back(std::move(component));
                            }
                        }
                    }
                    else if (entity_property_itr->value.IsString()) {
                        std::string property_value = MakeString(entity_property_itr->value);
                        if (entity_property_name == "name") {
                            entity_name = property_value;
                        }
                        auto p = std::make_shared<Property>(entity_property_name, property_value);
                        entity->meta_properties[entity_property_name] = p;
                    }
                    else if (entity_property_itr->value.IsBool()) {
                        bool property_value = entity_property_itr->value.GetBool();
                        auto p = std::make_shared<Property>(entity_property_name, property_value);
                        entity->meta_properties[entity_property_name] = p;
                    }
                    else if (entity_property_itr->value.IsDouble()) {
                        double property_value = entity_property_itr->value.GetDouble();
                        auto p = std::make_shared<Property>(entity_property_name, property_value);
                        entity->meta_properties[entity_property_name] = p;
                    }
                    else if (entity_property_itr->value.IsInt()) {
                        int property_value = entity_property_itr->value.GetInt();
                        if (entity_property_name == "id") {
                            entity_id = property_value;
                        }
                        auto p = std::make_shared<Property>(entity_property_name, property_value);
                        entity->meta_properties[entity_property_name] = p;
                    }
                    else if (entity_property_itr->value.IsUint()) {
                        unsigned int property_value = entity_property_itr->value.GetUint();
                        if (entity_property_name == "id") {
                            entity_id = property_value;
                        }
                        auto p = std::make_shared<Property>(entity_property_name, property_value);
                        entity->meta_properties[entity_property_name] = p;
                    }
                    else { }
                }

                this->entities.push_back(std::move(entity));
            }
        }
    }
}

bool JSONParser::Parse(const std::string& fname) {
    std::vector<Property> props;
    Property p("filename", fname);
    props.push_back(p);

    auto file = resource::ResourceMap::Create<resource::TextFile>(fname, props);

    if (!file) {
        LOGMSG(ERROR) << "Error parsing: " << fname;
        return false;
    }

    std::shared_ptr<JSONDocument> doc;
    for (auto temp_doc : this->documents) {
        if (temp_doc->file == file) {
            doc = temp_doc;
        }
    }

    if (doc == nullptr) {
        doc = std::make_shared<JSONDocument>();
        doc->file = file;
        this->documents.push_back(doc);
    }

    doc->document.Parse<0>(doc->file->GetText().c_str());
    if (doc->document.HasParseError()) {
        LOGMSG(ERROR) << "JSON error at offset " << doc->document.GetErrorOffset() << ": " << doc->document.GetParseError();
        return false;
    }

    if (doc->document.IsObject()) {
        for (auto itr = doc->document.MemberBegin(); itr != doc->document.MemberEnd(); ++itr) {
            std::string name(itr->name.GetString(), itr->name.GetStringLength());
            if (itr->value.IsString()) {
                std::string value = MakeString(itr->value);
                if (value.find("@") != std::string::npos) {
                    std::string file_path;

                    if (fname.find("/") != std::string::npos) {
                        file_path = fname.substr(0, fname.find_last_of("/") + 1);
                    }
                    else if (fname.find("\"") != std::string::npos) {
                        file_path = fname.substr(0, fname.find_last_of("\"") + 1);
                    }

                    value = file_path + value.substr(1);
                    Parse(value);
                }
            }
            else if (itr->value.IsObject() || itr->value.IsArray()) {
                if (name == "entities") {
                    ParseJSONEntity(itr->value);
                }
                if (this->parsers.find(name) != this->parsers.end()) {
                    this->parsers[name]->Parse(itr->value);
                }
            }
        }
    }

    return true;
}

void JSONParser::Serialize(const std::string& out_directory, const std::string& fname, std::shared_ptr<Parser> parser) {
    if (fname.length() > 0) {
        rapidjson::Document document;
        document.SetObject();

        if (parser) {
            parser->Serialize(document);
        }
        else {
            for (auto serializer : parsers) {
                serializer.second->Serialize(document);
            }
        }

        FILE* file = fopen((out_directory + fname).c_str(), "w");
        rapidjson::FileStream f(file);
        rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
        document.Accept(writer);
        fclose(file);
    }
    else {

        if (parser) {
            rapidjson::Document document;
            document.SetObject();
            parser->Serialize(document);

            FILE* file = fopen((out_directory + fname + ".json").c_str(), "w");
            rapidjson::FileStream f(file);
            rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
            document.Accept(writer);
            fclose(file);
        }
        else {
            for (auto serializer : parsers) {
                rapidjson::Document document;
                document.SetObject();
                serializer.second->Serialize(document);

                FILE* file = fopen((out_directory + serializer.first + ".json").c_str(), "w");
                rapidjson::FileStream f(file);
                rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
                document.Accept(writer);
                fclose(file);
            }
        }
    }
}

std::map<std::string, Parser*> JSONParser::parsers;

void JSONParser::RegisterParser(std::shared_ptr<Parser> parser) {
    parsers[parser->GetNodeTypeName()] = parser.get();
}

void JSONParser::RegisterParser(Parser* parser) {
    parsers[parser->GetNodeTypeName()] = parser;
}

} // End of json
} // End of trillek
