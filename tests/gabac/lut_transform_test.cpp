#include <vector>

#include "gabac/lut_transform.h"
#include "iostream"
#include "./test_common.h"

#include "gtest/gtest.h"


class lutTransformTest : public ::testing::Test
{
 protected:
    void SetUp() override{
    }

    void TearDown() override{
        // Code here will be called immediately after each test
    }
};


TEST_F(lutTransformTest, roundTripCoding0){
    // Void input
    gabac::DataStream symbols(0, 1);
    symbols.resize(1024 * 1024);
    fillVectorRandomUniform(0, 64, &symbols);
    gabac::DataStream transsymbols(0, 1);
    gabac::DataStream decodedSymbols(0, 1);
    std::vector<std::pair<uint64_t, uint64_t>> lut0 = {};
    gabac::DataStream inverseLut0(0, 1);
    gabac::DataStream inverseLut1(0, 1);

    EXPECT_NO_THROW(gabac::transformLutTransform0(0, symbols, &transsymbols, &inverseLut0, &inverseLut1));
    EXPECT_NO_THROW(gabac::inverseTransformLutTransform0(0, transsymbols, inverseLut0, inverseLut1, &decodedSymbols));
    EXPECT_EQ(decodedSymbols.size(), symbols.size());
    EXPECT_EQ(decodedSymbols, symbols);

EXPECT_NO_THROW(gabac::transformLutTransform0(1, symbols, &transsymbols, &inverseLut0, &inverseLut1));
EXPECT_NO_THROW(gabac::inverseTransformLutTransform0(1, transsymbols, inverseLut0, inverseLut1, &decodedSymbols));
EXPECT_EQ(decodedSymbols.size(), symbols.size());
EXPECT_EQ(decodedSymbols, symbols);

EXPECT_NO_THROW(gabac::transformLutTransform0(2, symbols, &transsymbols, &inverseLut0, &inverseLut1));
EXPECT_NO_THROW(gabac::inverseTransformLutTransform0(2, transsymbols, inverseLut0, inverseLut1, &decodedSymbols));
EXPECT_EQ(decodedSymbols.size(), symbols.size());
EXPECT_EQ(decodedSymbols, symbols);
}
