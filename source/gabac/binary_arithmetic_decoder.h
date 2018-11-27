#ifndef GABAC_BINARY_ARITHMETIC_DECODER_H_
#define GABAC_BINARY_ARITHMETIC_DECODER_H_


#include "gabac/bit_input_stream.h"
#include "gabac/context_model.h"


namespace gabac {


class BinaryArithmeticDecoder
{
 public:
    explicit BinaryArithmeticDecoder(
            const BitInputStream& bitInputStream
    );

    ~BinaryArithmeticDecoder();

    unsigned int decodeBin(
            ContextModel *contextModel
    );

    unsigned int decodeBinsEP(
            unsigned int numBins
    );

    void decodeBinTrm();

    void reset();

 private:
    void start();

    BitInputStream m_bitInputStream;

    int m_numBitsNeeded = 0;

    unsigned int m_range = 0;

    unsigned int m_value = 0;
};


}  // namespace gabac


#endif  // GABAC_BINARY_ARITHMETIC_DECODER_H_
