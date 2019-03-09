#ifndef GABAC_READER_H_
#define GABAC_READER_H_

#include <cstdint>
#include <vector>

#include "gabac/bit_input_stream.h"
#include "gabac/constants.h"
#include "gabac/context_model.h"
#include "gabac/context_selector.h"
#include "gabac/binary_arithmetic_decoder.h"

using std::size_t;

namespace gabac {


class Reader
{
 public:
    explicit Reader(
            const std::vector<unsigned char>& bitstream
    );

    ~Reader();

    size_t readNumSymbols();

    int64_t readAsBIbypass(
            unsigned int cLength
    );

    int64_t readAsBIcabac(
            unsigned int cLength,
            unsigned int offset
    );

    int64_t readAsTUbypass(
            unsigned int cMax
    );

    int64_t readAsTUcabac(
            unsigned int cMax,
            unsigned int offset
    );

    int64_t readAsEGbypass(
            unsigned int dummy
    );

    int64_t readAsEGcabac(
            unsigned int dummy,
            unsigned int offset
    );

    int64_t readAsSEGbypass(
            unsigned int dummy
    );

    int64_t readAsSEGcabac(
            unsigned int dummy,
            unsigned int offset
    );

    int64_t readAsTEGbypass(
            unsigned int treshold
    );

    int64_t readAsTEGcabac(
            unsigned int treshold,
            unsigned int offset
    );

    int64_t readAsSTEGbypass(
            unsigned int treshold
    );

    int64_t readAsSTEGcabac(
            unsigned int treshold,
            unsigned int offset
    );

    size_t start();

    void reset();

 private:
    BitInputStream m_bitInputStream;

    // ContextSelector m_contextSelector;

    BinaryArithmeticDecoder m_decBinCabac;

    std::vector<ContextModel> m_contextModels;
};


}  // namespace gabac


#endif  // GABAC_READER_H_
