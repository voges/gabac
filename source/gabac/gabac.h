/* C interface main include */

#ifndef GABAC_GABAC_H_
#define GABAC_GABAC_H_

/* General */
#include "gabac/constants.h"
#include "gabac/exceptions.h"
#include "gabac/release.h"

/* Encode / Decode */
#include "gabac/decoding.h"
#include "gabac/encoding.h"
#include "gabac/configuration.h"

/* Transformations */
#include "gabac/diff_coding.h"
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

/* io */
#include "gabac/input_stream.h"
#include "gabac/buffer_input_stream.h"
#include "gabac/file_input_stream.h"
#include "gabac/output_stream.h"
#include "gabac/buffer_output_stream.h"
#include "gabac/file_output_stream.h"
#include "gabac/data_block.h"

#endif /* GABAC_GABAC_H_ */
