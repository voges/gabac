#ifndef GABAC_C_INTERFACE_H_
#define GABAC_C_INTERFACE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---- Data block ----*/

/**
 * Basic chunk of data
 */
typedef struct gabac_data_block
{
    /**
     * Actual data
     */
    uint8_t *values;

    /**
     * Number of elements (not bytes, except if word_size = 1!)
     */
    size_t values_size;

    /**
     * Number of bytes per element
     */
    uint8_t word_size;
} gabac_data_block;


/**
 * Allocates a new data block and copies the contents from a source memory location
 * @param block Block to analyze
 * @param data Location to copy. You can pass NULL to initialize with zeros instead
 * @param size Number of elements. Pass zero to create an empty data block.
 * @param wordsize Bytes per element. Valid values are 1, 2, 4 and 8.
 * @return gabac_return_SUCCESS if no error occurred, gabac_return_FAILURE otherwise
 */
int gabac_data_block_init(
        gabac_data_block *block,
        const void *data,
        size_t size,
        uint8_t wordsize
);

/**
 * Free all ressources allocated inside a data block
 * @param block Note that this pointe in particular won't be freed. Just the internal memory.
 * @return gabac_return_SUCCESS
 */
int gabac_data_block_release(
        gabac_data_block *block
);

/**
 * Resizes a data block. If the new size is bigger, the new space is not initiaized.
 * @param block
 * @param size New size
 * @return gabac_return_SUCCESS if no error occurred, gabac_return_FAILURE otherwise
 */
int gabac_data_block_resize(
        gabac_data_block *block,
        size_t size
);

/**
 * Swaps the contents of two data blocks without actually copying data
 * @param stream1
 * @param stream2
 * @return gabac_return_SUCCESS
 */
int gabac_data_block_swap(
        gabac_data_block *stream1,
        gabac_data_block *stream2
);

/**
 * Copy the contents to an other block.
 * @param drain Will be resized accordingly
 * @param source
 * @return gabac_return_SUCCESS if no error occurred, gabac_return_FAILURE otherwise
 */
int gabac_data_block_copy(
        gabac_data_block *drain,
        gabac_data_block *source
);

/**
 * Extract a single element from a data block
 * @param block
 * @param index Must be smaller than values_size
 * @return The element
 */
uint64_t gabac_data_block_get(
        const gabac_data_block *block,
        size_t index
);

/**
 * Set a single element in a data block
 * @param block
 * @param index Must be smaller than values_size
 * @param val New value
 */
void gabac_data_block_set(
        const gabac_data_block *block,
        size_t index,
        uint64_t val
);

/* Data block end*/

/* Constants */

/**
 * Return states
 */
typedef enum gabac_return
{
    gabac_return_SUCCESS = 0,
    gabac_return_FAILURE = 1
} gabac_return;

/**
 * Different logging urgency
 */
typedef enum gabac_log_level
{
    gabac_log_level_TRACE = 0,
    gabac_log_level_DEBUG = 1,
    gabac_log_level_INFO = 2,
    gabac_log_level_WARNING = 3,
    gabac_log_level_ERROR = 4,
    gabac_log_level_FATAL = 5
} gabac_log_level;

/**
 * Gabac available transformations
 */
typedef enum gabac_transform
{
    gabac_transform_NONE = 0,
    gabac_transform_EQUALITY = 1,
    gabac_transform_MATCH = 2,
    gabac_transform_RLE = 3,
    gabac_transform_LUT = 4,
    gabac_transform_DIFF = 5,
    gabac_transform_CABAC = 6
} gabac_transform;

/**
 * Contains the number of expected parameters for each gabac_transform
 */
extern const uint8_t gabac_transform_PARAM_NUM[];

/**
 * Contains the number of expected streams for each gabac_transform
 */
extern const uint8_t gabac_transform_STREAM_NUM[];

/**
 * Contains the expected word size for each stream in a gabac_transform. 255 means
 * that the word size will be equal to the input stream word size
 */
extern const uint8_t gabac_transform_WORD_SIZES[][3];

/**
 * gabac_run operations
 */
typedef enum gabac_operation
{
    gabac_operation_ENCODE = 0,
    gabac_operation_DECODE = 1,
    gabac_operation_ANALYZE = 2
} gabac_operation;

/**
 * Binarizations for cabac transformation
 */
typedef enum gabac_binarization
{
    gabac_binarization_BI = 0,  /** Binary */
    gabac_binarization_TU = 1,  /** Truncated Unary */
    gabac_binarization_EG = 2,  /** Exponential Golomb */
    gabac_binarization_SEG = 3,  /** Signed Exponential Golomb */
    gabac_binarization_TEG = 4,  /** Truncated Exponential Golomb */
    gabac_binarization_STEG = 5  /** Signed Truncated Exponential Golomb */
} gabac_binarization;

/**
 * Context selection modes for cabac transformation
 */
