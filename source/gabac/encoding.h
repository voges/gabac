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
        uint32_t binarizationId,
        unsigned int *binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        unsigned char **bitstream,
        size_t *bitstreamSize
);


#ifdef __cplusplus
}  // extern "C"


#include <vector>
#include <limits>

#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/configuration.h"


namespace gabac {

class OutputStream;

class InputStream;

int encode_cabac(
        const BinarizationId& binarizationId,
        const std::vector<uint32_t>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *const symbols,
        size_t maxsize = std::numeric_limits<size_t>::max()
);

void encode(
        const IOConfiguration& ioConf,
        const EncodingConfiguration& enConf
);

}  // namespace gabac

#endif  /* __cplusplus */
#endif  /* GABAC_ENCODING_H_ */
