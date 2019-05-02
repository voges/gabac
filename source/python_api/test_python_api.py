#from new_gabac import gabac_execute_transform
import ctypes as ct
import json
import subprocess
import unittest
import copy

from new_gabac import libgabac
from new_gabac import GABAC_BINARIZATION, GABAC_CONTEXT_SELCT, GABAC_LOG_LEVEL, GABAC_LOG_LEVEL
from new_gabac import GABAC_OPERATION, GABAC_RETURN, GABAC_STREAM_MODE, GABAC_TRANSFORM
from new_gabac import gabac_data_block
from new_gabac import gabac_io_config


process = subprocess.Popen("git rev-parse --show-toplevel".split(), stdout=subprocess.PIPE)
output, error = process.communicate()
root_path = output.strip().decode("utf-8")

libc = ct.CDLL("libc.so.6")

def array(dtype, data):
    if isinstance(data, int):
        arr_dtype = data * dtype
        return arr_dtype()
    elif isinstance(data, (list, tuple, bytes)):
        arr_dtype = dtype * len(data)
        return arr_dtype(*data)
    elif isinstance(data, str):
        arr_dtype = dtype * len(data)
        return arr_dtype(*data.encode())
    elif isinstance(data, dict):
        raise TypeError("Not yet implemented for type dictionary")
    else:
        raise TypeError("Incorrect datatype of data")

def print_array(arr):
    for val in arr:
        if isinstance(val, bytes):
            print("{}".format(val.decode()), end='')
        else:
            print("{}".format(val), end='')
    print()

def print_block(block):
    for i in range(block.values_size):
        print("{:02d}".format(libgabac.gabac_data_block_get(ct.byref(block), i)), end='')
    print()

def get_block_values(block):
    values = ""
    for i in range(block.values_size):
        values += "{:02d}".format(libgabac.gabac_data_block_get(ct.byref(block), i))
    return values

