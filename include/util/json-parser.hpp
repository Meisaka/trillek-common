#ifndef JSON_PARSER_HPP_INCLUDED
#define JSON_PARSER_HPP_INCLUDED

#include <memory>
#include <map>
#include <vector>
#include <list>
#include <rapidjson/document.h>
#include <string>
#include <functional>

#include "property.hpp"
#include "logging.hpp"
#include "components/component-factory.hpp"

namespace trillek {
namespace resource {

class TextFile;

} // End of resource

namespace util {

std::string MakeString(const rapidjson::Value& v);

class Parser {
protected:
    Parser(std::string name) : node_type_name(name) { }
public:
    /**
     * \brief Serialize this object to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document) = 0;

    /**
     * \brief Parses this object from the provided JSON node.
     *
     * \param[in] rapidjson::Value& node The node to parse.
     * \return bool False if an error occured in parsing.
     */
    virtual bool Parse(rapidjson::Value& node) = 0;

    /**
     * \brief Gets the name of this node type.
     *
     * This type name matches the one the top-level JSON node.
     * \return std::string Node type name for this parser.
     */
    std::string GetNodeTypeName() {
        return this->node_type_name;
    }
private:
    std::string node_type_name;
};

struct JSONDocument {
    std::shared_ptr<resource::TextFile> file; // The loaded TextFile. Usefull if the file is reloaded.
    rapidjson::Document document; // Currently parsed document.
};

// Stores a parsed json entity's component object.
struct JSONComponent {
    std::vector<Property> properties;
    std::string component_name;
};

// Stores a parsed json entity object.
struct JSONEntity {
    std::map<std::string, std::shared_ptr<Property>> meta_properties; // ID, name, etc
    std::vector<std::shared_ptr<JSONComponent>> components;
    rapidjson::Document* document; // The document this entity was parsed from.
};

class JSONParser final {
public:
    JSONParser();

    void ParseJSONEntity(rapidjson::Value& node);
    std::shared_ptr<JSONComponent> ParseJSONComponent(rapidjson::Value& node);

    /**
     * \brief Parses a JSON file with the given filename.
     *
     * A TextFile resource is created with the filename if it doesn't exist already.
     * \param[in] const std::string& fname The filename of the JSON file to load.
     * \return bool True if parsing was successfully.
     */
    bool Parse(const std::string& fname);

    /**
     * \brief Serializes each parser node type to disk.
     *
     * If there isn't a supplied filename then each type is serialized as "node_type_name.json
     * \param[in] const std::string& out_directory The directory in which to place the file(s).
     * \param[in] const std::string& fname The filename of where to serialize the JSON.
     * \return void
     */
    void Serialize(const std::string& out_directory, const std::string& fname, std::shared_ptr<Parser> parser);

    /**
     * \brief Registers a serializer type.
     *
     * This method will call GetName() on the serializer to get its type name. If a type
     * with the same name exists it is overridden.
     * \param[in] std::shared_ptr<Parser> serializer The serializer to register.
     */
    static void RegisterParser(std::shared_ptr<Parser> parser);

    /**
     * \brief Registers a serializer type.
     *
     * This method will call GetName() on the serializer to get its type name. If a type
     * with the same name exists it is overridden.
     * \param[in] const Parser& serializer The serializer to register.
     */
    static void RegisterParser(Parser* parser);

    /**
     * \brief Registers all parsers as defined in the method body.
     *
     * This function is defined in a separate source file to reduce compile times.
     * Place each parser to be register at compile time in the method body.
     * Interally it just calls the tempalte method RegisterParser().
     * \return void
     */
    static void RegisterTypes();
    void RegisterComponentTypes();

    /**
    * \brief Register a type to be available for factory calls.
    *
    * \return void
    */
    template<ComponentType cptype, Component C, class T>
    void RegisterComponentType(ComponentFactory<cptype, C, T>&& adder) {
        // Store the type ID associated with the type name.
        LOGMSGC(DEBUG) << "adding factory of " << reflection::GetTypeName<std::integral_constant<Component, C>>();
        component_type_id[reflection::GetTypeName<std::integral_constant<Component, C>>()] = static_cast<uint32_t>(C);

        this->factories[static_cast<uint32_t>(C)] =
            std::bind(&ComponentFactory<cptype, C, T>::Create, adder, std::placeholders::_1, std::placeholders::_2);
    }

    /**
    * \brief Returns a type ID associated with the given name.
    *
    * \param[in] const std::string & type_Name The name to look for a type ID.
    * \return unsigned int Returns 0 if the name doesn't exist.
    */
    unsigned int GetTypeIDFromName(const std::string& type_Name) {
        if (!component_type_id.count(type_Name)) {
            LOGMSGC(ERROR) << "Could not find id type of " << type_Name;
            return 0;
        }
        return component_type_id.find(type_Name)->second;
    }
private:
    // Component creation
    std::map<std::string, unsigned int> component_type_id; // Stores a mapping of TypeName to TypeID
    std::map<unsigned int, std::function<bool(const unsigned int, const std::vector<Property> &properties)>> factories; // Mapping of type ID to factory function.

    std::vector<std::shared_ptr<JSONEntity>> entities;

    std::list<std::shared_ptr<JSONDocument>> documents;
    static std::map<std::string, Parser*> parsers; // Mapping of node_type_name to parser
};

} // End of json
} // End of trillek

#endif
