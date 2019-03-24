#ifndef GABAC_EQUALITY_CODING_H_
#define GABAC_EQUALITY_CODING_H_

namespace gabac {

class DataBlock;

void transformEqualityCoding(
        DataBlock *values,
        DataBlock *equalityFlags
);


void inverseTransformEqualityCoding(
        DataBlock *values,
        DataBlock *equalityFlags
);


}  // namespace gabac


#endif  /* GABAC_EQUALITY_CODING_H_ */
