import numpy as np
import itertools
import pprint
import random
from collections import OrderedDict

from new_gabac import GABAC_BINARIZATION, GABAC_CONTEXT_SELCT, GABAC_LOG_LEVEL, GABAC_LOG_LEVEL
from new_gabac import GABAC_OPERATION, GABAC_RETURN, GABAC_STREAM_MODE, GABAC_TRANSFORM

equ_params_values = [
    [0]
]

match_params_values = [
    np.power(2, np.arange(8)),
]

rle_params_values = [
    np.arange(256)
]

seq_transform_config_info = {
    GABAC_TRANSFORM.EQUALITY : {
        'num_transformed_seq' : 2,
        'possible_params_values' : equ_params_values
    },
    GABAC_TRANSFORM.MATCH    : {
        'num_transformed_seq' : 3,
        'possible_params_values' : match_params_values
    },
    GABAC_TRANSFORM.RLE      : {
        'num_transformed_seq' : 2,
        'possible_params_values' : rle_params_values
    }
}

lut_params_values = [
    [False, True]
]

diff_params_values = [
    [False, True]
]

cabac_params_values =[
    # GABAC_BINARIZATION
    np.arange(6),
    # Binariztion parameter -> Unknown parameter
    np.power(2, np.arange(5)),
    # GABAC_CONTEXT_SELCT
    np.arange(4),
]

class CabacConfiguration():

    @staticmethod
    def randomize_complete_config(
        seq_transform_id
    ):
        seq_transform_config = seq_transform_config_info[seq_transform_id]

        complete_config = []

        seq_transformation_config = [seq_transform_id]
        seq_transformation_config.append(
            CabacConfiguration.randomize_transformed_seq_params(
                seq_transform_config['possible_params_values']
            )
        )

        complete_config.append(seq_transformation_config)

        transformed_seq_configs = []
        for _ in range(seq_transform_config['num_transformed_seq']):
            transformed_seq_config = []
            for params_values in [lut_params_values, diff_params_values, cabac_params_values]:
                transformed_seq_config.append(
                    CabacConfiguration.randomize_transformed_seq_params(params_values)
                )
            transformed_seq_configs.append(transformed_seq_config)
        
        complete_config.append(transformed_seq_configs)

        return complete_config

    @staticmethod
    def config_to_dict(config):

        config_as_dict = OrderedDict(
            word_size = 1,
            sequence_transformation_id = config[0][0],
            sequence_transformation_parameter = config[0][1][0],
        )

        transformed_sequences_value = []
        for section_config in config[1]:
            transformed_sequences_value.append(OrderedDict(
                lut_transformation_enabled    = section_config[0][0],
                lut_transformation_parameter  = 0,
                diff_coding_enabled           = section_config[1][0],
                binarization_id               = section_config[2][0],
                binarization_parameters       = section_config[2][1],
                context_selection_id          = section_config[2][2]
            ))

        config_as_dict["transformed_sequences"] = transformed_sequences_value

        return config_as_dict


    @staticmethod
    def generate_config_list(
        seq_transform_id
    ):
        seq_transform_config = seq_transform_config_info[seq_transform_id]

        config_list = [seq_transform_id]
        config_list.extend(
            CabacConfiguration.randomize_transformed_seq_params(
                seq_transform_config['possible_params_values']
            )
        )

        for _ in range(seq_transform_config['num_transformed_seq']):
            for params_values in [lut_params_values, diff_params_values, cabac_params_values]:
                config_list.extend(
                    CabacConfiguration.randomize_transformed_seq_params(params_values)
                )

        return config_list

    @staticmethod
    def randomize_transformed_seq_params(
        params_values
    ):

        parameters = []
        for values in params_values:
            parameters.append(values[np.random.randint(len(values))])
        return parameters

    @staticmethod
    def config_list_to_config_dict(
        config_list
    ):

        config_as_dict = OrderedDict(
            word_size = 1,
            sequence_transformation_id = config_list[0],
            sequence_transformation_parameter = config_list[1],
        )

        transformed_sequences_value = []
        for idx in range(2, len(config_list), 5):
            transformed_sequences_value.append(OrderedDict(
                lut_transformation_enabled    = config_list[idx+0],
                lut_transformation_parameter  = 0,
                diff_coding_enabled           = config_list[idx+1],
                binarization_id               = config_list[idx+2],
                binarization_parameters       = config_list[idx+3],
                context_selection_id          = config_list[idx+4]
            ))

        config_as_dict["transformed_sequences"] = transformed_sequences_value

        return config_as_dict


class GeneticAlgorithm(object):

    def __init__(
        self,
        seq_transform,
    ):
        self.seq_transform = seq_transform

    def init_population(
        self,
        num_population
    ):
        # self.population = []
        # for _ in range(num_population):
        #     self.population.append(
        #         CabacConfiguration.generate_config_list(
        #             self.seq_transform
        #         )
        #     )

        self.num_population = num_population

    def crossover(
        self,
        parent01, 
        parent02,
        prob,
        mode='slice'
    ):

        if mode == 'slice':
            # left_slice = slice(
            #     1, 
            #     1 + round(
            #         prob * (len(parent01)-1)
            #     )
            # )
            # right_slice  = slice(
            #     1 + round(
            #         prob * (len(parent01)-1)
            #     ),
            #     len(parent01)
            # )

            # child01 = [parent01[0]]
            # child01.extend(parent01[left_slice])
            # child01.extend(parent02[right_slice])
            # child02 = [parent01[0]]
            # child02.extend(parent01[left_slice])
            # child02.extend(parent02[right_slice])

            rand_idx = random.choice(len(parent01))

            child01 = parent01[rand_idx:] + parent02[:rand_idx]
            child02 = parent02[rand_idx:] + parent01[:rand_idx]

            return child01, child02

        if mode == 'random':
            pass

    def set_mutation_rate(
        self,
        iteration
    ):
        pass

    def selection(
        self
    ):
        pass

    def mutate(
        self,
    ):

        return None
    
    def obj_func(
        self
    ):

        # Score
        return 0

    def run(self, max_iter=999):


        parent = CabacConfiguration.generate_config_list(
            self.seq_transform
        )

        for iteration in range(max_iter):
            self.set_mutation_rate(iteration)

            population = [self.mutate(parent) for _ in range(self.num_population)]
            parent = min(population, key=self.obj_func)








    


ga = GeneticAlgorithm(GABAC_TRANSFORM.RLE)

ga.init_population(10)

config_dict = CabacConfiguration.config_list_to_config_dict(ga.population[0])

ga.crossover(ga.population[0], ga.population[1], 0.6)

# pp = pprint.PrettyPrinter(indent=4)
# pp.pprint(config_dict)

pass
            

        


