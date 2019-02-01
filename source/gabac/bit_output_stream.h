#ifndef GABAC_BIT_OUTPUT_STREAM_H_
#define GABAC_BIT_OUTPUT_STREAM_H_


#include <vector>
#include "gabac/data_stream.h"

namespace gabac {


class BitOutputStream
{
 public:
    explicit BitOutputStream(
            DataStream *bitstream
    );

    ~BitOutputStream();

    void flush();

    void write(
            unsigned int bits,
            unsigned int numBits
    );

    void writeAlignZero();

 private:
    DataStream *m_bitstream;

    unsigned char m_heldBits;

    unsigned int m_numHeldBits;
};


}  // namespace gabac


#endif  // GABAC_BIT_OUTPUT_STREAM_H_
