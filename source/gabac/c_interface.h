#ifndef PROJECT_C_INTERFACE_H
#define PROJECT_C_INTERFACE_H

struct gabac_data_block
{
    uint8_t *values;
    size_t values_size;
    uint8_t word_size;
};

enum gabac_log_level
{
    gabac_log_TRACE = 0,
    gabac_log_DEBUG = 1,
    gabac_log_INFO = 2,
    gabac_log_WARNING = 3,
    gabac_log_ERROR = 4,
    gabac_log_FATAL = 5
};

enum gabac_io_mode
{
    gabac_file = 0,
    gabac_buffer = 1,
};

struct gabac_io_config
{
    void *input;
    gabac_io_mode input_mode;

    void *output;
    gabac_io_mode output_mode;

    void *log;
    gabac_io_mode log_mode;

    size_t blocksize;
    gabac_log_level level;
};

enum gabac_return_code
{
    gabac_return_success = 0,
    gabac_return_failure = 1
};

enum gabac_sequence_transform
{
    no_transform = 0,
    equality_coding = 1,
    match_coding = 2,
    rle_coding = 3
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

enum gabac_context_selection_id
{
    gabac_context_selection_bypass = 0,
    gabac_context_selection_adaptive_coding_order_0 = 1,
    gabac_context_selection_adaptive_coding_order_1 = 2,
    gabac_context_selection_adaptive_coding_order_2 = 3
};


int gabac_transformEqualityCoding(
        gabac_data_block *values,
        gabac_data_block *eq_flags
);


int gabac_inverseTransformEqualityCoding(
        gabac_data_block *values,
        gabac_data_block *eq_flags
);

int gabac_transformMatchCoding(
        uint32_t windowSize,
        gabac_data_block *raw_values,
        gabac_data_block *pointers,
        gabac_data_block *lengths
);


int gabac_inverseTransformMatchCoding(
        gabac_data_block *raw_values,
        gabac_data_block *pointers,
        gabac_data_block *lengths
);

int gabac_transformRleCoding(
        uint64_t guard,
        gabac_data_block *raw_values,
        gabac_data_block *lengths
);


int gabac_inverseTransformRleCoding(
        uint64_t guard,
        gabac_data_block *raw_values,
        gabac_data_block *lengths
);

int gabac_transformLutTransform(
        unsigned order,
        gabac_data_block *transformed_symbols,
        gabac_data_block *inverse_lut,
        gabac_data_block *inverse_lut1
);

int gabac_inverseTransformLutTransform(
        unsigned order,
        gabac_data_block *transformed_symbols,
        gabac_data_block *inverse_lut,
        gabac_data_block *inverse_lut1
);

int gabac_transformDiffCoding(
        gabac_data_block *transformedSymbols
);


int gabac_inverseTransformDiffCoding(
        gabac_data_block *transformedSymbols
);

int gabac_cabac_encode(
        gabac_binarization binarization_id,
        uint32_t *binarization_parameters,
        size_t binarization_parameters_size,
        gabac_context_selection_id context_selection_id,
        gabac_data_block *symbols
);

int gabac_cabac_decode(
        uint8_t wordsize,
        gabac_binarization binarizationId,
        uint32_t *binarization_parameters,
        size_t binarization_parameters_size,
        gabac_context_selection_id context_selection_id,
        gabac_data_block *symbols
);

int gabac_analyze(
        gabac_io_config *io_config,
        char *analysis_config_json
);

int gabac_encode(
        gabac_io_config *io_config
        char *encoding_config_json
);

int gabac_decode(
        gabac_io_config *io_config
        char *encoding_config_json
);


#endif //PROJECT_C_INTERFACE_H
