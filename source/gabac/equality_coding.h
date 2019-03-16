#ifndef GABAC_EQUALITY_CODING_H_
#define GABAC_EQUALITY_CODING_H_

#include "gabac/data_block.h"


namespace gabac {


void transformEqualityCoding(
        gabac::DataBlock *values,
        gabac::DataBlock *equalityFlags
);


void inverseTransformEqualityCoding(
        DataBlock *values,
        DataBlock *equalityFlags
);


}  // namespace gabac


#endif  /* GABAC_EQUALITY_CODING_H_ */
