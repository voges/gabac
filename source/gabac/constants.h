#ifndef GABAC_CONSTANTS_H_
#define GABAC_CONSTANTS_H_

#ifdef __cplusplus

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
}  // namespace gabac
#endif

#endif  /* GABAC_CONSTANTS_H_ */
