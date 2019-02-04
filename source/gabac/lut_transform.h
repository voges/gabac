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

/**
 *
 * @param symbols
 * @param symbolsSize
 * @param transformedSymbols
 * @param inverseLUT
 * @param inverseLUTSize
 * @return
 */
int gabac_transformLutTransform0(
        const uint64_t *symbols,
        size_t symbolsSize,
        uint64_t **transformedSymbols,
        uint64_t **inverseLUT,
        size_t *inverseLUTSize
);

/**
 *
 * @param transformedSymbols
 * @param transformedSymbolsSize
 * @param inverseLUT
 * @param inverseLUTSize
 * @param symbols
 * @return
 */
int gabac_inverseTransformLutTransform0(
        const uint64_t *transformedSymbols,
        size_t transformedSymbolsSize,
        const uint64_t *inverseLUT,
        size_t inverseLUTSize,
        uint64_t **symbols
);


#ifdef __cplusplus
}  // extern "C"

// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------

#include <utility>
#include <vector>

#include "gabac/data_stream.h"

namespace gabac {

/**
 *
 * @param symbols
 * @param transformedSymbols
 * @param inverseLUT
 */
void transformLutTransform0(
        unsigned order,
        DataStream *transformedSymbols,
        DataStream *inverseLUT,
        DataStream *inverseLUT1
);

/**
 *
 * @param transformedSymbols
 * @param inverseLUT
 * @param symbols
 */
void inverseTransformLutTransform0(
        unsigned order,
        const DataStream& transformedSymbols,
        const DataStream& inverseLUT,
        const DataStream& inverseLUT1,
        DataStream *symbols
);

}  // namespace gabac

// ----------------------------------------------------------------------------

#endif  /* __cplusplus */
#endif  /* GABAC_LUT_TRANSFORM_H_ */

/* ----------------------------------------------------------------------------
// --------------------------------------------------------------------------*/
