import os
import copy
import json
import random
import math
import ctypes as ct
from collections import OrderedDict

import numpy as np
import matplotlib.pyplot as plt

from gabac_api import libgabac
from gabac_api import gabac_stream
from gabac_api import gabac_io_config
from gabac_api import gabac_data_block
from gabac_api import GABAC_BINARIZATION, GABAC_CONTEXT_SELECT, GABAC_LOG_LEVEL, GABAC_LOG_LEVEL
from gabac_api import GABAC_OPERATION, GABAC_RETURN, GABAC_STREAM_MODE, GABAC_TRANSFORM
from gabac_api import root_path
from test_python_api import array, libc

from gabac_conf_gen import GabacConfiguration

class SimulatedAnnealingForGabac(object):
    r"""
    Source:
        https://en.wikipedia.org/wiki/Simulated_annealing

    Notations :
        T : temperature. Decreases to 0.
        s : a system state
        E(s) : Energy at s. The function we want to minimize
        ∆E : variation of E, from state s to state s_next
        P(∆E , T) : Probability to move from s to s_next. 
            if  ( ∆E < 0 ) P = 1
                else P = exp ( - ∆E / T) . Decreases as T →  0
    
    Pseudo-code:
        Let s = s0  -- initial state
        For k = 0 through kmax (exclusive):
            T ← temperature(k , kmax)
            Pick a random neighbour state , s_next ← neighbour(s)
            ∆E ← E(s) - E(s_next) 
            If P(∆E , T) ≥ random(0, 1), move to the new state:
                s ← s_next
        Output: the final state s
    """

    def __init__(
        self,
        data,
        seq_transform_id,
        kmax=50,
        kt=1,
    ):
        # Gabac-specific parameter
        self.data = data
        self.seq_transform_id = seq_transform_id

        # Hyperparameters
        self.kmax = kmax
        self.kt = kt

        # Init
        self.gc = GabacConfiguration(self.seq_transform_id)
        while True:
            first_config = self.gc.generate_random_config()
            return_val, enc_length, enc_time = self.gc.run_gabac(self.data, self.gc.json_to_cchar(first_config))
            if return_val == GABAC_RETURN.SUCCESS:
                break

        self.s0 = first_config
        self.E0 = enc_length / len(data)
        
        print('E{:03d}: {:.3f}'.format(0, self.E0))

    def _temperature(
        self,
        k,
    ):
        return (1 - k/self.kmax) * self.kt

    def start(self):
        s = self.s0
        E = self.E0

        best_s = s
        best_E = E

        result = np.zeros((self.kmax, 3))

        for k in range(self.kmax):
            T = self._temperature(k)

            while True:
                new_s = self.gc.generate_random_neighbor(s)

                with open('curr_config.json', 'w') as f:
                    json.dump(new_s, f, indent=4)

                return_val, enc_length, enc_time = self.gc.run_gabac(self.data, self.gc.json_to_cchar(new_s))
                if return_val == GABAC_RETURN.SUCCESS:
                    break

            new_E = enc_length/len(self.data)

            dE = new_E - E

            result[k, 0] = new_E

            if dE > 0.0:
                print('E{:3d}: {:.3f} dE {:.02e} '.format(
                    k, new_E, dE
                ), end='')

                # if math.exp(-dE / T) < random.random():
                if np.exp(dE/T) - 1 < random.random():
                    print('accept', end='')
                    s = new_s
                    E = new_E

                else:
                    pass

                print()

            else:
                print('E{:3d}: {:.3f} dE {:.02e} (<) '.format(
                    k, new_E, dE
                ))
                s = new_s
                E = new_E

            result[k, 2] = E

            if E < best_E:
                best_s = s
                best_E = E
                
            result[k, 1] = best_E

        self.result = result
        return best_s, best_E

    def show_plot(self):
        plt.plot(np.arange(self.kmax), self.result[:,0], 'b')
        plt.plot(np.arange(self.kmax), self.result[:,2], 'r')
        plt.plot(np.arange(self.kmax), self.result[:,1], 'k')
        plt.show()
    

