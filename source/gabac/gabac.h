/* C interface main include */

#ifndef GABAC_GABAC_H_
#define GABAC_GABAC_H_

#ifdef __cplusplus

/* General */
#include "gabac/constants.h"
#include "gabac/exceptions.h"

/* Encode / Decode */
#include "gabac/configuration.h"
#include "gabac/decoding.h"
#include "gabac/encoding.h"

/* io */
#include "gabac/block_stepper.h"
#include "gabac/data_block.h"
#include "gabac/streams.h"

#else

#include "gabac/c_interface.h"

#endif

#endif /* GABAC_GABAC_H_ */
