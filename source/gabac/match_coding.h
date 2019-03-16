#ifndef GABAC_MATCH_CODING_H_
#define GABAC_MATCH_CODING_H_

#include <cstdint>
#include <vector>

#include "gabac/data_block.h"

namespace gabac {


void transformMatchCoding(
        uint32_t windowSize,
        DataBlock *rawValues,
        DataBlock *pointers,
        DataBlock *lengths
);


void inverseTransformMatchCoding(
        DataBlock *rawValues,
        DataBlock *pointers,
        DataBlock *lengths
);


}  // namespace gabac

#endif  /* GABAC_MATCH_CODING_H_ */
