/**
 *  @file lut_transform.h
 */


#ifndef GABAC_LUT_TRANSFORM_H_
#define GABAC_LUT_TRANSFORM_H_


/* ----------------------------------------------------------------------------
// C wrapper BEGIN
// --------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* ------------------------------------------------------------------------- */

#include <stdlib.h> /* NOLINT */
#include <stdint.h> /* NOLINT */


#ifdef __cplusplus
}  // extern "C"

// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------

#include <utility>
#include <vector>

#include "gabac/data_block.h"

namespace gabac {

/**
 *
 * @param symbols
 * @param transformedSymbols
 * @param inverseLUT
 */
void transformLutTransform0(
        unsigned order,
        DataBlock *transformedSymbols,
        DataBlock *inverseLUT,
        DataBlock *inverseLUT1
);

/**
 *
 * @param transformedSymbols
 * @param inverseLUT
 * @param symbols
 */
void inverseTransformLutTransform0(
        unsigned order,
        DataBlock *symbols,
        DataBlock *inverseLUT,
        DataBlock *inverseLUT1
);

}  // namespace gabac

// ----------------------------------------------------------------------------

#endif  /* __cplusplus */
#endif  /* GABAC_LUT_TRANSFORM_H_ */

/* ----------------------------------------------------------------------------
// --------------------------------------------------------------------------*/
