import os
import copy
import json
import time
import random
import ctypes as ct
from collections import OrderedDict

import numpy as np

from gabac_api import libgabac
from gabac_api import gabac_stream
from gabac_api import gabac_io_config
from gabac_api import gabac_data_block
from gabac_api import GABAC_BINARIZATION, GABAC_CONTEXT_SELECT, GABAC_LOG_LEVEL, GABAC_LOG_LEVEL
from gabac_api import GABAC_OPERATION, GABAC_RETURN, GABAC_STREAM_MODE, GABAC_TRANSFORM
from gabac_api import root_path

from gabac_api import array, libc, get_block_values, are_blocks_equal

class GabacConfiguration():
    # TODO: Improve code for parameter with numerical value because json cannot accept numpy

    transformed_seq_conf_template = OrderedDict(
        lut_transformation_enabled = [
            False, 
            True
        ],
        lut_transformation_parameter = 0,
        diff_coding_enabled = [
            False, 
            True
        ],
        binarization_id = [
            # GABAC_BINARIZATION.BI, 
            # GABAC_BINARIZATION.TU,
            GABAC_BINARIZATION.SEG,
            GABAC_BINARIZATION.EG,
            GABAC_BINARIZATION.STEG,
            GABAC_BINARIZATION.TEG
        ],
        # binarization_parameters = [ v.item() for v in np.power(2, np.arange(5)) ],
        binarization_parameters = [ v.item() for v in np.arange(32) ],
        context_selection_id = [
            GABAC_CONTEXT_SELECT.BYPASS,
            GABAC_CONTEXT_SELECT.ADAPTIVE_ORDER_0,
            GABAC_CONTEXT_SELECT.ADAPTIVE_ORDER_1,
            GABAC_CONTEXT_SELECT.ADAPTIVE_ORDER_2,
        ]
    )

    transformed_seq_parameter_value_as_list = [
        "binarization_parameters"
    ]

    main_conf_template = {
        # "word_size" : [
        #     # [ v.item() for v in np.power(2, np.arange(6)) ]
        #     # [ v.item() for v in np.power(2, np.arange(4)) ]
        #     v.item() for v in np.power(2, np.arange(3))
        # ],
        "word_size" : 1,
        "sequence_transformation_id" : None,
        "sequence_transformation_parameter" : None,
        "transformed_sequences" : None
    }

    equality_config_template = copy.deepcopy(main_conf_template)
    equality_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.EQUALITY
    equality_config_template["sequence_transformation_parameter"] = 0

    match_config_template = copy.deepcopy(main_conf_template)
    match_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.MATCH
    match_config_template["sequence_transformation_parameter"] = [ v.item() for v in np.power(2, np.arange(4)) ]
    
    rle_config_template = copy.deepcopy(main_conf_template)
    rle_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.RLE 
    rle_config_template["sequence_transformation_parameter"] = 255

    none_config_template = copy.deepcopy(main_conf_template)
    none_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.NONE 
    none_config_template["sequence_transformation_parameter"] = 0

    available_variant = {
        GABAC_TRANSFORM.NONE : none_config_template,
        GABAC_TRANSFORM.EQUALITY : equality_config_template,
        GABAC_TRANSFORM.MATCH : match_config_template,
        GABAC_TRANSFORM.RLE : rle_config_template,
    }

    num_transformed_seq = {
        GABAC_TRANSFORM.NONE : 1,
        GABAC_TRANSFORM.EQUALITY : 2,
        GABAC_TRANSFORM.MATCH : 3,
        GABAC_TRANSFORM.RLE : 2 }

    def __init__(
        self,
        seq_transform_id,
        ena_roundtrip=True,
    ):

        self.seq_transform_id = seq_transform_id
        self.ena_roundtrip = ena_roundtrip

        self.variant_config_template = self.available_variant[seq_transform_id]

        self.main_config_params_w_var_values = []
        for key in self.variant_config_template.keys():
            if isinstance(self.variant_config_template[key], list):
                self.main_config_params_w_var_values.append(
                    key
                )

        self.transformed_seq_conf_params_w_var_values = []
        for idx in range(self.num_transformed_seq[seq_transform_id]):
            for key in self.transformed_seq_conf_template.keys():
                if isinstance(self.transformed_seq_conf_template[key], list):
                    self.transformed_seq_conf_params_w_var_values.append((key, idx))

        self.conf_params_w_var_values = [
            *self.main_config_params_w_var_values
        ]

        for value in self.transformed_seq_conf_params_w_var_values:
            self.conf_params_w_var_values.append(value)

    def generate_random_neighbor(self, 
        config
    ):
        new_config = copy.deepcopy(config)

        chosen_param = random.choice(self.conf_params_w_var_values)

        # print(chosen_param)

        try:
            while True:
                param_value = random.choice(self.transformed_seq_conf_template[chosen_param[0]])

                if chosen_param[0] in self.transformed_seq_parameter_value_as_list:
                    param_value = [param_value]

                if param_value != new_config["transformed_sequences"][chosen_param[1]][chosen_param[0]]:
                    new_config["transformed_sequences"][chosen_param[1]][chosen_param[0]] = param_value
                    break
             
        except:
            param_value = random.choice(self.variant_config_template[chosen_param])

            new_config[chosen_param] = param_value
        
        return new_config

    def generate_random_config(self):

        conf = copy.deepcopy(self.variant_config_template)
        
        for key in self.main_config_params_w_var_values:
            conf[key] = random.choice(self.variant_config_template[key])

        conf["transformed_sequences"] = []

        for _ in range(self.num_transformed_seq[self.seq_transform_id]):
            transformed_seq_conf = copy.deepcopy(self.transformed_seq_conf_template)

            for key in self.transformed_seq_conf_template.keys():
                if isinstance(GabacConfiguration.transformed_seq_conf_template[key], list):
                    if key in GabacConfiguration.transformed_seq_parameter_value_as_list:
                        transformed_seq_conf[key] = [
                            random.choice(
                                GabacConfiguration.transformed_seq_conf_template[key]
                            )
                        ]
                    else:
                        transformed_seq_conf[key] = random.choice(
                            GabacConfiguration.transformed_seq_conf_template[key]
                        )
            
            conf["transformed_sequences"].append(transformed_seq_conf)

        return conf

    # @staticmethod
    # def generate_random_config(
    #     seq_transform_id
    # ):
    #     conf_template = GabacConfiguration.available_variant[seq_transform_id]

    #     conf = copy.deepcopy(conf_template)
    #     conf["transformed_sequences"] = []

    #     for key in conf_template.keys():
    #         if isinstance(conf_template[key], list):
    #             conf[key] = random.choice(*conf_template[key])

    #     for _ in range(GabacConfiguration.num_transformed_seq[seq_transform_id]):
    #         transformed_seq_conf = copy.deepcopy(GabacConfiguration.transformed_seq_conf_template)

    #         for key in GabacConfiguration.transformed_seq_conf_template.keys():
    #             if isinstance(GabacConfiguration.transformed_seq_conf_template[key], list):
    #                 if key in GabacConfiguration.transformed_seq_parameter_value_as_list:
    #                     transformed_seq_conf[key] = [
    #                         random.choice(
    #                             GabacConfiguration.transformed_seq_conf_template[key]
    #                         )
    #                     ]
    #                 else:
    #                     transformed_seq_conf[key] = random.choice(
    #                         GabacConfiguration.transformed_seq_conf_template[key]
    #                     )
            
    #         conf["transformed_sequences"].append(transformed_seq_conf)

    #     return conf

    def json_to_cchar(
        self,
        config
    ):
        return array(ct.c_char, json.dumps(config, indent=4).encode('utf-8'))

    # @staticmethod
    # def json_to_cchar(
    #     config
    # ):
    #     return array(ct.c_char, json.dumps(config, indent=4).encode('utf-8'))

    # @staticmethod
    # def generate_random_cchar_config(
    #     seq_transform_id
    # ):
    #     return GabacConfiguration.json_to_cchar(
    #         GabacConfiguration.generate_random_config(
    #             seq_transform_id
    #         )
    #     )

    def run_gabac(
        self,
        data,
        config_cchar
    ):

        io_config = gabac_io_config()
        in_block = gabac_data_block()
        copy_in_block = gabac_data_block()

        logfilename = array(ct.c_char, "log.txt")

        start_time = time.time()

        if libgabac.gabac_data_block_init(
            in_block,
            data,
            len(data),
            ct.sizeof(ct.c_char)
        ):
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        if libgabac.gabac_data_block_copy(
            copy_in_block,
            in_block
        ):
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        original_length = in_block.values_size * in_block.word_size

        if libgabac.gabac_stream_create_buffer(
            io_config.input,
            in_block
        ):
            libgabac.gabac_data_block_release(in_block)
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        if libgabac.gabac_stream_create_buffer(
            io_config.output, 
            None
        ):
            libgabac.gabac_stream_release(io_config.input)
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        # Create log stream from file. You could also pass stdout instead.
        if libgabac.gabac_stream_create_file(
            io_config.log,
            logfilename,
            len(logfilename),
            1
        ):
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        # if libgabac.gabac_stream_create_buffer(
        #     io_config.log, 
        #     None
        # ):
        #     libgabac.gabac_stream_release(io_config.input)
        #     return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        # Encode using config
        if libgabac.gabac_run(
            GABAC_OPERATION.ENCODE,
            io_config,
            config_cchar,
            len(config_cchar),
        ):
            libgabac.gabac_data_block_release(in_block)
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            libgabac.gabac_stream_release(io_config.log)
            
            return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

        encoding_time = time.time() - start_time

        # Swap contents of output stream back into input stream to prepare decoding
        libgabac.gabac_stream_swap_block(io_config.output, in_block)
        encoded_length = in_block.values_size * in_block.word_size

        if self.ena_roundtrip:
            libgabac.gabac_stream_swap_block(io_config.input, in_block)

            if libgabac.gabac_run(
                GABAC_OPERATION.DECODE, 
                io_config,
                config_cchar,
                len(config_cchar),   
            ):
                libgabac.gabac_data_block_release(in_block)
                libgabac.gabac_stream_release(io_config.input)
                libgabac.gabac_stream_release(io_config.output)
                libgabac.gabac_stream_release(io_config.log)

                return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity

            libgabac.gabac_stream_swap_block(io_config.output, in_block)

            if libgabac.gabac_data_block_equals(in_block, copy_in_block):
                libgabac.gabac_data_block_release(in_block)
                libgabac.gabac_stream_release(io_config.input)
                libgabac.gabac_stream_release(io_config.output)
                libgabac.gabac_stream_release(io_config.log)
                return GABAC_RETURN.SUCCESS, encoded_length, encoding_time
            else:
                return GABAC_RETURN.FAILURE, np.Infinity, np.Infinity
        else:
            libgabac.gabac_data_block_release(in_block)
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            libgabac.gabac_stream_release(io_config.log)
            return GABAC_RETURN.SUCCESS, encoded_length, encoding_time

        # return encoded_length
        # return encoded_length - original_length
        #return encoded_length/original_length

