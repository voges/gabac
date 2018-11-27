#ifndef GABAC_RLE_CODING_H_
#define GABAC_RLE_CODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_transformRleCoding(
        const uint64_t *symbols,
        size_t symbolsSize,
        uint64_t guard,
        uint64_t **rawValues,
        size_t *rawValuesSize,
        uint64_t **lengths,
        size_t *lengthsSize
);


int gabac_inverseTransformRleCoding(
        const uint64_t *rawValues,
        size_t rawValuesSize,
        const uint64_t *lengths,
        size_t lengthsSize,
        uint64_t guard,
        uint64_t **symbols,
        size_t *symbolsSize
);


#ifdef __cplusplus
}  // extern "C"


#include <cstdint>
#include <vector>


namespace gabac {


void transformRleCoding(
        const std::vector<uint64_t>& symbols,
        uint64_t guard,
        std::vector<uint64_t> *rawValues,
        std::vector<uint64_t> *lengths
);


void inverseTransformRleCoding(
        const std::vector<uint64_t>& rawValues,
        const std::vector<uint64_t>& lengths,
        uint64_t guard,
        std::vector<uint64_t> *symbols
);


}  // namespace gabac


#endif  /* __cplusplus */
#endif  /* GABAC_RLE_CODING_H_ */
