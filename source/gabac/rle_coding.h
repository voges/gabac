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

#include "gabac/data_stream.h"

namespace gabac {


void transformRleCoding(
        uint64_t guard,
        DataStream *rawValues,
        DataStream *lengths
);


void inverseTransformRleCoding(
        const uint64_t guard,
        gabac::DataStream* const rawValues,
        gabac::DataStream* const lengths
);


}  // namespace gabac


#endif  /* __cplusplus */
#endif  /* GABAC_RLE_CODING_H_ */
