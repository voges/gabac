#include "configuration.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "exceptions.h"

#include "gabac/input_stream.h"

namespace gabac {


EncodingConfiguration::EncodingConfiguration()
        : wordSize(0),
        sequenceTransformationId(gabac::SequenceTransformationId::no_transform),
        sequenceTransformationParameter(0),
        transformedSequenceConfigurations(){
    // Nothing to do here
}


EncodingConfiguration::EncodingConfiguration(
        const std::string& json
){
    try
    {
        // Read the stringstream JSON data to a property tree
        std::stringstream tmp(json);
        boost::property_tree::ptree propertyTree;
        boost::property_tree::read_json(tmp, propertyTree);

        // Convert the property tree contents to our internal structure
        this->wordSize
                = propertyTree.get<unsigned int>("word_size");
        this->sequenceTransformationId
                =
                static_cast<gabac::SequenceTransformationId>(propertyTree.get<unsigned int>("sequence_transformation_id"));
        this->sequenceTransformationParameter
                = propertyTree.get<unsigned int>("sequence_transformation_parameter");
        for (const auto& child : propertyTree.get_child("transformed_sequences")) {
            // Declare a transformed sequence configuration
            TransformedSequenceConfiguration transformedSequenceConfiguration;

            // Fill the transformed sequence configuration
            transformedSequenceConfiguration.lutTransformationEnabled
                    = static_cast<bool>(child.second.get<unsigned int>("lut_transformation_enabled"));
            if (transformedSequenceConfiguration.lutTransformationEnabled) {
                transformedSequenceConfiguration.lutBits
                        = child.second.get<unsigned int>("lut_transformation_bits");
                transformedSequenceConfiguration.lutOrder
                        = child.second.get<unsigned int>("lut_transformation_order");
            } else {
                transformedSequenceConfiguration.lutBits = 0;
                transformedSequenceConfiguration.lutOrder = 0;
            }
            transformedSequenceConfiguration.diffCodingEnabled
                    = child.second.get<bool>("diff_coding_enabled");
            transformedSequenceConfiguration.binarizationId
                    = static_cast<gabac::BinarizationId>(child.second.get<unsigned int>("binarization_id"));
            for (const auto& grandchild : child.second.get_child("binarization_parameters"))
            {
                transformedSequenceConfiguration.binarizationParameters
                        .push_back(grandchild.second.get_value<unsigned int>());
            }
            transformedSequenceConfiguration.contextSelectionId
                    = static_cast<gabac::ContextSelectionId>(child.second.get<unsigned int>("context_selection_id"));

            // Append the filled transformed sequence configuration to our
            // list of transformed sequence configurations
            this->transformedSequenceConfigurations.push_back(transformedSequenceConfiguration);
        }
    }
    catch (const boost::property_tree::ptree_error& e)
    {
        GABAC_DIE("JSON parsing error: " + std::string(e.what()));
    }
}


EncodingConfiguration::~EncodingConfiguration() = default;


std::string EncodingConfiguration::toJsonString() const{
    std::string jsonString;

    try
    {
        // Set up a property tree
        boost::property_tree::ptree root;

        // Convert the internal structure to a property tree
        root.put(
                "word_size",
                this->wordSize
        );
        root.put(
                "sequence_transformation_id",
                static_cast<int>(this->sequenceTransformationId)
        );
        root.put(
                "sequence_transformation_parameter",
                static_cast<int>(this->sequenceTransformationParameter)
        );
        boost::property_tree::ptree transformedSequencesNode;
        for (const auto& transformedSequenceConfiguration : this->transformedSequenceConfigurations)
        {
            // Declare a property tree for a transformed sequence configuration
            boost::property_tree::ptree transformedSequenceNode;

            // Fill the property tree for the transformed sequence
            // configuration
            transformedSequenceNode.put(
                    "lut_transformation_enabled",
                    static_cast<int>(transformedSequenceConfiguration.lutTransformationEnabled)
            );
            if(transformedSequenceConfiguration.lutTransformationEnabled) {
                transformedSequenceNode.put(
                        "lut_transformation_bits",
                        static_cast<int>(transformedSequenceConfiguration.lutBits)
                );
                transformedSequenceNode.put(
                        "lut_transformation_order",
                        static_cast<int>(transformedSequenceConfiguration.lutOrder)
                );
            }
            transformedSequenceNode.put(
                    "diff_coding_enabled",
                    static_cast<int>(transformedSequenceConfiguration.diffCodingEnabled)
            );
            transformedSequenceNode.put(
                    "binarization_id",
                    static_cast<int>(transformedSequenceConfiguration.binarizationId)
            );
            boost::property_tree::ptree binarizationParametersNode;
            for (const auto& binarizationParameter : transformedSequenceConfiguration.binarizationParameters)
            {
                boost::property_tree::ptree tmp;
                tmp.put("", binarizationParameter);
                binarizationParametersNode.push_back(std::make_pair("", tmp));
            }
            transformedSequenceNode.add_child(
                    "binarization_parameters",
                    binarizationParametersNode
            );
            transformedSequenceNode.put(
                    "context_selection_id",
                    static_cast<int>(transformedSequenceConfiguration.contextSelectionId)
            );

            // Add the filled property tree for the transformed sequence
            // configuration to our tree
            transformedSequencesNode.push_back(std::make_pair("", transformedSequenceNode));
        }
        root.push_back(std::make_pair("transformed_sequences", transformedSequencesNode));

        // Convert the property tree to a JSON string
        std::stringstream s;
        boost::property_tree::write_json(s, root);
        jsonString = s.str();
    }
    catch (const boost::property_tree::ptree_error& e)
    {
        GABAC_DIE("JSON write error: " + std::string(e.what()));
    }

    return jsonString;
}


std::string EncodingConfiguration::toPrintableString() const{
    std::stringstream s;

    s << this->wordSize << "  |  ";
    s << static_cast<int>(this->sequenceTransformationId) << "  |  ";
    s << this->sequenceTransformationParameter << "  |  ";
    for (const auto& transformedSequenceConfiguration : this->transformedSequenceConfigurations)
    {
        s << transformedSequenceConfiguration.toPrintableString();
    }

    return s.str();
}

std::string TransformedSequenceConfiguration::toPrintableString() const{
    std::stringstream s;
    s << "[";
    s << static_cast<int>(lutTransformationEnabled) << "  |  ";
    s << static_cast<int>(lutBits) << "  |  ";
    s << static_cast<int>(lutOrder) << "  |  ";
    s << static_cast<int>(diffCodingEnabled) << "  |  ";
    s << static_cast<int>(binarizationId) << "  |  ";
    s << "[ ";
    for (const auto& binarizationParameter : binarizationParameters)
    {
        s << binarizationParameter << " ";
    }
    s << "]  |  ";
    s << static_cast<int>(contextSelectionId);
    s << "]";
    return s.str();
}

void IOConfiguration::validate() const{
    if(!inputStream){
        GABAC_DIE("Invalid input stream");
    }
    if(!outputStream){
        GABAC_DIE("Invalid output stream");
    }
    if(!outputStream) {
        GABAC_DIE("Invalid logging output stream");
    }
    if(unsigned(level) > unsigned(IOConfiguration::LogLevel::FATAL)){
        GABAC_DIE("Invalid logging level");
    }
}


}  // namespace gabac
