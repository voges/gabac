#include "gabac/constants.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "gabac/equality_coding.h"
#include "gabac/diff_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

namespace gabac {


//------------------------------------------------------------------------------


const std::vector<BinarizationProperties> binarizationInformation = {
        {
                "BI",
                1,
                32,
                [](uint64_t) -> int64_t
                {
                    return 0;
                },
                [](uint64_t parameter) -> int64_t
                {
                    return uint64_t((1ull << (parameter)) - 1u);
                },
        },
        {
                "TU",
                1,
                32,
                [](uint64_t) -> int64_t
                {
                    return 0;
                },
                [](uint64_t parameter) -> int64_t
                {
                    return parameter;
                },
        },
        {
                "EG",
                0,
                0,
                [](uint64_t) -> int64_t
                {
                    return 0;
                },
                [](uint64_t) -> int64_t
                {
                    return std::numeric_limits<int32_t>::max();
                },
        },
        {
                "SEG",
                0,
                0,
                [](uint64_t) -> int64_t
                {
                    return std::numeric_limits<int32_t>::min() / 2;
                },
                [](uint64_t) -> int64_t
                {
                    return std::numeric_limits<int32_t>::max() / 2;
                },
        },
        {
                "TEG",
                0,
                255,
                [](uint64_t) -> int64_t
                {
                    return 0;
                },
                [](uint64_t param) -> int64_t
                {
                    return std::numeric_limits<int32_t>::max() + param;
                },
        },
        {
                "STEG",
                0,
                255,
                [](uint64_t param) -> int64_t
                {
                    return std::numeric_limits<int32_t>::min() / 2ll - param;
                },
                [](uint64_t param) -> int64_t
                {
                    return std::numeric_limits<int32_t>::max() / 2ll + param;
                },
        }
};

//------------------------------------------------------------------------------

const std::vector<TransformationProperties> transformationInformation = {
        {
                "no_transform", // Name
                {"out"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](DataStream& sequence, uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    (*transformedSequences)[0].swap(sequence);
                },
                [](std::vector<DataStream>& transformedSequences, uint64_t,
                   DataStream *const sequence
                )
                {
                    sequence->swap(transformedSequences[0]);
                }
        },
        {
                "equality_coding", // Name
                {"eq_flags",   "raw_symbols"}, // StreamNames
                {1, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const DataStream& sequence, uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::transformEqualityCoding(
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](const std::vector<DataStream>& transformedSequences, uint64_t,
                   DataStream *const sequence
                )
                {
                    gabac::inverseTransformEqualityCoding(
                            transformedSequences[0],
                            transformedSequences[1],
                            sequence
                    );
                }
        },
        {
                "match_coding", // Name
                {"pointers",   "lengths", "raw_values"}, // StreamNames
                {4, 4, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const DataStream& sequence, uint64_t param,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    assert(param <= std::numeric_limits<uint32_t>::max());
                    gabac::transformMatchCoding(
                            sequence,
                            static_cast<uint32_t>(param),
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                },
                [](const std::vector<DataStream>& transformedSequences, uint64_t,
                   DataStream *const sequence
                )
                {
                    gabac::inverseTransformMatchCoding(
                            transformedSequences[0],
                            transformedSequences[1],
                            transformedSequences[2],
                            sequence
                    );
                }
        },
        {
                "rle_coding", // Name
                {"raw_values", "lengths"}, // StreamNames
                {0, 4}, // WordSizes (0: non fixed current stream wordsize)
                [](const DataStream& sequence, uint64_t param,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::transformRleCoding(
                            sequence,
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](const std::vector<DataStream>& transformedSequences, uint64_t param,
                   DataStream *const sequence
                )
                {
                    gabac::inverseTransformRleCoding(
                            transformedSequences[0],
                            transformedSequences[1],
                            param,
                            sequence
                    );
                }
        },
        {
                "lut_coding", // Name
                {"sequence",   "lut0", "lut1"}, // StreamNames
                {0, 0, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const DataStream& sequence, uint64_t order,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::transformLutTransform0(
                            order,
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                },
                [](const std::vector<DataStream>& transformedSequences, uint64_t order,
                   DataStream *const sequence
                )
                {
                    gabac::inverseTransformLutTransform0(
                            order,
                            transformedSequences[0],
                            transformedSequences[1],
                            transformedSequences[2],
                            sequence
                    );
                }
        },
        {
                "diff_coding", // Name
                {"sequence"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](const DataStream&, uint64_t,
                   std::vector<DataStream> *const
                )
                {
                },
                [](const std::vector<DataStream>&, uint64_t,
                   DataStream *const
                )
                {
                }
        }
};

//------------------------------------------------------------------------------

std::vector<unsigned> fixWordSizes(const std::vector<unsigned>& list, unsigned wordsize){
    std::vector<unsigned> ret = list;
    std::replace(ret.begin(), ret.end(), 0u, wordsize);
    return ret;
}

}