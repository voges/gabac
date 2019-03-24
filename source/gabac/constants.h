#ifndef GABAC_CONSTANTS_H_
#define GABAC_CONSTANTS_H_

#include <functional>
#include <vector>

namespace gabac {

class DataBlock;

enum class ReturnCode
{
    success = 0,
    failure = 1
};

enum class SequenceTransformationId
{
    no_transform = 0,
    equality_coding = 1,
    match_coding = 2,
    rle_coding = 3,
    lut_transform = 4,
    diff_coding = 5,
    cabac_coding = 6
};

enum class BinarizationId
{
    BI = 0,  /** Binary */
    TU = 1,  /** Truncated Unary */
    EG = 2,  /** Exponential Golomb */
    SEG = 3,  /** Signed Exponential Golomb */
    TEG = 4,  /** Truncated Exponential Golomb */
    STEG = 5  /** Signed Truncated Exponential Golomb */
};

enum class ContextSelectionId
{
    bypass = 0,
    adaptive_coding_order_0 = 1,
    adaptive_coding_order_1 = 2,
    adaptive_coding_order_2 = 3
};

using SequenceTransform = std::function<void(const std::vector<uint64_t>& param,
                                             std::vector<gabac::DataBlock> *const
)>;

using SignedBinarizationBorder = std::function<uint64_t(uint64_t parameter
)>;


struct TransformationProperties
{
    std::string name;
    std::vector<std::string> paramNames;
    std::vector<std::string> streamNames;
    std::vector<uint8_t> wordsizes; // Wordsizes of output streams
    SequenceTransform transform; // Function for transformation
    SequenceTransform inverseTransform; // Function for inversetransformation
};

struct BinarizationProperties
{
    std::string name;
    int64_t paramMin;
    int64_t paramMax;
    bool isSigned;
    SignedBinarizationBorder min;
    SignedBinarizationBorder max;

    bool sbCheck(uint64_t minv, uint64_t maxv, uint64_t parameter) const;
};

const TransformationProperties& getTransformation (const gabac::SequenceTransformationId& id);
const BinarizationProperties& getBinarization (const gabac::BinarizationId& id);


}  // namespace gabac


#endif  /* GABAC_CONSTANTS_H_ */
