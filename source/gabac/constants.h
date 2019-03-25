#ifndef GABAC_CONSTANTS_H_
#define GABAC_CONSTANTS_H_

#include <functional>
#include <string>
#include <vector>

namespace gabac {

class DataBlock;

/**
 * Supported transformations
 */
enum class SequenceTransformationId
{
    no_transform = 0,  /**< Do nothing */
    equality_coding = 1,  /**< Find equal values sequentially */
    match_coding = 2,  /**< Find larger sequence matches */
    rle_coding = 3,  /**< Find run lengths */
    lut_transform = 4,  /**< Remap symbols based on probability */
    diff_coding = 5,  /**< Use differences between symbol values instead of symbols */
    cabac_coding = 6  /**< Entropy coding based on cabac */
};

/**
 * Supported binarization
 */
enum class BinarizationId
{
    BI = 0,  /**< Binary */
    TU = 1,  /**< Truncated Unary */
    EG = 2,  /**< Exponential Golomb */
    SEG = 3,  /**< Signed Exponential Golomb */
    TEG = 4,  /**< Truncated Exponential Golomb */
    STEG = 5  /**< Signed Truncated Exponential Golomb */
};

/**
 * Supported cabac contexts
 */
enum class ContextSelectionId
{
    bypass = 0,  /**< Do not use arithmetic coding */
    adaptive_coding_order_0 = 1,  /**< Current symbol only */
    adaptive_coding_order_1 = 2,  /**< Use current + previous symbol */
    adaptive_coding_order_2 = 3  /**< Use current + previous + before previous symbol */
};

/**
 * Transformation signature
 */
using SequenceTransform = std::function<void(const std::vector<uint64_t>& param,
                                             std::vector<gabac::DataBlock> *const
)>;

/**
 * Get property based on binarization parameter
 */
using SignedBinarizationBorder = std::function<uint64_t(uint64_t parameter
)>;

/**
 * Transformation meta data available to applications using gabac
 */
struct TransformationProperties
{
    std::string name;  /**< Name of transformation */
    std::vector<std::string> paramNames;  /**< Name of every parameter */
    std::vector<std::string> streamNames;  /**< Name of every stream */
    std::vector<uint8_t> wordsizes;  /**< Wordsizes of every stream. Zero means word size of input data */
    SequenceTransform transform;  /**< Function pointer to transformation */
    SequenceTransform inverseTransform;  /**< Function pointer to inverse transformation */
};

struct BinarizationProperties
{
    std::string name;  /**< Name of binarization */
    int64_t paramMin;  /**< Minimum value for parameter */
    int64_t paramMax;  /**< Maximum value for parameter */
    bool isSigned;  /**< If this supports signed symbols */
    SignedBinarizationBorder min;  /**< Minimum supported symbol with a particular parameter */
    SignedBinarizationBorder max;  /**< Maximum supported symbol with a particular parameter */

    /**
     * Check if a stream with the minimum and maximum values minv and maxv is supported
     * by this binarization with some parameter
     * @param minv Minimum symbol of stream
     * @param maxv Maximum symbol of stream
     * @param parameter Binarization parameter
     * @return True if the stream is supported
     * @note For signed binarization you can convert negative values to uint64_t. It will be
     * converted back internally
     */
    bool sbCheck(uint64_t minv, uint64_t maxv, uint64_t parameter) const;
};

/**
 * Get the meta information of one transformation
 * @param id Transformation ID
 * @return Properties object
 */
const TransformationProperties& getTransformation(const gabac::SequenceTransformationId& id);

/**
 * Get the meta information of one binarization
 * @param id Binarization ID
 * @return Properties object
 */
const BinarizationProperties& getBinarization(const gabac::BinarizationId& id);


}  // namespace gabac


#endif  // GABAC_CONSTANTS_H_
