#include <vector>

#include "gabac/bit_input_stream.h"

#include "gtest/gtest.h"


class BitInputStreamTest : public ::testing::Test
{
 protected:
    void SetUp() override{
        // Code here will be called immediately before each test
    }

    void TearDown() override{
        // Code here will be called immediately after each test
    }
};


TEST_F(BitInputStreamTest, readByte){
    std::vector<unsigned char> bitstream = {0xFF};
    gabac::BitInputStream bitInputStream(bitstream);
    EXPECT_EQ(bitstream[0], bitInputStream.readByte());
}
