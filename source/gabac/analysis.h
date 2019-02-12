#ifndef GABACIFY_ANALYSIS_H_
#define GABACIFY_ANALYSIS_H_


#include <vector>

#include "gabac/configuration.h"


namespace gabac {

inline const AnalysisConfiguration& getCandidateConfig(){
    static const AnalysisConfiguration config = {
            { // Wordsizes
                    1,
                       2,
                          4,
                             8
            },
            { // Sequence Transformations
                    gabac::SequenceTransformationId::no_transform,
                       gabac::SequenceTransformationId::equality_coding,
                          gabac::SequenceTransformationId::match_coding,
                             gabac::SequenceTransformationId::rle_coding
            },
            { // Match coding window sizes
                    32,
                       256
            },
            { // RLE Guard
                    255
            },
            { // LUT transform
                    false,
                       true
            },
            { // Diff transform
                    false,
                       true
            },
            { // Binarizations (unsigned)
                    gabac::BinarizationId::BI,
                       gabac::BinarizationId::TU,
                          gabac::BinarizationId::EG,
                             gabac::BinarizationId::TEG
            },
            { // Binarizations (signed)
                    gabac::BinarizationId::SEG,
                       gabac::BinarizationId::STEG
            },
            { // Binarization parameters (TEG and STEG only)
                    1, 2, 3, 5, 7, 9,
                    15, 30, 255
            },
            { // Context modes
                    // gabac::ContextSelectionId::bypass,
                    gabac::ContextSelectionId::adaptive_coding_order_0,
                       gabac::ContextSelectionId::adaptive_coding_order_1,
                          gabac::ContextSelectionId::adaptive_coding_order_2
            },
            { // LUT order
                    0,
                       1
            }
    };
    return config;
}

size_t analyze(const IOConfiguration& ioconf,
               const AnalysisConfiguration& aconf,
               EncodingConfiguration *econf
);

}


#endif  // GABACIFY_ANALYSIS_H_
