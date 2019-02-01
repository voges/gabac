#ifndef GABAC_EQUALITY_CODING_H_
#define GABAC_EQUALITY_CODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_transformEqualityCoding(
        const uint64_t *symbols,
        size_t symbolsSize,
        uint64_t **equalityFlags,
        uint64_t **values,
        size_t *valuesSize
);


int gabac_inverseTransformEqualityCoding(
        const uint64_t *equalityFlags,
        size_t equalityFlagsSize,
        const uint64_t *values,
        size_t valuesSize,
        uint64_t **symbols
);


#ifdef __cplusplus
}  // extern "C"


#include "gabac/data_stream.h"


namespace gabac {


void transformEqualityCoding(
        const gabac::DataStream& symbols,
        gabac::DataStream *equalityFlags,
        gabac::DataStream *values
);


void inverseTransformEqualityCoding(
        const gabac::DataStream& equalityFlags,
        const gabac::DataStream& values,
        gabac::DataStream *symbols
);


}  // namespace gabac


#endif  /* __cplusplus */


#endif  /* GABAC_EQUALITY_CODING_H_ */
