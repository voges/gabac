#ifndef GABAC_BIT_INPUT_STREAM_H_
#define GABAC_BIT_INPUT_STREAM_H_


#include <cstdlib>
#include <vector>

#include "gabac/data_stream.h"

using std::size_t;

namespace gabac {


class BitInputStream
{
 public:
    explicit BitInputStream(
            const gabac::DataStream& bitstream
    );

    ~BitInputStream();

    unsigned int getNumBitsUntilByteAligned() const;

    unsigned char readByte();

    void reset();

 private:
    unsigned int read(
            unsigned int numBits
    );

    gabac::DataStream m_bitstream;

    size_t m_bitstreamIndex;

    unsigned char m_heldBits;

    unsigned int m_numHeldBits;
};


}  // namespace gabac


#endif  // GABAC_BIT_INPUT_STREAM_H_
