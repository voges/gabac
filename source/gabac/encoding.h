#ifndef GABAC_ENCODING_H_
#define GABAC_ENCODING_H_

#include <vector>
#include <limits>

#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/configuration.h"


namespace gabac {


ReturnCode encode_cabac(
        const BinarizationId& binarizationId,
        const std::vector<uint32_t>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *symbols,
        size_t maxsize = std::numeric_limits<size_t>::max()
);

void encode(
        const IOConfiguration& ioConf,
        const EncodingConfiguration& enConf
);

}  // namespace gabac

#endif  /* GABAC_ENCODING_H_ */
