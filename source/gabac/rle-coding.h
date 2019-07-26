#ifndef GABAC_RLE_CODING_H_
#define GABAC_RLE_CODING_H_

#include <cstdint>

#include "data-block.h"

namespace gabac {

void transformRleCoding(uint64_t guard, DataBlock *rawValues,
                        DataBlock *lengths);

void inverseTransformRleCoding(uint64_t guard, DataBlock *rawValues,
                               DataBlock *lengths);

}  // namespace gabac

#endif  // GABAC_RLE_CODING_H_