typedef enum gabac_context_select
{
    gabac_context_select_BYPASS = 0,
    gabac_context_select_ADAPTIVE_ORDER_0 = 1,
    gabac_context_select_ADAPTIVE_ORDER_1 = 2,
    gabac_context_select_ADAPTIVE_ORDER_2 = 3
} gabac_context_select;

/* Constants end */

/* Operations */

/**
 * Flags for different data types
 */
typedef enum gabac_stream_mode
{
    gabac_stream_mode_FILE = 0,
    gabac_stream_mode_BUFFER = 1
} gabac_stream_mode;

/**
 * An i/o-stream
 */
typedef struct gabac_stream
{
    /**
     * Will be a data_block* if input_mode == buffer and FILE* if input_mode == file
     */
    void *data;

    /**
     * Flag for data type
     */
    gabac_stream_mode input_mode;
} gabac_stream;

/**
 * Constant for stdout
 */
extern const char *gabac_stream_create_file_STDOUT;

/**
 * Constant for stderr
 */
extern const char *gabac_stream_create_file_STDERR;

/**
 * Constant for stdin
 */
extern const char *gabac_stream_create_file_STDIN;

/**
 * Constant for tmpfile
 */
extern const char *gabac_stream_create_file_TMP;

/**
 * Initialize a stream from a file
 * @param stream Where to initialize
 * @param filename Filename to open. You can also pass gabac_stream_create_file_*
 *        to open stdout, stderr, stdin or a tmpfile instead. Note that you need to
 *        call swap after using a tmpfile to read it. Otherwise it will be closed
 *        when released. Stdout is not closed.
 * @param filename_size Length of file name. Pass 0 when using a gabac constant.
 * @param write True to open in write mode instead of read mode
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_stream_create_file(
        gabac_stream *stream,
        const char *filename,
        size_t filename_size,
        int write
);

/**
 * Create a stream from a data block.
 * @param stream Where to initialize
 * @param block Initial stream content (will be swapped inside). Intended for input streams.
 *        You can also pass NULL for no initial contents.
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_stream_create_buffer(
        gabac_stream *stream,
        gabac_data_block *block
);

/**
 * Swap the block contents in a stream into an other data block. Does not work in file mode.
 * @param stream Stream
 * @param block External block
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_stream_swap_block(
        gabac_stream *stream,
        gabac_data_block *block
);

/**
 * Swap the file pointer in a stream with an other data block. Does not work in file mode.
 * @param stream Stream
 * @param block External block
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_stream_swap_file(
        gabac_stream *stream,
        FILE **file
);

/**
 * Release a stream (including associated files or buffers)
 * @param stream
 * @return gabac_return_SUCCESS
 */
int gabac_stream_release(
        gabac_stream *stream
);

/**
 * Stream i/o information
 */
typedef struct gabac_io_config
{
    /**
     * Where to get data from
     */
    gabac_stream input;

    /**
     * Where to write data to
     */
    gabac_stream output;

    /**
     * Where to output log
     */
    gabac_stream log;

    /**
     * Logging level
     */
    gabac_log_level log_level;

    /**
     * Block size. Put 0 for infinite block size
     */
    size_t blocksize;
} gabac_io_config;

/**
 * Execute a single transformation in gabac
 * transform       | param0       | param1    | param2         | param3    | stream0(wordsize) | stream1(wordsize) | stream2(wordsize)    /
 * -------------------------------------------------------------------------------------------------------------------------------------  /
 * no_transform    | _            | _         | _              | _         | sequence(I)       | _                 | _                    /
 * equality_coding | _            | _         | _              | _         | raw_values(I)     | equality_flags(1) | _                    /
 * match_coding    | window_size  | _         | _              | _         | raw_values(I)     | pointers(4)       | lengths(4)           /
 * rle_coding      | guard        | _         | _              | _         | raw_values(I)     | run_lengths(4)    | _                    /
 * lut_transform   | order        | _         | _              | _         | sequence(I)       | order0_table(I)   | high_order_table(I)  /
 * diff_coding     | _            | _         | _              | _         | sequence(I)       | _                 | _                    /
 * cabac_coding    | binarization | bin_param | context_select | wordsize* | sequence(I)       | _                 | _                    /
 *  --> I: input data word size; --> *: only needed for decoding;                                                                         /
 *
 * @param transformationID Which trasnformation to execute
 * @param param Array of parameters for the transformation
 * @param inverse If you want to do the inverse transformation instead
 * @param input Array of input data streams
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_execute_transform(
        uint8_t transformationID,
        const uint64_t *param,
        int inverse,
        gabac_data_block *input
);

/**
 * Execute a complete run of gabac
 * @param operation Which operation to execute
 * @param io_config
 * @param config_json Pointer to json string
 * @param json_length Length of json stream
 * @return gabac_return_SUCCESS or gabac_return_FAILURE if an error occurred
 */
int gabac_run(
        gabac_operation operation,
        gabac_io_config *io_config,
        const char *config_json,
        size_t json_length
);

/* Operations end */

#ifdef __cplusplus
}
#endif

#endif  /* GABAC_C_INTERFACE_H_ */
