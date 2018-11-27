#ifndef GABAC_BIT_INPUT_STREAM_H_
#define GABAC_BIT_INPUT_STREAM_H_


#include <cstdlib>
#include <vector>

using std::size_t;

namespace gabac {


class BitInputStream
{
 public:
    explicit BitInputStream(
            const std::vector<unsigned char>& bitstream
    );

    ~BitInputStream();

    unsigned int getNumBitsUntilByteAligned() const;

    unsigned char readByte();

    void reset();

 private:
    unsigned int read(
            unsigned int numBits
    );

    std::vector<unsigned char> m_bitstream;

    size_t m_bitstreamIndex;

    unsigned char m_heldBits;

    unsigned int m_numHeldBits;
};


}  // namespace gabac


#endif  // GABAC_BIT_INPUT_STREAM_H_
