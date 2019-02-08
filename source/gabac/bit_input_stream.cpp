#include "gabac/bit_input_stream.h"

#include <cassert>
#include <limits>
#include <vector>


namespace gabac {


static unsigned char readIn(
        gabac::StreamReader* reader
){
    if(!reader->isValid())
        throw std::runtime_error("Index out of bounds");
    auto byte = static_cast<unsigned char>(reader->get());
    reader->inc();
    return byte;
}


BitInputStream::BitInputStream(
        DataStream* const bitstream
)
        : m_heldBits(0), m_numHeldBits(0){
    m_bitstream.swap(bitstream);
    m_reader = m_bitstream.getReader();
    reset();
}


BitInputStream::~BitInputStream() = default;


unsigned int BitInputStream::getNumBitsUntilByteAligned() const{
    return m_numHeldBits & 0x7u;
}


unsigned char BitInputStream::readByte(){
    unsigned int result = read(8);
    assert(result <= std::numeric_limits<unsigned char>::max());
    return static_cast<unsigned char>(result);
}


void BitInputStream::reset(){
    m_heldBits = 0;
    m_numHeldBits = 0;
    m_reader = m_bitstream.getReader();
}


unsigned int BitInputStream::read(
        unsigned int numBits
){
    assert(numBits <= 32);

    unsigned int bits = 0;
    if (numBits <= m_numHeldBits)
    {
        // Get numBits most significant bits from heldBits as bits
        bits = m_heldBits >> (m_numHeldBits - numBits);
        bits &= ~(0xffu << numBits);
        m_numHeldBits -= numBits;
        return bits;
    }

    // More bits requested than currently held, flush all heldBits to bits
    numBits -= m_numHeldBits;
    bits = m_heldBits & ~(0xffu << m_numHeldBits);
    bits <<= numBits;  // make room for the bits to come

    // Read in more bytes to satisfy the request
    unsigned int numBytesToLoad = ((numBits - 1u) >> 3u) + 1;
    unsigned int alignedWord = 0;
    switch (numBytesToLoad)
    {
        case 4:
        {
            alignedWord |= (readIn(&m_reader) << 24u);
        }  // fall-through
        case 3:
        {
            alignedWord |= (readIn(&m_reader) << 16u);
        }  // fall-through
        case 2:
        {
            alignedWord |= (readIn(&m_reader) << 8u);
        }  // fall-through
        case 1:
        {
            alignedWord |= (readIn(&m_reader));
        }  // fall-through
        default:
        {
            // Nothing to do here
        }  // fall-through
    }

    // Append requested bits and hold the remaining read bits
    unsigned int numNextHeldBits = (32 - numBits) % 8;
    bits |= alignedWord >> numNextHeldBits;
    m_numHeldBits = numNextHeldBits;
    assert(alignedWord <= std::numeric_limits<unsigned char>::max());
    m_heldBits = static_cast<unsigned char>(alignedWord);

    return bits;
}


}  // namespace gabac
