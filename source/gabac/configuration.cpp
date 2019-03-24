#include "gabac/configuration.h"

#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "gabac/constants.h"
#include "gabac/exceptions.h"
#include "gabac/stream_handler.h"
#include "gabac/streams.h"

namespace gabac {


EncodingConfiguration::EncodingConfiguration()
        : wordSize(0),
        sequenceTransformationId(gabac::SequenceTransformationId::no_transform),
        sequenceTransformationParameter(0),
        transformedSequenceConfigurations(){
    // Nothing to do here
}

EncodingConfiguration::EncodingConfiguration(
        const std::string& jsonstring
){
    using nlohmann::json;
    try {
        auto jtree = json::parse(jsonstring);
        this->wordSize = jtree["word_size"];
        this->sequenceTransformationId = jtree["sequence_transformation_id"];
        this->sequenceTransformationParameter = jtree["sequence_transformation_parameter"];

        for (const auto& child : jtree["transformed_sequences"]) {
            // Declare a transformed sequence configuration
            TransformedSequenceConfiguration transformedSequenceConfiguration;

            // Fill the transformed sequence configuration
            transformedSequenceConfiguration.lutTransformationEnabled = child["lut_transformation_enabled"];
            transformedSequenceConfiguration.lutBits = wordSize * 8;
            transformedSequenceConfiguration.lutOrder = 0;
            if (transformedSequenceConfiguration.lutTransformationEnabled) {
                if (child.count("lut_transformation_bits") > 0) {
                    transformedSequenceConfiguration.lutBits = child["lut_transformation_bits"];
                }
                if (child.count("lut_transformation_order") > 0) {
                    transformedSequenceConfiguration.lutOrder = child["lut_transformation_order"];
                }
            } else {
                transformedSequenceConfiguration.lutBits = 0;
                transformedSequenceConfiguration.lutOrder = 0;
            }
            transformedSequenceConfiguration.diffCodingEnabled = child["diff_coding_enabled"];
            transformedSequenceConfiguration.binarizationId = child["binarization_id"];
            for (const auto& grandchild : child["binarization_parameters"]) {
                transformedSequenceConfiguration.binarizationParameters.push_back(grandchild);
            }
            transformedSequenceConfiguration.contextSelectionId = child["context_selection_id"];

            // Append the filled transformed sequence configuration to our
            // list of transformed sequence configurations
            this->transformedSequenceConfigurations.push_back(transformedSequenceConfiguration);
        }
    }
    catch (json::exception& e) {
        GABAC_DIE("JSON parsing error: " + std::string(e.what()));
    }
}


EncodingConfiguration::~EncodingConfiguration() = default;


std::string EncodingConfiguration::toJsonString() const{
    using nlohmann::json;
    try {
        json root;
        root["word_size"] = this->wordSize;
        root["sequence_transformation_id"] = this->sequenceTransformationId;
        root["sequence_transformation_parameter"] = this->sequenceTransformationParameter;

        json sequenceList;
        for (const auto& transformedSequenceConfiguration : this->transformedSequenceConfigurations) {
            json curSequence;

            // Fill the property tree for the transformed sequence
            // configuration

            curSequence["lut_transformation_enabled"] = transformedSequenceConfiguration.lutTransformationEnabled;

            if (transformedSequenceConfiguration.lutTransformationEnabled) {
                curSequence["lut_transformation_bits"] = transformedSequenceConfiguration.lutBits;
                curSequence["lut_transformation_order"] = transformedSequenceConfiguration.lutOrder;
            }

            curSequence["diff_coding_enabled"] = transformedSequenceConfiguration.diffCodingEnabled;
            curSequence["binarization_id"] = transformedSequenceConfiguration.binarizationId;

            curSequence["binarization_parameters"] = transformedSequenceConfiguration.binarizationParameters;

            curSequence["context_selection_id"] = transformedSequenceConfiguration.contextSelectionId;

            sequenceList.push_back(curSequence);
        }
        root["transformed_sequences"] = sequenceList;
        return root.dump(4);
    }
    catch (json::exception& e) {
        GABAC_DIE("JSON parsing error: " + std::string(e.what()));
    }
}

std::string EncodingConfiguration::toPrintableString() const{
    std::stringstream s;

    s << this->wordSize << "  |  ";
    s << static_cast<int>(this->sequenceTransformationId) << "  |  ";
    s << this->sequenceTransformationParameter << "  |  ";
    for (const auto& transformedSequenceConfiguration : this->transformedSequenceConfigurations) {
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
    for (const auto& binarizationParameter : binarizationParameters) {
        s << binarizationParameter << " ";
    }
    s << "]  |  ";
    s << static_cast<int>(contextSelectionId);
    s << "]";
    return s.str();
}

void IOConfiguration::validate() const{
    if (!inputStream) {
        GABAC_DIE("Invalid input stream");
    }
    if (!outputStream) {
        GABAC_DIE("Invalid output stream");
    }
    if (!logStream) {
        GABAC_DIE("Invalid logging output stream");
    }
    if (unsigned(this->level) > unsigned(IOConfiguration::LogLevel::FATAL)){
        GABAC_DIE("Invalid logging level");
    }
}

std::ostream& IOConfiguration::log(const LogLevel& l) const{
    static NullStream nullstr;
    if (static_cast<int>(l) >= static_cast<int>(level)) {
        return *logStream;
    }
    return nullstr;
}


}  // namespace gabac
