#ifndef PROJECT_C_INTERFACE_H
#define PROJECT_C_INTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Data block */

struct gabac_data_block
{
    uint8_t *values;
    size_t values_size;
    uint8_t word_size;
};

int gabac_data_block_init(
        gabac_data_block *block,
        size_t size,
        uint8_t wordsize
);

int gabac_data_block_release(
        gabac_data_block *block
);

int gabac_data_block_resize(
        gabac_data_block *block,
        size_t size
);

uint64_t gabac_data_block_get(
        const gabac_data_block *block,
        size_t index
);

void gabac_data_block_set(
        const gabac_data_block *block,
        size_t index,
        uint64_t val
);

/* Data block end*/

/* Constants */

enum gabac_return
{
    gabac_return_SUCCESS = 0,
    gabac_return_FAILURE = 1
};

enum gabac_log_level
{
    gabac_log_level_TRACE = 0,
    gabac_log_level_DEBUG = 1,
    gabac_log_level_INFO = 2,
    gabac_log_level_WARNING = 3,
    gabac_log_level_ERROR = 4,
    gabac_log_level_FATAL = 5
};

enum gabac_io_mode
{
    gabac_io_mode_FILE = 0,
    gabac_io_mode_BUFFER = 1,
};

enum gabac_transform
{

    gabac_transform_NONE = 0,
    gabac_transform_EQUALITY = 1,
    gabac_transform_MATCH = 2,
    gabac_transform_RLE = 3,
    gabac_transform_LUT = 4,
    gabac_transform_DIFF = 5,
    gabac_transform_CABAC = 6

};

extern const uint8_t gabac_transform_PARAM_NUM[];
extern const uint8_t gabac_transform_STREAM_NUM[];
extern const uint8_t gabac_transform_WORD_SIZES[][3];

enum gabac_operation
{
    gabac_operation_ENCODE = 0,
    gabac_operation_DECODE = 1,
    gabac_operation_ANALYZE = 2
};

enum gabac_binarization
{
    gabac_binarization_BI = 0,  /** Binary */
    gabac_binarization_TU = 1,  /** Truncated Unary */
    gabac_binarization_EG = 2,  /** Exponential Golomb */
    gabac_binarization_SEG = 3,  /** Signed Exponential Golomb */
    gabac_binarization_TEG = 4,  /** Truncated Exponential Golomb */
    gabac_binarization_STEG = 5  /** Signed Truncated Exponential Golomb */
};

enum gabac_context_select
{
    gabac_context_select_BYPASS = 0,
    gabac_context_select_ADAPTIVE_ORDER_0 = 1,
    gabac_context_select_ADAPTIVE_ORDER_1 = 2,
    gabac_context_select_ADAPTIVE_ORDER_2 = 3
};

/* Constants end */

/* Operations */

struct gabac_stream
{
    void *data;
    size_t size;
    gabac_io_mode input_mode;
};

int gabac_stream_create_file(
        gabac_stream *stream,
        const char *path,
        bool write
);

int gabac_stream_create_buffer(
        gabac_stream *stream,
        char *buffer,
        size_t buffer_size
);

int gabac_stream_release(
        gabac_stream *stream
);

struct gabac_io_config
{
    gabac_stream input;
    gabac_stream output;
    gabac_stream log;
    gabac_log_level log_level;

    size_t blocksize;
};

/* transform       | param0       | param1    | param2         | param3    | stream0(wordsize) | stream1(wordsize) | stream2(wordsize)   */
/* ------------------------------------------------------------------------------------------------------------------------------------- */
/* no_transform    | _            | _         | _              | _         | sequence(I)       | _                 | _                   */
/* equality_coding | _            | _         | _              | _         | raw_values(I)     | equality_flags(1) | _                   */
/* match_coding    | window_size  | _         | _              | _         | raw_values(I)     | pointers(4)       | lengths(4)          */
/* rle_coding      | guard        | _         | _              | _         | raw_values(I)     | run_lengths(4)    | _                   */
/* lut_transform   | order        | _         | _              | _         | sequence(I)       | order0_table(I)   | high_order_table(I) */
/* diff_coding     | _            | _         | _              | _         | sequence(I)       | _                 | _                   */
/* cabac_coding    | binarization | bin_param | context_select | wordsize* | sequence(I)       | _                 | _                   */
/*  --> I: input data word size; --> *: only needed for decoding;                                                                        */

int gabac_execute_tansform(
        size_t transformationID,
        uint64_t *param,
        bool inverse,
        gabac_data_block **input
);

int gabac_run(
        gabac_operation operation,
        gabac_io_config *io_config,
        char *config_json,
        size_t json_length
);

/* Operations end */

#ifdef __cplusplus
}
#endif


#endif //PROJECT_C_INTERFACE_H
