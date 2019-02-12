/**
 *  @file decoding.h
 */


#ifndef GABAC_DECODING_H_
#define GABAC_DECODING_H_


/* ----------------------------------------------------------------------------
// C wrapper BEGIN
// --------------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_decode(
        unsigned char *bitstream,
        size_t bitstreamSize,
        unsigned int binarizationId,
        unsigned int *binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        int64_t **symbols,
        size_t *symbolsSize
);


#ifdef __cplusplus
}  // extern "C"


// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------



#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/configuration.h"


namespace gabac {

struct EncodingConfiguration;
class InputStream;
class OutputStream;

int decode_cabac(
        const uint8_t wordsize,
        const BinarizationId& binarizationId,
        const std::vector<uint32_t>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *const bitstream
);

void decode(
        const IOConfiguration& ioConf,
        const EncodingConfiguration& enConf
);


}  // namespace gabac


#endif  /* __cplusplus */
#endif  /* GABAC_DECODING_H_ */
