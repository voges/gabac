#ifndef GABACIFY_CONFIGURATION_H_
#define GABACIFY_CONFIGURATION_H_


#include <string>
#include <vector>

#include "gabac/constants.h"


namespace gabacify {


struct TransformedSequenceConfiguration
{
    bool lutTransformationEnabled;
    unsigned int lutTransformationParameter;
    bool diffCodingEnabled;
    gabac::BinarizationId binarizationId;
    std::vector<unsigned int> binarizationParameters;
    gabac::ContextSelectionId contextSelectionId;

    std::string toPrintableString() const;
};


class Configuration
{
 public:
    Configuration();

    explicit Configuration(
            const std::string& json
    );

    ~Configuration();

    std::string toJsonString() const;

    std::string toPrintableString() const;

    unsigned int wordSize;
    gabac::SequenceTransformationId sequenceTransformationId;
    unsigned int sequenceTransformationParameter;
    std::vector<TransformedSequenceConfiguration> transformedSequenceConfigurations;
};


}  // namespace gabacify


#endif  // GABACIFY_CONFIGURATION_H_
