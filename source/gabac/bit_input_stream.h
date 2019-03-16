#ifndef GABAC_BIT_INPUT_STREAM_H_
#define GABAC_BIT_INPUT_STREAM_H_


#include <cstdlib>
#include <cstddef>
#include <vector>

#include "gabac/data_block.h"
#include "gabac/block_stepper.h"


namespace gabac {


class BitInputStream
{
 public:
    explicit BitInputStream(
            DataBlock *bitstream
    );

    ~BitInputStream();

    unsigned int getNumBitsUntilByteAligned() const;

    unsigned char readByte();

    void reset();

 private:
    unsigned int read(
            unsigned int numBits
    );

    gabac::DataBlock m_bitstream;

    gabac::BlockStepper m_reader;

    unsigned char m_heldBits;

    unsigned int m_numHeldBits;
};


}  // namespace gabac


#endif  // GABAC_BIT_INPUT_STREAM_H_
