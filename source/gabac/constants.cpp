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
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(1);
                },
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(1);
                }
        },
        {
                "equality_coding", // Name
                {"raw_symbols", "eq_flags"}, // StreamNames
                {0, 1}, // WordSizes (0: non fixed current stream wordsize)
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(2);
                    (*transformedSequences)[1] = gabac::DataStream(0, 1);
                    gabac::transformEqualityCoding(
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::inverseTransformEqualityCoding(
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                    transformedSequences->resize(1);
                }
        },
        {
                "match_coding", // Name
                {"raw_values", "pointers", "lengths"}, // StreamNames
                {0, 4, 4}, // WordSizes (0: non fixed current stream wordsize)
                [](uint64_t param,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(3);
                    assert(param <= std::numeric_limits<uint32_t>::max());
                    (*transformedSequences)[1] = gabac::DataStream(0, 4);
                    (*transformedSequences)[2] = gabac::DataStream(0, 4);
                    gabac::transformMatchCoding(
                            static_cast<uint32_t>(param),
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                },
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::inverseTransformMatchCoding(
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                    transformedSequences->resize(1);
                }
        },
        {
                "rle_coding", // Name
                {"raw_values", "lengths"}, // StreamNames
                {0, 4}, // WordSizes (0: non fixed current stream wordsize)
                [](uint64_t param,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(2);
                    (*transformedSequences)[1] = gabac::DataStream(0, 4);
                    gabac::transformRleCoding(
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                },
                [](uint64_t param,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::inverseTransformRleCoding(
                            param,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1]
                    );
                    transformedSequences->resize(1);
                }
        },
        {
                "lut_coding", // Name
                {"sequence",   "lut0", "lut1"}, // StreamNames
                {0, 0, 0}, // WordSizes (0: non fixed current stream wordsize)
                [](uint64_t order,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(3);
                    (*transformedSequences)[1] = gabac::DataStream(0, (*transformedSequences)[0].getWordSize());
                    (*transformedSequences)[2] = gabac::DataStream(0, (*transformedSequences)[0].getWordSize());
                    gabac::transformLutTransform0(
                            order,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                    transformedSequences->resize(1);
                },
                [](uint64_t order,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    gabac::inverseTransformLutTransform0(
                            order,
                            &(*transformedSequences)[0],
                            &(*transformedSequences)[1],
                            &(*transformedSequences)[2]
                    );
                    transformedSequences->resize(1);
                }
        },
        {
                "diff_coding", // Name
                {"sequence"}, // StreamNames
                {0}, // WordSizes (0: non fixed current stream wordsize)
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(1);
                    gabac::transformDiffCoding(&(*transformedSequences)[0]);
                },
                [](uint64_t,
                   std::vector<DataStream> *const transformedSequences
                )
                {
                    transformedSequences->resize(1);
                    gabac::inverseTransformDiffCoding(&(*transformedSequences)[0]);
                }
        }
};

//------------------------------------------------------------------------------

}