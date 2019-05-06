import os
import copy
import json
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
from test_python_api import array, libc

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
            GABAC_BINARIZATION.BI, 
            GABAC_BINARIZATION.TU,
            GABAC_BINARIZATION.SEG,
            GABAC_BINARIZATION.EG,
            GABAC_BINARIZATION.STEG,
            GABAC_BINARIZATION.TEG
        ],
        # binarization_parameters = [ v.item() for v in np.power(2, np.arange(5)) ],
        binarization_parameters = [ v.item() for v in np.power(2, np.arange(4)) ],
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
        "word_size" : [
            # [ v.item() for v in np.power(2, np.arange(6)) ]
            [ v.item() for v in np.power(2, np.arange(4)) ]
        ],
        "sequence_transformation_id" : None,
        "sequence_transformation_parameter" : None,
        "transformed_sequences" : None
    }

    equality_config_template = copy.deepcopy(main_conf_template)
    equality_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.EQUALITY
    equality_config_template["sequence_transformation_parameter"] = 0

    match_config_template = copy.deepcopy(main_conf_template)
    match_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.MATCH
    # match_config_template["sequence_transformation_parameter"] = [ v.item() for v in np.power(2, np.arange(8)) ]
    match_config_template["sequence_transformation_parameter"] = [ v.item() for v in np.power(2, np.arange(4)) ]
    
    rle_config_template = copy.deepcopy(main_conf_template)
    rle_config_template["sequence_transformation_id"] = GABAC_TRANSFORM.RLE 
    rle_config_template["sequence_transformation_parameter"] = 255

    available_variant = {
        GABAC_TRANSFORM.EQUALITY : equality_config_template,
        GABAC_TRANSFORM.MATCH : match_config_template,
        GABAC_TRANSFORM.RLE : rle_config_template,
    }

    num_transformed_seq = {
        GABAC_TRANSFORM.EQUALITY : 2,
        GABAC_TRANSFORM.MATCH : 3,
        GABAC_TRANSFORM.RLE : 2,
    }

    @staticmethod
    def generate_random_config(
        seq_transform_id
    ):
        conf_template = GabacConfiguration.available_variant[seq_transform_id]

        conf = copy.deepcopy(conf_template)
        conf["transformed_sequences"] = []

        for key in conf_template.keys():
            if isinstance(conf_template[key], list):
                conf[key] = random.choice(*conf_template[key])

        for _ in range(GabacConfiguration.num_transformed_seq[seq_transform_id]):
            transformed_seq_conf = copy.deepcopy(GabacConfiguration.transformed_seq_conf_template)

            for key in GabacConfiguration.transformed_seq_conf_template.keys():
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

    @staticmethod
    def json_to_char(
        config
    ):
        return array(ct.c_char, json.dumps(config, indent=4).encode('utf-8'))


    @staticmethod
    def randomize_param(
        config,
        prob
    ):
        pass

class GeneticAlgorithmForGabac(object):

    def __init__(
        self,
        seq_transformation_id,
        mutation_prob=0.05,
        num_populations=100,
        num_generations=100,
        data=None
    ):
        pass
        self.gc = GabacConfiguration()

        self.seq_transform_id = seq_transformation_id
        self.mutation_prob = mutation_prob
        self.num_populations = num_populations
        self.num_generations = num_generations
        self.data = data

    def _init_population(self):
        self.populations = []
        for _ in range(self.num_populations):
            self.populations.append(
                self.gc.json_to_char(
                    self.gc.generate_random_config(self.seq_transform_id)
                )
            )

        self.fitness = np.zeros((self.num_populations))

    def _run_gabac(self, config_cchar):
        io_config = gabac_io_config()
        in_block = gabac_data_block()

        logfilename = array(ct.c_char, "log.txt")

        if libgabac.gabac_data_block_init(
            in_block,
            copy.deepcopy(self.data),
            len(self.data),
            ct.sizeof(ct.c_char)
        ):
            raise OSError('Cannot initialize data block')
            # return GABAC_RETURN.FAILURE, -1

        original_length = in_block.values_size * in_block.word_size

        if libgabac.gabac_stream_create_buffer(
            io_config.input,
            in_block
        ):
            libgabac.gabac_data_block_release(in_block)
            # return original_length
            return 1

        if libgabac.gabac_stream_create_buffer(
            io_config.output, 
            None
        ):
            libgabac.gabac_stream_release(io_config.input)
            # return original_length
            return 1

        # Create log stream from file. You could also pass stdout instead.
        if libgabac.gabac_stream_create_file(
            io_config.log,
            logfilename,
            len(logfilename),
            1
        ):
            libc.printf(b"*** Could not allocate log stream!\n")
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            # return original_length
            return 1

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
            # return original_length
            return 1

        # Swap contents of output stream back into input stream to prepare decoding
        libgabac.gabac_stream_swap_block(io_config.output, in_block)
        
        encoded_length = in_block.values_size * in_block.word_size

        libgabac.gabac_data_block_release(in_block)
        libgabac.gabac_stream_release(io_config.input)
        libgabac.gabac_stream_release(io_config.output)
        libgabac.gabac_stream_release(io_config.log)

        # return encoded_length
        # return encoded_length - original_length
        return encoded_length/original_length

    def _evaluate_fitness(self, curr_gen):
        
        for idx, config_cchar in enumerate(self.populations):
            try:
                self.fitness[idx] = self._run_gabac(config_cchar)
            except:
                pass

            print("Gen {:03d} idx {:03d} size {:.3f}".format(curr_gen, idx, self.fitness[idx]))

    def start(self):
        self._init_population()

        for curr_gen in range(self.num_generations):
            self._evaluate_fitness(curr_gen)
            pass


# x = GabacConfiguration.generate_random_config(GABAC_TRANSFORM.RLE)
# with open('test_config.json', 'w') as f:
#     json.dump(x, f, indent=4)
# print(json.dumps(x, indent=4))

with open(os.path.join(root_path, 'resources', 'input_files', 'one_mebibyte_random'), 'rb') as f:
    data = f.read()

ga = GeneticAlgorithmForGabac(GABAC_TRANSFORM.RLE, data=data)
ga.start()
pass
            
        


