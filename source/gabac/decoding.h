/**
 *  @file decoding.h
 */


#ifndef GABAC_DECODING_H_
#define GABAC_DECODING_H_


#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/configuration.h"


namespace gabac {

class EncodingConfiguration;

ReturnCode decode_cabac(
        uint8_t wordsize,
        const BinarizationId& binarizationId,
        const std::vector<uint32_t>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *bitstream
);

void decode(
        const IOConfiguration& ioConf,
        const EncodingConfiguration& enConf
);


}  // namespace gabac

#endif  /* GABAC_DECODING_H_ */
