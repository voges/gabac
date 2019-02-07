#ifndef GABAC_DIFF_CODING_H_
#define GABAC_DIFF_CODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_transformDiffCoding(
        const uint64_t *symbols,
        size_t symbolsSize,
        int64_t **transformedSymbols
);


int gabac_inverseTransformDiffCoding(
        const int64_t *transformedSymbols,
        size_t transformedSymbolsSize,
        uint64_t **symbols
);


#ifdef __cplusplus
}  // extern "C"


#include "gabac/data_stream.h"


namespace gabac {


void transformDiffCoding(
        DataStream *transformedSymbols
);


void inverseTransformDiffCoding(
        DataStream *transformedSymbols
);


}  // namespace gabac


#endif  /* __cplusplus */
#endif  /* GABAC_DIFF_CODING_H_ */
