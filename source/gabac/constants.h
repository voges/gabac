#ifndef GABAC_CONSTANTS_H_
#define GABAC_CONSTANTS_H_

#ifdef __cplusplus

#include <cstdint>
#include <functional>
#include <vector>
#include <string>

namespace gabac {


enum class SequenceTransformationId
#else
    enum SequenceTransformationId
#endif
{
    no_transform = 0,
    equality_coding = 1,
    match_coding = 2,
    rle_coding = 3
};

#ifdef __cplusplus
enum class BinarizationId
#else
    enum BinarizationId
#endif
{
    BI = 0,  /** Binary */
    TU = 1,  /** Truncated Unary */
    EG = 2,  /** Exponential Golomb */
    SEG = 3,  /** Signed Exponential Golomb */
    TEG = 4,  /** Truncated Exponential Golomb */
    STEG = 5  /** Signed Truncated Exponential Golomb */
};

#ifdef __cplusplus
enum class ContextSelectionId
#else
    enum ContextSelectionId
#endif
{
    bypass = 0,
    adaptive_coding_order_0 = 1,
    adaptive_coding_order_1 = 2,
    adaptive_coding_order_2 = 3
};

#ifdef __cplusplus


using SequenceTransform = std::function<void(const std::vector<uint64_t>& sequence, const uint64_t param,
                                             std::vector<std::vector<uint64_t>> *const
)>;

using InverseSequenceTransform = std::function<void(const std::vector<std::vector<uint64_t>>&, const uint64_t,
                                                    std::vector<uint64_t> *const
)>;

using SignedBinarizationCheck = std::function<bool(int64_t min, int64_t max, uint64_t parameter
)>;

using UnsignedBinarizationCheck = std::function<bool(uint64_t min, uint64_t max, uint64_t parameter
)>;


struct TransformationProperties
{
    std::string name;
    std::vector<std::string> streamNames;
    std::vector<unsigned int> wordsizes; // Wordsizes of output streams
    SequenceTransform transform; // Function for transformation
    InverseSequenceTransform inverseTransform; // Function for inversetransformation
};

struct BinarizationProperties
{
    std::string name;
    bool isSigned;
    SignedBinarizationCheck sbCheck;
    UnsignedBinarizationCheck ubCheck;
};

extern const std::vector<TransformationProperties> transformationInformation;
extern const std::vector<BinarizationProperties> binarizationInformation;

std::vector<unsigned> fixWordSizes(const std::vector<unsigned>& list, unsigned wordsize);

}  // namespace gabac

#endif


#endif  /* GABAC_CONSTANTS_H_ */
