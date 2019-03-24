#ifndef PROJECT_DECODE_CABAC_H
#define PROJECT_DECODE_CABAC_H

#include <cstdint>
#include <vector>

namespace gabac {

enum class ReturnCode;
enum class BinarizationId;
enum class ContextSelectionId;
class DataBlock;


ReturnCode decode_cabac(
        const BinarizationId& binarizationId,
        const std::vector<uint32_t>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        uint8_t wordsize,
        DataBlock *bitstream
);

}

#endif //PROJECT_DECODE_CABAC_H
