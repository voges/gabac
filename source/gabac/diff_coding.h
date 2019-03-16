#ifndef GABAC_DIFF_CODING_H_
#define GABAC_DIFF_CODING_H_



#include "gabac/data_block.h"


namespace gabac {


void transformDiffCoding(
        DataBlock *transformedSymbols
);


void inverseTransformDiffCoding(
        DataBlock *transformedSymbols
);


}  // namespace gabac


#endif  /* GABAC_DIFF_CODING_H_ */
