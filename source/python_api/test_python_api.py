#from new_gabac import gabac_execute_transform
import ctypes as ct
import json

from new_gabac import libgabac
from new_gabac import GABAC_BINARIZATION, GABAC_CONTEXT_SELCT, GABAC_LOG_LEVEL, GABAC_LOG_LEVEL
from new_gabac import GABAC_OPERATION, GABAC_RETURN, GABAC_STREAM_MODE, GABAC_TRANSFORM
from new_gabac import gabac_data_block
from new_gabac import gabac_io_config

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
        print("{:d}".format(libgabac.gabac_data_block_get(ct.byref(block), i)), end='')
    print()

### Constants
FALSE = 0
TRUE = 1

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
    b"\x05\x00\x00\x00")

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

# config_json_py = {
#     "word_size" : 1,
#     "sequence_transformation_id" : 0,
#     "sequence_transformation_parameter" : 0,
#     "transfomred_sequences" : [
#         {
#             "lut_transformation_enabled" : False,
#             "diff_coding_enabled" : False,
#             "binarization_id" : 0,
#             "binarization_parameters" : [
#                 8
#             ],
#             "context_selection_id" : 1
#         }
#     ]
# }


# config_json = array(
#     ct.c_char,
#     json.dumps(config_json_py)
# )

config_json = array(
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

print_array(config_json)

def example_transformations():
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
    print("--> Test Transformations")
    print("***Init blocks...")

    # Allocate data block with 4 byte block size and the appriate length. 
    # The example data is copied 
    if libgabac.gabac_data_block_init(
        blocks[0],
        input_data2,
        ct.sizeof(input_data2) // ct.sizeof(ct.c_int),
        ct.sizeof(ct.c_int)
    ):
        print("Block 0 init failed!")
        return -1

    if libgabac.gabac_data_block_init(
        blocks[1], 
        None, 
        0, 
        ct.sizeof(ct.c_uint8)
    ):
        libgabac.gabac_data_block_release(blocks[0])
        print("Block 1 init failed!\n")
        return -1

    print("Block 0:")
    print_block(blocks[0])
    print("Block 1:")
    print_block(blocks[1])

    # Execute the actual RLE transformation. 
    # blocks[0] and blocks[1] will now contain the transformed streams
    print("INFO:Executing RLE-Coding!")

    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.RLE,
        parameters_RLE,
        FALSE,
        blocks
    ) == GABAC_RETURN.FAILURE:
        print("RLE transform failed!")

    print_block(blocks[0])
    print_block(blocks[1])

    print("***Executing Diff-Coding on block 0!")
    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.DIFF, 
        None, 
        0,
        blocks
    ):
        raise ValueError("Diff coding failed!")

    print_block(blocks[0])
    print()

    print("***Executing CABAC-Coding on block 1!")
    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.CABAC,
        parameters_CABAC,
        FALSE, 
        ct.byref(blocks[1])
    ):
        print("Cabac coding failed!")

    print_block(blocks[1])
    print()

    print("***Executing CABAC-Decoding on block 1!")
    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.CABAC,
        parameters_CABAC,
        TRUE,
        ct.byref(blocks[1])
    ):
        print("Cabac decoding failed!")

    print_block(blocks[1])
    print()

    print("***Executing Diff-Decoding on block 0!")
    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.DIFF,
        None,
        TRUE,
        blocks
    ):
        print("Inverse diff transform failed!")
    print_block(blocks[0])
    print()

    # After this last decoding step you should retrieve the raw example data again
    print("***Executing RLE-Decoding!")
    if libgabac.gabac_execute_transform(
        GABAC_TRANSFORM.RLE,
        parameters_RLE,
        TRUE,
        blocks
    ):
        print("Inverse diff coding failed!")

    print_block(blocks[0])
    print_block(blocks[1])
    print()

    libgabac.gabac_data_block_release(ct.byref(blocks[0]))
    libgabac.gabac_data_block_release(ct.byref(blocks[1]))

    return 0

def example_run():
    # Init IO configuration
    io_config = gabac_io_config()

    # Data block for input stream buffer
    in_block = gabac_data_block()

    logfilename = array(ct.c_char, "log.txt")
    #print_array(logfilename)

    print("--> Test full run\n")
    print("*** Init streams...\n")

    # We will let gabac compress its own configuration.
    # Notice that the -1 is to cut of the \0 at the end of the stream
    if libgabac.gabac_data_block_init(
        ct.byref(in_block),
        config_json,
        ct.sizeof(config_json) - 1,
        ct.sizeof(ct.c_uint8)
    ):
        print("*** Could not allocate buffer!")
        return -1

    print_block(in_block)

    # Swap newly created input data block into a newly created input stream
    if libgabac.gabac_stream_create_buffer(
        ct.byref(io_config.input),
        ct.byref(in_block)
    ):
        print("*** Could not allocate in stream!")
        libgabac.gabac_data_block_release(ct.byref(in_block))
        return -1

    # Create empty output stream
    if libgabac.gabac_stream_create_buffer(
        ct.byref(io_config.output), 
        None
    ):
        print("*** Could not allocate out stream!")
        libgabac.gabac_stream_release(ct.byref(io_config.input))
        return -1

    # Create log stream from file. You could also pass stdout instead.
    if libgabac.gabac_stream_create_file(
        ct.byref(io_config.log),
        logfilename, 
        len(logfilename), 
        TRUE
    ):
        print("*** Could not allocate log stream!")
        libgabac.gabac_stream_release(ct.byref(io_config.input))
        libgabac.gabac_stream_release(ct.byref(io_config.output))
        return -1


    # Encode using config!
    print("*** Run gabac encode...")
    if libgabac.gabac_run(
        GABAC_OPERATION.ENCODE,
        ct.byref(io_config),
        config_json,
        ct.sizeof(config_json) - 1
    ):
        raise Exception("*** Gabac encode failed!")

    # Swap contents of output stream back into input stream to prepare decoding
    libgabac.gabac_stream_swap_block(ct.byref(io_config.output), ct.byref(in_block))
    print_block(in_block)
    libgabac.gabac_stream_swap_block(ct.byref(io_config.input), ct.byref(in_block))

    # Decode
    print("*** Run gabac decode...")
    if libgabac.gabac_run(
        GABAC_OPERATION.DECODE, 
        ct.byref(io_config),
        config_json, 
        ct.sizeof(config_json) - 1
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

example_run()
example_transformations()