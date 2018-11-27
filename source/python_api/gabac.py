"""GABAC library."""
import ctypes as ct
import numpy as np
import os
import sys

libgabac_path = os.environ['LIBGABAC_PATH']

libgabac = ct.cdll.LoadLibrary(libgabac_path)


class ReturnCode:
    """Return Codes."""

    GABAC_SUCCESS = 0
    GABAC_FAILURE = 1


class SequenceTransformationId:
    """Parameters."""

    no_transform = 0
    equality_coding = 1
    match_coding = 2
    rle_coding = 3


class BinarizationId:
    """Parameters."""

    BI = 0
    TU = 1
    EG = 2
    SEG = 3
    TEG = 4
    STEG = 5


class ContextSelectionId:
    """Parameters."""

    bypass = 0
    adaptive_coding_order_0 = 1
    adaptive_coding_order_1 = 2
    adaptive_coding_order_2 = 3


# encode
# -----------------------------------------------------------------------------

libgabac.gabac_encode.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_uint,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_uint,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_encode.restype = ct.c_int


def encode(symbols,
           binarization_id,
           binarization_parameters,
           context_selection_id):
    """
    Encode symbols.

    :param symbols:
    :param binarization_id:
    :param binarization_parameters:
    :param context_selection_id:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (encoded_symbols).
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(binarization_parameters) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if binarization_parameters.dtype != np.intc:
        raise TypeError("Wrong data input type.")
    bitstream = ct.pointer(ct.c_ubyte())
    bitstream_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_encode(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        binarization_id,
        binarization_parameters.ctypes.data_as(ct.POINTER(ct.c_uint)),
        len(binarization_parameters),
        context_selection_id,
        ct.pointer(bitstream),
        ct.pointer(bitstream_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_encode() failed")

    bitstream_ptr = ct.cast(
        bitstream,
        ct.POINTER(ct.c_ubyte * bitstream_size.value)
    )[0]

    bitstream = np.ndarray(
        (len(bitstream_ptr),),
        'ubyte',
        bitstream_ptr,
        order='C'
    )

    return bitstream


# decode
# -----------------------------------------------------------------------------

libgabac.gabac_decode.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_uint,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_uint,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_decode.restype = ct.c_int


def decode(bitstream,
           binarization_id,
           binarization_parameters,
           context_selection_id):
    """
    Decode bitstream.

    :param bitstream:
    :param binarization_id:
    :param binarization_parameters:
    :param context_selection_id:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (decoded_symbols).
    """
    if type(bitstream) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if bitstream.dtype != np.uint8:
        raise TypeError("Wrong data input type.")
    if type(binarization_parameters) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if binarization_parameters.dtype != np.intc:
        raise TypeError("Wrong data input type.")

    symbols = ct.pointer(ct.c_uint64())
    symbols_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_decode(
        bitstream.ctypes.data_as(ct.POINTER(ct.c_ubyte)),
        len(bitstream),
        binarization_id,
        binarization_parameters.ctypes.data_as(ct.POINTER(ct.c_uint)),
        len(binarization_parameters),
        context_selection_id,
        ct.pointer(symbols),
        ct.pointer(symbols_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_encode() failed")

    symbols_ptr = ct.cast(
        symbols,
        ct.POINTER(ct.c_uint64 * symbols_size.value)
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return symbols


# transformDiffCoding
# -----------------------------------------------------------------------------

libgabac.gabac_transformDiffCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p]
)

libgabac.gabac_transformDiffCoding.restype = ct.c_int


def transform_diff_coding(symbols):
    """
    Transform symbols using diff coding.

    :param symbols:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (transformed_symbols).
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    transformed_symbols_ptr = ct.pointer(ct.c_uint64())

    gabac_rc = libgabac.gabac_transformDiffCoding(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        ct.pointer(transformed_symbols_ptr)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformDiffCoding() failed")

    transformed_symbols_ptr = ct.cast(
        transformed_symbols_ptr,
        ct.POINTER(ct.c_int64 * len(symbols))
    )[0]

    transformed_symbols = np.ndarray(
        (len(transformed_symbols_ptr),),
        'int64',
        transformed_symbols_ptr,
        order='C'
    )

    return transformed_symbols


# inverseTransformDiffCoding
# -----------------------------------------------------------------------------

libgabac.gabac_inverseTransformDiffCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p]
)

