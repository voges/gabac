#ifndef GABAC_TEST_COMMON_H_
#define GABAC_TEST_COMMON_H_


#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace gabac {
class DataBlock;
}

void fillVectorRandomUniform(uint64_t min, uint64_t max, gabac::DataBlock *vector);
void fillVectorRandomGeometric(gabac::DataBlock *vector);


#endif  // GABAC_TEST_COMMON_H_
