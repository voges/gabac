#ifndef GABAC_MATCH_CODING_H_
#define GABAC_MATCH_CODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */



int gabac_transformMatchCoding(
        const uint64_t *symbols,
        size_t symbolsSize,
        uint32_t windowSize,
        uint64_t **pointers,
        size_t *pointersSize,
        uint64_t **lengths,
        size_t *lengthsSize,
        uint64_t **rawValues,
        size_t *rawValuesSize
);


int gabac_inverseTransformMatchCoding(
        const uint64_t *pointers,
        size_t pointersSize,
        const uint64_t *lengths,
        size_t lengthsSize,
        const uint64_t *rawValues,
        size_t rawValuesSize,
        uint64_t **symbols,
        size_t *symbolsSize
);


#ifdef __cplusplus
}  // extern "C"


#include <cstdint>
#include <vector>

#include "gabac/data_stream.h"

namespace gabac {


void transformMatchCoding(
        uint32_t windowSize,
        DataStream *pointers,
        DataStream *lengths,
        DataStream *rawValues
);


void inverseTransformMatchCoding(
        const DataStream& pointers,
        const DataStream& lengths,
        const DataStream& rawValues,
        DataStream *symbols
);


}  // namespace gabac

#endif  /* __cplusplus */
#endif  /* GABAC_MATCH_CODING_H_ */