libgabac.gabac_inverseTransformDiffCoding.restype = ct.c_int


def inverse_transform_diff_coding(transformed_symbols):
    """
    Recover symbols from diff coding.

    :param transformed_symbols:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (decoded_symbols).
    """
    if type(transformed_symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if transformed_symbols.dtype != np.int64:
        raise TypeError("Wrong data input type.")
    symbols_ptr = ct.pointer(ct.c_int64())

    gabac_rc = libgabac.gabac_inverseTransformDiffCoding(
        transformed_symbols.ctypes.data_as(ct.POINTER(ct.c_int64)),
        len(transformed_symbols),
        ct.pointer(symbols_ptr)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_inverseTransformDiffCoding() failed")

    symbols_ptr = ct.cast(
        symbols_ptr,
        ct.POINTER(ct.c_uint64 * len(transformed_symbols))
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return symbols


# transformEqualityCoding
# -----------------------------------------------------------------------------

libgabac.gabac_transformEqualityCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_transformEqualityCoding.restype = ct.c_int


def transform_equality_coding(symbols):
    """
    Transform symbols using equality coding.

    :param symbols:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 2 stream (equality_flags, raw_values).
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    equality_flags_ptr = ct.pointer(ct.c_uint64())
    values_ptr = ct.pointer(ct.c_uint64())
    values_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_transformEqualityCoding(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        ct.pointer(equality_flags_ptr),
        ct.pointer(values_ptr),
        ct.pointer(values_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformEqualityCoding() failed")

    equality_flags_ptr = ct.cast(
        equality_flags_ptr,
        ct.POINTER(ct.c_uint64 * len(symbols))
    )[0]
    values_ptr = ct.cast(
        values_ptr,
        ct.POINTER(ct.c_uint64 * values_size.value)
    )[0]

    equality_flags = np.ndarray(
        (len(equality_flags_ptr),),
        'uint64',
        equality_flags_ptr,
        order='C'
    )
    values = np.ndarray(
        (len(values_ptr),),
        'uint64',
        values_ptr,
        order='C'
    )

    return equality_flags, values


# inverseTransformEqualityCoding
# -----------------------------------------------------------------------------

libgabac.gabac_inverseTransformEqualityCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p]
)

libgabac.gabac_inverseTransformEqualityCoding.restype = ct.c_int


def inverse_transform_equality_coding(equality_flags,
                                      values):
    """
    Recover symbols using equality coding.

    :param equality_flags:
    :param values:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (decoded_symbols).
    """
    if type(equality_flags) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if equality_flags.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(values) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if values.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    symbols_ptr = ct.pointer(ct.c_uint64())

    gabac_rc = libgabac.gabac_inverseTransformEqualityCoding(
        equality_flags.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(equality_flags),
        values.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(values),
        ct.pointer(symbols_ptr)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_inverseTransformEqualityCoding() failed")

    symbols_ptr = ct.cast(
        symbols_ptr,
        ct.POINTER(ct.c_uint64 * len(equality_flags))
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return symbols


# transformLutTransform0
# -----------------------------------------------------------------------------

libgabac.gabac_transformLutTransform0.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_transformLutTransform0.restype = ct.c_int


def transform_lut_transform0(symbols):
    """
    Transform symbols as LUT transform order 0.

    :param symbols: Symbols to be transformed.
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 2 Streams: (transformed symbols, inverse LUT)
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    transformed_ptr = ct.pointer(ct.c_uint64())
    invlut_ptr = ct.pointer(ct.c_uint64())
    invlut_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_transformLutTransform0(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        ct.pointer(transformed_ptr),
        ct.pointer(invlut_ptr),
        ct.pointer(invlut_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformMatchCoding() failed")

    transformed_ptr = ct.cast(
        transformed_ptr,
        ct.POINTER(ct.c_uint64 * len(symbols))
    )[0]
    invlut_ptr = ct.cast(
        invlut_ptr,
        ct.POINTER(ct.c_uint64 * invlut_size.value)
    )[0]

    transformed = np.ndarray(
        (len(transformed_ptr),),
        'uint64',
        transformed_ptr,
        order='C'
    )
    invlut = np.ndarray(
        (len(invlut_ptr),),
        'uint64',
        invlut_ptr,
        order='C'
    )

    return (
        transformed,
        invlut
    )


# inverseTransformLutTransform0
# -----------------------------------------------------------------------------

libgabac.gabac_inverseTransformLutTransform0.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p]
)

libgabac.gabac_inverseTransformLutTransform0.restype = ct.c_int


def inverse_transform_lut_transform0(transsymbols, invlut):
    """
    Perform inverse LUT transform order 0.

    :param transsymbols: Transformed symbols.
    :param invlut: inverse lut order 0
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream: (Untransformed symbols)
    """
    if type(transsymbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if transsymbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(invlut) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if invlut.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    symbols_ptr = ct.pointer(ct.c_uint64())

    gabac_rc = libgabac.gabac_inverseTransformLutTransform0(
        transsymbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(transsymbols),
        invlut.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(invlut),
        ct.pointer(symbols_ptr)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformMatchCoding() failed")

    symbols_ptr = ct.cast(
        symbols_ptr,
        ct.POINTER(ct.c_uint64 * len(transsymbols))
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return (
        symbols
    )


# transformMatchCoding
# -----------------------------------------------------------------------------

libgabac.gabac_transformMatchCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_transformMatchCoding.restype = ct.c_int


def transform_match_coding(symbols,
                           windowsize):
    """
    Transform symbols using match coding.

    :param symbols:
    :param windowsize:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 3 streams (pointers, lengths, raw_values).
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    pointers_ptr = ct.pointer(ct.c_uint64())
    pointers_size = ct.c_size_t()
    lengths_ptr = ct.pointer(ct.c_uint64())
    lengths_size = ct.c_size_t()
    raw_values_ptr = ct.pointer(ct.c_uint64())
    raw_values_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_transformMatchCoding(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        windowsize,
        ct.pointer(pointers_ptr),
        ct.pointer(pointers_size),
        ct.pointer(lengths_ptr),
        ct.pointer(lengths_size),
        ct.pointer(raw_values_ptr),
        ct.pointer(raw_values_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformMatchCoding() failed")

    pointers_ptr = ct.cast(
        pointers_ptr,
        ct.POINTER(ct.c_uint64 * pointers_size.value)
    )[0]
    lengths_ptr = ct.cast(
        lengths_ptr,
        ct.POINTER(ct.c_uint64 * lengths_size.value)
    )[0]
    raw_values_ptr = ct.cast(
        raw_values_ptr,
        ct.POINTER(ct.c_uint64 * raw_values_size.value)
    )[0]

    pointers = np.ndarray(
        (len(pointers_ptr),),
        'uint64',
        pointers_ptr,
        order='C'
    )
    lengths = np.ndarray(
        (len(lengths_ptr),),
        'uint64',
        lengths_ptr,
        order='C'
    )
    raw_values = np.ndarray(
        (len(raw_values_ptr),),
        'uint64',
        raw_values_ptr,
        order='C'
    )

    return (
        pointers,
        lengths,
        raw_values
    )


# inverseTransformMatchCoding
# -----------------------------------------------------------------------------

libgabac.gabac_inverseTransformMatchCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_inverseTransformMatchCoding.restype = ct.c_int


def inverse_transform_match_coding(pointers,
                                   lengths,
                                   raw_values):
    """
    Recover symbols from match coding.

    :param pointers:
    :param lengths:
    :param raw_values:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (decoded_symbols).
    """
    if type(pointers) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if pointers.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(lengths) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if lengths.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(raw_values) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if raw_values.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    symbols_ptr = ct.pointer(ct.c_uint64())
    symbols_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_inverseTransformMatchCoding(
        pointers.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(pointers),
        lengths.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(lengths),
        raw_values.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(raw_values),
        ct.pointer(symbols_ptr),
        ct.pointer(symbols_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_inverseTransformMatchCoding() failed")

    symbols_ptr = ct.cast(
        symbols_ptr,
        ct.POINTER(ct.c_uint64 * symbols_size.value)
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return symbols


# transformRleCoding
# -----------------------------------------------------------------------------

libgabac.gabac_transformRleCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_transformRleCoding.restype = ct.c_int


def transform_rle_coding(symbols, guard):
    """
    Transform symbols using run length coding.

    :param symbols:
    :param guard:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 2 streams (raw_values, run_lengths).
    """
    if type(symbols) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if symbols.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    raw_values_ptr = ct.pointer(ct.c_uint64())
    raw_values_size = ct.c_size_t()
    lengths_ptr = ct.pointer(ct.c_uint64())
    lengths_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_transformRleCoding(
        symbols.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(symbols),
        ct.c_size_t(guard),
        ct.pointer(raw_values_ptr),
        ct.pointer(raw_values_size),
        ct.pointer(lengths_ptr),
        ct.pointer(lengths_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_transformRleCoding() failed")

    raw_values_ptr = ct.cast(
        raw_values_ptr,
        ct.POINTER(ct.c_uint64 * raw_values_size.value)
    )[0]
    lengths_ptr = ct.cast(
        lengths_ptr,
        ct.POINTER(ct.c_uint64 * lengths_size.value)
    )[0]

    raw_values = np.ndarray(
        (len(raw_values_ptr),),
        'uint64',
        raw_values_ptr,
        order='C'
    )
    lengths = np.ndarray(
        (len(lengths_ptr),),
        'uint64',
        lengths_ptr,
        order='C'
    )

    return raw_values, lengths


# inverseTransformRleCoding
# -----------------------------------------------------------------------------

libgabac.gabac_inverseTransformRleCoding.argtypes = (
    [ct.c_void_p,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_size_t,
     ct.c_size_t,
     ct.c_void_p,
     ct.c_void_p]
)

libgabac.gabac_inverseTransformRleCoding.restype = ct.c_int


def inverse_transform_rle_coding(raw_values,
                                 lengths,
                                 guard):
    """
    Recover symbols from run length coding.

    :param raw_values:
    :param lengths:
    :param guard:
    :warning You have to retrieve all returned streams and free them using the
    provided functions to avoid memory leaks!
    :return: 1 stream (decoded_symbols).
    """
    if type(raw_values) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if raw_values.dtype != np.uint64:
        raise TypeError("Wrong data input type.")
    if type(lengths) is not np.ndarray:
        raise TypeError("Input should be a numpy array.")
    if lengths.dtype != np.uint64:
        raise TypeError("Wrong data input type.")

    symbols_ptr = ct.pointer(ct.c_uint64())
    symbols_size = ct.c_size_t()

    gabac_rc = libgabac.gabac_inverseTransformRleCoding(
        raw_values.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(raw_values),
        lengths.ctypes.data_as(ct.POINTER(ct.c_uint64)),
        len(lengths),
        ct.c_size_t(guard),
        ct.pointer(symbols_ptr),
        ct.pointer(symbols_size)
    )

    if gabac_rc != ReturnCode.GABAC_SUCCESS:
        sys.exit("error: gabac_inverseTransformRleCoding() failed")

    symbols_ptr = ct.cast(
        symbols_ptr,
        ct.POINTER(ct.c_uint64 * symbols_size.value)
    )[0]

    symbols = np.ndarray(
        (len(symbols_ptr),),
        'uint64',
        symbols_ptr,
        order='C'
    )

    return symbols


# release
# -----------------------------------------------------------------------------

libgabac.gabac_release.argtypes = [ct.c_void_p]

libgabac.gabac_release.restype = ct.c_int


def release(chunk):
    """
    Free chunk of ubyte data.

    :param chunk:
    :return: none
    """
    libgabac.gabac_release(chunk.ctypes.data_as(ct.POINTER(ct.c_byte)))
