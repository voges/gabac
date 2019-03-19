#include "gabac/c_interface.h"

#include <cstdlib>
#include <cstdio>

#include <memory>
#include <sstream>

#include "gabac/analysis.h"
#include "gabac/configuration.h"
#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/decoding.h"
#include "gabac/encoding.h"
#include "gabac/equality_coding.h"
#include "gabac/exceptions.h"
#include "gabac/stream_handler.h"


const uint8_t gabac_sequence_transform_params[] = {
        0,
        0,
        1,
        1,
        1,
        0,
        4
};
const uint8_t gabac_sequence_transform_streams[] = {
        1,
        2,
        3,
        2,
        3,
        1,
        1
};

const uint8_t gabac_sequence_transform_word_sizes[][3] ={
        {0, 255, 255},
        {0, 1, 255},
        {0, 4, 4},
        {0, 4, 255},
        {0, 0, 0},
        {0, 255, 255},
        {0, 255, 255}
};


int gabac_data_block_init(gabac_data_block* block, size_t size, uint8_t wordsize) {
    if(wordsize != 1 && wordsize != 2 && wordsize != 4 && wordsize != 8) {
        goto ERROR;
    }

    block->values = static_cast<uint8_t*>(malloc(wordsize * size * sizeof(uint8_t)));
    if(!block->values) {
        goto ERROR;
    }
    block->values_size = size;
    block->word_size = wordsize;
    return gabac_return_SUCCESS;

    ERROR:
    block->values = nullptr;
    block->values_size = 0;
    block->word_size = 0;
    return gabac_return_FAILURE;
}

int gabac_data_block_release(gabac_data_block* block) {
    free(block->values);
    block->values = nullptr;
    block->values_size = 0;
    block->word_size = 0;
    return gabac_return_SUCCESS;
}

int gabac_data_block_resize(gabac_data_block* block, size_t size) {
    block->values_size = size * block->word_size;
    block->values = static_cast<uint8_t*>(realloc(block->values, size * block->values_size));
    if(!block->values) {
        block->values_size = 0;
        block->word_size = 0;
        return gabac_return_FAILURE;
    }
    return gabac_return_SUCCESS;
}

uint64_t gabac_data_block_get(const gabac_data_block* block, size_t index) {
    switch (block->word_size) {
        case 1:
            return *(uint8_t *) (block->values + index);
        case 2:
            return *(uint16_t *) (block->values + (index << 1u));
        case 4:
            return *(uint32_t *) (block->values+ (index << 2u));
        case 8:
            return *(uint64_t *) (block->values + (index << 3u));
        default:
            return 0;
    }
}

void gabac_data_block_set(const gabac_data_block* block, size_t index, uint64_t val) {
    switch (block->word_size) {
        case 1:
            *(block->values + index) = static_cast<uint8_t>(val);
            return;
        case 2:
            *(uint16_t *) (block->values + (index << 1u)) = static_cast<uint16_t>(val);
            return;
        case 4:
            *(uint32_t *) (block->values + (index << 2u)) = static_cast<uint32_t>(val);
            return;
        case 8:
            *(uint64_t *) (block->values + (index << 3u)) = static_cast<uint64_t>(val);
            return;
        default:
            return;
    }
}


int gabac_stream_create_file(gabac_stream* stream, const char* path, bool write){
    const char* mode = "rb";
    if(write) {
        mode = "wb";
    }
    stream->data = fopen(path, mode);
    if(!stream->data) {
        return gabac_return_FAILURE;
    }
    stream->size = 0;
    stream->input_mode = gabac_io_mode_FILE;
    return gabac_return_SUCCESS;
}

int gabac_stream_create_buffer(gabac_stream* stream, char* buffer, size_t buffer_size){
    stream->data = buffer;
    stream->size = buffer_size;
    stream->input_mode = gabac_io_mode_BUFFER;
    return gabac_return_SUCCESS;
}

int gabac_stream_release(gabac_stream* stream){
    if(stream->input_mode == gabac_io_mode_BUFFER) {
        free(stream->data);
        stream->size = 0;
    } else {
        if(stream->data != stdout && stream->data != stderr && stream->data != stdin) {
            fclose(static_cast<FILE *>(stream->data));
        }
    }
    return gabac_return_SUCCESS;
}

