#include "gabac/constants.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
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
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max < int64_t(1ull << parameter);
                },
                [](uint64_t, uint64_t max, uint64_t parameter) -> bool
                {
                    return max < (1ull << parameter);
                }
        },
        {
                "TU",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && uint64_t(max) <= parameter;
                },
                [](uint64_t, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= parameter;
                }
        },
        {
                "EG",
                false,
                [](int64_t min, int64_t max, uint64_t) -> bool
                {
                    return min >= 0 && max <= int64_t((1ull << 32u) - 1u);
                },
                [](uint64_t, uint64_t max, uint64_t) -> bool
                {
                    return max <= int64_t((1ull << 32u) - 1u);
                }
        },
        {
                "SEG",
                true,
                [](int64_t min, int64_t max, uint64_t) -> bool
                {
                    return min >= -int64_t(1ull << 16u) && max <= int64_t((1ull << 16u) - 1u);
                },
                [](uint64_t, uint64_t max, uint64_t) -> bool
                {
                    return max <= ((1ull << 16u) - 1u);
                }
        },
        {
                "TEG",
                false,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= 0 && max <= int64_t((1ull << (32u - (parameter >> 1u))) - 1u);
                },
                [](uint64_t, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ull << (32u - (parameter >> 1u))) - 1u);
                }
        },
        {
                "STEG",
                true,
                [](int64_t min, int64_t max, uint64_t parameter) -> bool
                {
                    return min >= -int64_t((1ull << (16u - (parameter >> 1u))) - 1u) &&
                           max <= int64_t((1ull << (16u - (parameter >> 1u))) - 1u);
                },
                [](uint64_t, uint64_t max, uint64_t parameter) -> bool
                {
                    return max <= ((1ull << (16u - (parameter >> 1u))) - 1u);
                }
        }
};

//------------------------------------------------------------------------------

const std::vector<TransformationProperties> transformationInformation = {
        {
                "no_transform", // Name
                {"out"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    (*transformedSequences)[0] = sequence;
                },
                [](const std::vector<std::vector<uint64_t>>& transformedSequences, uint64_t,
                   std::vector<uint64_t> *const sequence
                )
                {
                    *sequence = transformedSequences[0];
                }
        },
        {
                "equality_coding", // Name
                {"eq_flags",   "raw_symbols"}, // StreamNames
                {1, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformEqualityCoding(
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](const std::vector<std::vector<uint64_t>>& transformedSequences, uint64_t,
                   std::vector<uint64_t> *const sequence
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
                [](const std::vector<uint64_t>& sequence, uint64_t param,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
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
                [](const std::vector<std::vector<uint64_t>>& transformedSequences, uint64_t,
                   std::vector<uint64_t> *const sequence
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
                [](const std::vector<uint64_t>& sequence, uint64_t param,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformRleCoding(
                            sequence,
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](const std::vector<std::vector<uint64_t>>& transformedSequences, uint64_t param,
                   std::vector<uint64_t> *const sequence
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
                {"sequence",   "lut"}, // StreamNames
                {0, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>& sequence, uint64_t,
                   std::vector<std::vector<uint64_t>> *const transformedSequences
                )
                {
                    gabac::transformLutTransform0(
                            sequence,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](const std::vector<std::vector<uint64_t>>& transformedSequences, uint64_t,
                   std::vector<uint64_t> *const sequence
                )
                {
                    gabac::inverseTransformLutTransform0(
                            transformedSequences[0],
                            transformedSequences[1],
                            sequence
                    );
                }
        },
        {
                "diff_coding", // Name
                {"sequence"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](const std::vector<uint64_t>&, uint64_t,
                   std::vector<std::vector<uint64_t>> *const
                )
                {
                },
                [](const std::vector<std::vector<uint64_t>>&, uint64_t,
                   std::vector<uint64_t> *const
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