#ifndef GABAC_WRITER_H_
#define GABAC_WRITER_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "gabac/bit_output_stream.h"
#include "gabac/constants.h"
#include "gabac/context_model.h"
#include "gabac/context_selector.h"
#include "gabac/binary_arithmetic_encoder.h"

using std::size_t;

namespace gabac {

class Writer
{
 public:
    explicit Writer(
            std::vector<unsigned char> *bitstream
    );

    ~Writer();

    void start(
            size_t numSymbols
    );

    void reset();

    void writeBypassValue(
            int64_t symbol,
            const BinarizationId& binarizationId,
            const std::vector<unsigned int>& binarizationParameters
    );

    void writeCabacAdaptiveValue(
            int64_t symbol,
            const BinarizationId& binarizationId,
            const std::vector<unsigned int>& binarizationParameters,
            unsigned int prevValue,
            unsigned int prevPrevValue
    );

    void writeAsBIbypass(
            int64_t input,
            unsigned int cLength
    );

    void writeAsBIcabac(
            int64_t input,
            unsigned int cLength,
            unsigned int offset
    );

    void writeAsTUbypass(
            int64_t input,
            unsigned int cMax
    );

    void writeAsTUcabac(
            int64_t input,
            unsigned int cMax,
            unsigned int offset
    );

    void writeAsEGbypass(
            int64_t input
    );

    void writeAsEGcabac(
            int64_t input,
            unsigned int offset
    );

    void writeAsSEGbypass(
            int64_t input
    );

    void writeAsSEGcabac(
            int64_t input,
            unsigned int offset
    );

    void writeAsTEGbypass(
            int64_t input,
            unsigned int cTruncExpGolParam
    );

    void writeAsTEGcabac(
            int64_t input,
            unsigned int cTruncExpGolParam,
            unsigned int offset
    );

    void writeAsSTEGbypass(
            int64_t input,
            unsigned int cSignedTruncExpGolParam
    );

    void writeAsSTEGcabac(
            int64_t input,
            unsigned int cSignedTruncExpGolParam,
            unsigned int offset
    );

    void writeNumSymbols(
            unsigned int numSymbols
    );

 private:
    BitOutputStream m_bitOutputStream;

    // ContextSelector m_contextSelector;

    BinaryArithmeticEncoder m_binaryArithmeticEncoder;

    std::vector<ContextModel> m_contextModels;
};


}  // namespace gabac


#endif  // GABAC_WRITER_H_