int gabac_executeTransform(
        size_t transformationID,
        uint64_t *param,
        bool inverse,
        gabac_data_block** input
){
    try {
        std::vector<gabac::DataBlock> blocks(gabac::transformationInformation[transformationID].wordsizes.size());
        std::vector<uint64_t> params_vec(gabac_sequence_transform_params[transformationID]);
        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i] = gabac::DataBlock(input[i]->values, input[i]->values_size, input[i]->word_size);
            if (gabac_data_block_release(input[i])) {
                GABAC_THROW_RUNTIME_EXCEPTION("C interface error");
            }
        }

        for(size_t i = 0; i < params_vec.size(); ++i) {
            params_vec[i] = param[i];
        }

        if(inverse) {
            gabac::transformationInformation[transformationID].inverseTransform(params_vec, &blocks);
        } else {
            gabac::transformationInformation[transformationID].transform(params_vec, &blocks);
        }

        for (size_t i = 0; i < gabac::transformationInformation[transformationID].wordsizes.size(); ++i) {
            if (gabac_data_block_init(input[i], blocks[i].size(), blocks[i].getWordSize())) {
                GABAC_THROW_RUNTIME_EXCEPTION("C interface error");
            }
            memcpy(input[i]->values, blocks[i].getData(), blocks[i].getRawSize());
            blocks[i].clear();
            blocks[i].shrink_to_fit();
        }
        return gabac_return_SUCCESS;
    } catch(...) {
        return gabac_return_FAILURE;
    }

}

int gabac_run(
        gabac_operation operation,
        gabac_io_config *io_config,
        char *config_json,
        size_t json_length
) {
    try {
        gabac::IOConfiguration ioconf_cpp;
        ioconf_cpp.blocksize = io_config->blocksize;
        ioconf_cpp.level = static_cast<gabac::IOConfiguration::LogLevel>(io_config->log_level);
        std::unique_ptr<std::ostream> output;
        std::unique_ptr<std::ostream> log;
        std::string config(config_json, json_length);

        if (io_config->output.input_mode == gabac_io_mode_FILE) {
            output.reset(new gabac::OFileStream(static_cast<FILE *>(io_config->output.data)));
        } else {
            output.reset(new std::ostringstream());
        }
        ioconf_cpp.outputStream = output.get();

        if (io_config->log.input_mode == gabac_io_mode_FILE) {
            log.reset(new gabac::OFileStream(static_cast<FILE *>(io_config->log.data)));
        } else {
            log.reset(new std::ostringstream());
        }
        ioconf_cpp.logStream = log.get();

        {
            std::unique_ptr<std::istream> input;
            if (io_config->input.input_mode == gabac_io_mode_FILE) {
                input.reset(new gabac::IFileStream(static_cast<FILE *>(io_config->input.data)));
            } else {
                input.reset(
                        new std::istringstream(
                                std::string(
                                        static_cast<const char *>(io_config->input.data),
                                        io_config->input.size
                                )));
            }
            ioconf_cpp.inputStream = input.get();
            gabac::EncodingConfiguration enConf;
            gabac::AnalysisConfiguration analyseConfig;
            switch (operation) {
                case gabac_operation_ANALYZE:
                    // analyseConfig = ...
                    gabac::analyze(ioconf_cpp, analyseConfig);
                    break;
                case gabac_operation_ENCODE:
                    enConf = gabac::EncodingConfiguration(config);
                    gabac::encode(ioconf_cpp, enConf);
                    break;
                case gabac_operation_DECODE:
                    enConf = gabac::EncodingConfiguration(config);
                    gabac::decode(ioconf_cpp, enConf);
                    break;
            }
        }

        if (io_config->log.input_mode == gabac_io_mode_BUFFER) {
            std::ostringstream *strstr = static_cast<std::ostringstream *> (log.get());
            io_config->log.size = strstr->str().size();
            io_config->log.data = realloc(io_config->log.data, io_config->log.size);
            memcpy(io_config->log.data, strstr->str().data(), io_config->log.size);
            strstr->str("");
        }

        if (io_config->output.input_mode == gabac_io_mode_BUFFER) {
            std::ostringstream *strstr = static_cast<std::ostringstream *> (output.get());
            io_config->output.size = strstr->str().size();
            io_config->output.data = realloc(io_config->output.data, io_config->output.size);
            memcpy(io_config->output.data, strstr->str().data(), io_config->output.size);
            strstr->str("");
        }
        return gabac_return_SUCCESS;
    }catch(...) {
        return gabac_return_FAILURE;
    }
}