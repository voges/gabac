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
    gabac::DataStream bitstream(0, 1);
    bitstream = {0xFF};
    gabac::BitInputStream bitInputStream(&bitstream);
    EXPECT_EQ(0xFF, bitInputStream.readByte());
}