class PythonApiTest(unittest.TestCase):
    input_data1 = array(
        ct.c_char, 
        b"\x01\x00\x00\x00" + \
        b"\x02\x00\x00\x00" + \
        b"\x03\x00\x00\x00" + \
        b"\x04\x00\x00\x00" + \
        b"\x05\x00\x00\x00" + \
        b"\x06\x00\x00\x00" + \
        b"\x07\x00\x00\x00" + \
        b"\x08\x00\x00\x00" + \
        b"\x09\x00\x00\x00" + \
        b"\x0a\x00\x00\x00")

    input_data2 = array(
        ct.c_char, 
        b"\x01\x00\x00\x00" + \
        b"\x01\x00\x00\x00" + \
        b"\x01\x00\x00\x00" + \
        b"\x03\x00\x00\x00" + \
        b"\x03\x00\x00\x00" + \
        b"\x03\x00\x00\x00" + \
        b"\x04\x00\x00\x00" + \
        b"\x04\x00\x00\x00" + \
        b"\x04\x00\x00\x00" + \
        b"\x05\x00\x00\x00"
    )

    config_json_py = {
        "word_size" : 1,
        "sequence_transformation_id" : 3,
        "sequence_transformation_parameter": [2],
        "transformed_sequences" : [
            {
                "lut_transformation_enabled" : True,
                "lut_transformation_parameter": [0],
                "diff_coding_enabled": False,
                "binarization_id" : 2,
                "binarization_parameters" : [ ],
                "context_selection_id" : 2
            },
            {
                "lut_transformation_enabled" : False,
                "lut_transformation_parameter": 0,
                "diff_coding_enabled": True,
                "binarization_id" : 3,
                "binarization_parameters" : [ ],
                "context_selection_id" : 2
            }
        ]
    }


    # config_json = array(
    #     ct.c_char,
    #     json.dumps(config_json_py)
    # )

    config_json_raw = json.dumps(config_json_py).encode(
        'utf-8'
    )
    config_json_ptr = ct.c_char_p(
        config_json_raw
    )


    config_json_cchar = array(
        ct.c_char,
        b"{\n" + \
        b"\"word_size\": 1,\n" + \
        b"\"sequence_transformation_id\": 0,\n" + \
        b"\"sequence_transformation_parameter\": 0,\n" + \
        b"\"transformed_sequences\":\n" + \
        b"[\n" + \
        b"{\n" + \
        b"\"lut_transformation_enabled\": false,\n" + \
        b"\"diff_coding_enabled\": false,\n" + \
        b"\"binarization_id\": 0,\n" + \
        b"\"binarization_parameters\":\n" + \
        b"[\n" + \
        b"8\n" + \
        b"],\n" + \
        b"\"context_selection_id\": 1\n" + \
        b"}\n" + \
        b"]\n" + \
        b"}"
    )

    def test_api(self):
        
        self.assertEqual(0, self._example_transformations(self.input_data1))
        self.assertEqual(0, self._example_transformations(self.input_data2))
        self.assertEqual(0, self._example_run())

    def _example_transformations(self, input_data):
        blocks = array(gabac_data_block, 2)
        parameters_RLE = array(ct.c_uint64, [255])
        parameters_CABAC = array(
            ct.c_uint64, 
            [
                GABAC_BINARIZATION.TU, 
                2, 
                GABAC_CONTEXT_SELCT.ADAPTIVE_ORDER_0, 
                ct.sizeof(ct.c_int)
            ]
        )

        # Allocate data block with 4 byte block size and the appriate length. 
        # The example data is copied 
        if libgabac.gabac_data_block_init(
            ct.byref(blocks[0]),
            input_data,
            ct.sizeof(input_data) // ct.sizeof(ct.c_int),
            ct.sizeof(ct.c_int)
        ):
            return -1

        if libgabac.gabac_data_block_init(
            ct.byref(blocks[1]), 
            None, 
            0, 
            ct.sizeof(ct.c_uint8)
        ):
            libgabac.gabac_data_block_release(blocks[0])
            return -1

        original_values = [get_block_values(block) for block in blocks]

        # Execute the actual RLE transformation. 
        # blocks[0] and blocks[1] will now contain the transformed streams
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.RLE,
            parameters_RLE,
            #GABAC_TRANSFORM.EQUALITY,
            #array(ct.c_uint64, []),
            GABAC_OPERATION.ENCODE,
            blocks
        ) == GABAC_RETURN.FAILURE:
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        # Diff-Coding on block 0
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.DIFF, 
            None, 
            GABAC_OPERATION.ENCODE,
            blocks
        ):
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        # Executing CABAC-Coding on block 1
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.CABAC,
            parameters_CABAC,
            GABAC_OPERATION.ENCODE, 
            ct.byref(blocks[1])
        ):
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        # Executing CABAC-Decoding on block 1
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.CABAC,
            parameters_CABAC,
            GABAC_OPERATION.DECODE,
            ct.byref(blocks[1])
        ):
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        # Diff-Decoding on block 0
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.DIFF,
            None,
            GABAC_OPERATION.DECODE,
            blocks
        ):
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        # After this last decoding step you should retrieve the raw example data again
        # RLE-Decoding
        if libgabac.gabac_execute_transform(
            GABAC_TRANSFORM.RLE,
            parameters_RLE,
            GABAC_OPERATION.DECODE,
            blocks
        ):
            libgabac.gabac_data_block_release(ct.byref(blocks[0]))
            libgabac.gabac_data_block_release(ct.byref(blocks[1]))
            return -1

        enc_dec_values = [get_block_values(block) for block in blocks]

        libgabac.gabac_data_block_release(ct.byref(blocks[0]))
        libgabac.gabac_data_block_release(ct.byref(blocks[1]))

        if enc_dec_values[0] == original_values[0]:
            return 0
        else:
            return -1

    def _example_run(self):
        # Init IO configuration
        io_config = gabac_io_config()
        io_config.log_level = GABAC_LOG_LEVEL.TRACE

        # Data block for input stream buffer
        in_block = gabac_data_block()

        logfilename = array(ct.c_char, "log.txt")

        # We will let gabac compress its own configuration.
        # Notice that offset -1 is to cut of the \0 at the end of the stream
        if libgabac.gabac_data_block_init(
            in_block,
            ### With cchar
            self.config_json_cchar,
            ct.sizeof(self.config_json_cchar) - 1,
            ct.sizeof(ct.c_char)

            # ### With raw
            # self.config_json_raw,
            # len(self.config_json_raw),
            # ct.sizeof(ct.c_char),
        ):
            return -1

        print_block(in_block)

        # Swap newly created input data block into a newly created input stream
        if libgabac.gabac_stream_create_buffer(
            io_config.input,
            in_block
        ):
            libgabac.gabac_data_block_release(in_block)
            return -1

        # Create empty output stream
        if libgabac.gabac_stream_create_buffer(
            io_config.output, 
            None
        ):
            libgabac.gabac_stream_release(io_config.input)
            return -1

        # Create log stream from file. You could also pass stdout instead.
        if libgabac.gabac_stream_create_file(
            io_config.log,
            logfilename, 
            len(logfilename), 
            1
        ):
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            return -1


        # Encode using config
        if libgabac.gabac_run(
            GABAC_OPERATION.ENCODE,
            io_config,
            # ct.byref(io_config),
            # ct.byref(self.config_json),
            # ct.sizeof(self.config_json) - 1
            # ct.byref(io_config),
            # self.config_json_ptr,
            # ct.sizeof(self.config_json_ptr) - 1

            ### With cchar
            self.config_json_cchar,
            ct.sizeof(self.config_json_cchar) - 1,
            
            # ### With ptr
            # self.config_json_raw,
            # len(self.config_json_raw)-1,
        ):
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            return -1

        # Swap contents of output stream back into input stream to prepare decoding
        if libgabac.gabac_stream_swap_block(
            io_config.output, 
            in_block
        ):
            libgabac.gabac_stream_release(io_config.input)
            libgabac.gabac_stream_release(io_config.output)
            return -1
        libgabac.gabac_stream_swap_block(ct.byref(io_config.input), ct.byref(in_block))

        # Decode
        print("*** Run gabac decode...")
        if libgabac.gabac_run(
            GABAC_OPERATION.DECODE, 
            ### With cchar
            io_config,
            self.config_json_raw,
            len(self.config_json_raw)-1,
        ):
            print("*** Gabac decode failed!")

        # Retrieve results, you should end up with your input data again
        libgabac.gabac_stream_swap_block(ct.byref(io_config.output), ct.byref(in_block))
        print_block(in_block)

        # Free all ressources
        print("*** Release...")
        libgabac.gabac_data_block_release(ct.byref(in_block))
        libgabac.gabac_stream_release(ct.byref(io_config.input))
        libgabac.gabac_stream_release(ct.byref(io_config.output))
        libgabac.gabac_stream_release(ct.byref(io_config.log))
        return 0

if __name__ == '__main__':
    unittest.main()