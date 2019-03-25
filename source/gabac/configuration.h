#ifndef GABAC_CONFIGURATION_H_
#define GABAC_CONFIGURATION_H_

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace gabac {

enum class BinarizationId;
enum class ContextSelectionId;
enum class SequenceTransformationId;

/**
 * Parameters for single stream
 */
struct TransformedSequenceConfiguration
{
    bool lutTransformationEnabled;  /**< LUT transformation switch */
    unsigned int lutBits;  /**< Bits/values in LUT table */
    unsigned int lutOrder;  /**< Context size for LUT */
    bool diffCodingEnabled;  /**< Diff coding switch */
    gabac::BinarizationId binarizationId;  /**< Which binarization to use */
    std::vector<unsigned int> binarizationParameters;  /**< Parameters for binarization */
    gabac::ContextSelectionId contextSelectionId;  /**< Which context to use in CABAC */

    /**
     * Create a human readable string from this config
     * @return Text
     */
    std::string toPrintableString() const;
};


class EncodingConfiguration
{
 public:
    /**
     * Create default config
     */
    EncodingConfiguration();

    /**
     * Create config from JSON
     * @param jsonstring JSON encoded configuration
     */
    explicit EncodingConfiguration(
            const std::string& jsonstring
    );

    /**
     * Destroy config
     */
    ~EncodingConfiguration();

    /**
     * Encode as json
     * @return JSON string
     */
    std::string toJsonString() const;

    /**
     * Generate human readable text
     * @return Text
     */
    std::string toPrintableString() const;

    unsigned int wordSize;  /**< How many bytes are considered one symbol */
    gabac::SequenceTransformationId sequenceTransformationId;  /**< Which transformation to apply to the input stream */
    unsigned int sequenceTransformationParameter;  /**< Parameter for input stream transformation */
    std::vector<TransformedSequenceConfiguration> transformedSequenceConfigurations;  /**< Instructions for generated streams */
};

struct IOConfiguration
{
    std::istream *inputStream;  /**< Where to read from */
    std::ostream *outputStream;  /**< Where to write to */
    size_t blocksize;  /**< How many bytes to read at once. Put 0 to read all in one go */

    std::ostream *logStream;  /**< Where to write logging information*/

    /**
     * Logging level config
     */
    enum class LogLevel
    {
        TRACE = 0,  /**< Log every step in great detail */
        DEBUG = 1,  /**< Intermediate results */
        INFO = 2,  /**< Expected Results */
        WARNING = 3,  /**< Suspicious events (may be an error) */
        ERROR = 4,  /**< Handled errors */
        FATAL = 5   /**< Error causing application to terminate */
    };
    LogLevel level;  /**< Selected level */

    /**
     * Returns the logging stream if the provided level is higher
     * than the currently selected level and a null stream otherwise
     * @param l Logging level of the message you want to write.
     * @return The appropriate stream
     * @example Usage: log(LogLevel::FATAL) << "N=NP" << std::endl;
     */
    std::ostream& log(const LogLevel& l) const;

    /**
     * Check if all streams are open and working
     */
    void validate() const;
};

/**
 * Candidates for analysis
 */
struct AnalysisConfiguration
{
    std::vector<unsigned> candidateWordsizes;  /**< Which word sizes to test */
    std::vector<gabac::SequenceTransformationId> candidateSequenceTransformationIds;  /**<  Which transformations to test */
    std::vector<uint32_t> candidateMatchCodingParameters;  /**< Which match coding window sizes to test */
    std::vector<uint32_t> candidateRLECodingParameters;  /**< Which RLE guards to test */
    std::vector<bool> candidateLUTCodingParameters;  /**< Which LUT states (true, false) to test */
    std::vector<bool> candidateDiffParameters;  /**< Which diff states (true, false) to test */
    std::vector<gabac::BinarizationId> candidateUnsignedBinarizationIds;  /**< Which unsigned bins to test */
    std::vector<gabac::BinarizationId> candidateSignedBinarizationIds;  /**< Which signed bins to test */
    std::vector<unsigned> candidateBinarizationParameters;  /**< Which bin parameters to test */
    std::vector<gabac::ContextSelectionId> candidateContextSelectionIds;  /**< Which cabac contexts to test */
    std::vector<unsigned> candidateLutOrder;  /**< Which LUT orders to test */
};

}  // namespace gabac

#endif  // GABAC_CONFIGURATION_H_
