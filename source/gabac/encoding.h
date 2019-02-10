#ifndef GABAC_ENCODING_H_
#define GABAC_ENCODING_H_


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#include <stdint.h> /* NOLINT */
#include <stdlib.h> /* NOLINT */


int gabac_encode(
        int64_t *symbols,
        size_t symbolsSize,
        unsigned int binarizationId,
        unsigned int *binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        unsigned char **bitstream,
        size_t *bitstreamSize
);


#ifdef __cplusplus
}  // extern "C"


#include <vector>

#include "gabac/constants.h"
#include "gabac/data_block.h"
#include "gabac/configuration.h"


namespace gabac {

class OutputStream;
class InputStream;

int encode_cabac(
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *const symbols
);

void doDiffTransform(bool enabled,
                     gabac::DataBlock *diffAndLutTransformedSequence
);

void doLutTransform(bool enabled,
                    unsigned int order,
                    std::vector<gabac::DataBlock> *lutSequences,
                    unsigned *bits0,
                    gabac::OutputStream* out
);

void doSequenceTransform(gabac::DataBlock& sequence,
                         const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataBlock> *transformedSequences
);

void encode(
        const gabac::Configuration& configuration,
        gabac::InputStream* inStream,
        gabac::OutputStream* outStream,
        size_t blocksize
);

}  // namespace gabac

#endif  /* __cplusplus */
#endif  /* GABAC_ENCODING_H_ */
