#ifndef GABAC_ENCODING_H_
#define GABAC_ENCODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_encode(
        int64_t *symbols,
        size_t symbolsSize,
        unsigned int binarizationId,
        unsigned int *binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        unsigned char **bitstream,
        size_t *bitstreamSize
);


#ifdef __cplusplus
}  // extern "C"


#include <vector>

#include "gabac/constants.h"
#include "gabac/data_stream.h"


namespace gabac {


int encode(
        const DataStream& symbols,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataStream *bitstream
);


}  // namespace gabac

#endif  /* __cplusplus */
#endif  /* GABAC_ENCODING_H_ */
