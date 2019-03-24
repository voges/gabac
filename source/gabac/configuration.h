#ifndef GABACIFY_CONFIGURATION_H_
#define GABACIFY_CONFIGURATION_H_

#include <istream>
#include <ostream>
#include <string>
#include <vector>


namespace gabac {

enum class BinarizationId;
enum class ContextSelectionId;
enum class SequenceTransformationId;

struct TransformedSequenceConfiguration
{
    bool lutTransformationEnabled;
    unsigned int lutBits;
    unsigned int lutOrder;
    bool diffCodingEnabled;
    gabac::BinarizationId binarizationId;
    std::vector<unsigned int> binarizationParameters;
    gabac::ContextSelectionId contextSelectionId;

    std::string toPrintableString() const;
};


class EncodingConfiguration
{
 public:
    EncodingConfiguration();

    explicit EncodingConfiguration(
            const std::string& json
    );

    ~EncodingConfiguration();

    std::string toJsonString() const;

    std::string toPrintableString() const;

    unsigned int wordSize;
    gabac::SequenceTransformationId sequenceTransformationId;
    unsigned int sequenceTransformationParameter;
    std::vector<TransformedSequenceConfiguration> transformedSequenceConfigurations;
};

struct IOConfiguration
{
    std::istream *inputStream;
    std::ostream *outputStream;
    size_t blocksize;

    std::ostream *logStream;

    enum class LogLevel
    {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5
    };

    LogLevel level;

    std::ostream& log(const LogLevel& l) const;

    void validate() const;
};

struct AnalysisConfiguration
{
    std::vector<unsigned> candidateWordsizes;
    std::vector<gabac::SequenceTransformationId> candidateSequenceTransformationIds;
    std::vector<uint32_t> candidateMatchCodingParameters;
    std::vector<uint32_t> candidateRLECodingParameters;
    std::vector<bool> candidateLUTCodingParameters;
    std::vector<bool> candidateDiffParameters;
    std::vector<gabac::BinarizationId> candidateUnsignedBinarizationIds;
    std::vector<gabac::BinarizationId> candidateSignedBinarizationIds;
    std::vector<unsigned> candidateBinarizationParameters;
    std::vector<gabac::ContextSelectionId> candidateContextSelectionIds;
    std::vector<unsigned> candidateLutOrder;
};


}  // namespace gabac


#endif  // GABACIFY_CONFIGURATION_H_
