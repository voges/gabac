#include <vector>

#include "gabac/equality_coding.h"
#include "iostream"
#include "./test_common.h"

#include "gtest/gtest.h"


class equalityCodingTest : public ::testing::Test
{
 protected:
    void SetUp() override{
    }

    void TearDown() override{
        // Code here will be called immediately after each test
    }
};


TEST_F(equalityCodingTest, transformEqualityCoding){
    {
        // Void input
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {1};
        std::vector<uint64_t> rawSymbols = {1};
        EXPECT_NO_THROW(gabac::transformEqualityCoding(symbols, &flags, &rawSymbols));
        EXPECT_EQ(flags.size(), 0);
        EXPECT_EQ(rawSymbols.size(), 0);
    }
    {
        // Single positive-valued symbol
        std::vector<uint64_t> symbols = {42};
        std::vector<uint64_t> flags = {};
        std::vector<uint64_t> rawSymbols = {};
        EXPECT_NO_THROW(gabac::transformEqualityCoding(symbols, &flags, &rawSymbols));
        EXPECT_EQ(rawSymbols.size(), 1);
        EXPECT_EQ(rawSymbols[0], 41);
        EXPECT_EQ(flags.size(), 1);
        EXPECT_EQ(flags[0], 0);
    }
    {
        // Random sequence
        std::vector<uint64_t> symbols = {
                uint64_t(-3438430427565543845LL),
                uint64_t(-3438430427565543845LL),
                8686590606261860295LL,
                810438489069303389LL,
                810438489069303389LL,
                810438489069303389LL,
                0
        };
        std::vector<uint64_t> flags = {};
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> expectedRawSymbols = {
                uint64_t(-3438430427565543845LL) - 1,
                8686590606261860295LL,
                810438489069303389LL,
                0
        };
        std::vector<uint64_t> expectedFlags = {
                0, 1, 0, 0, 1, 1, 0
        };
        EXPECT_NO_THROW(gabac::transformEqualityCoding(symbols, &flags, &rawSymbols));
        EXPECT_EQ(flags.size(), symbols.size());
        EXPECT_EQ(flags, expectedFlags);

        EXPECT_EQ(rawSymbols.size(), expectedRawSymbols.size());
        EXPECT_EQ(rawSymbols, expectedRawSymbols);
    }
}

TEST_F(equalityCodingTest, inverseTransformEqualityCoding){
    {
        // Void input
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {1};
        std::vector<uint64_t> rawSymbols = {};
        EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 1);
        EXPECT_EQ(symbols[0], 0);
    }
    {
        // void input
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {};
        std::vector<uint64_t> rawSymbols = {1};
        EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 0);
    }
    {
        // Single positive-valued symbol
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {0};
        std::vector<uint64_t> rawSymbols = {41};
        EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 1);
        EXPECT_EQ(symbols[0], 42);
    }
    {
        // Single negative-valued symbol
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {0};
        std::vector<uint64_t> rawSymbols = {uint64_t(-42)};
        EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 1);
        EXPECT_EQ(symbols[0], uint64_t(-42) + 1);
    }
    {
        // Random sequence with positive and negative values
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> flags = {0, 1, 0, 0, 1, 1, 0};
        std::vector<uint64_t> expectedSymbols = {uint64_t(-3438430427565543845LL) + 1,
                                                 uint64_t(-3438430427565543845LL) + 1,
                                                 8686590606261860294LL,
                                                 810438489069303389LL,
                                                 810438489069303389LL,
                                                 810438489069303389LL,
                                                 0};
        std::vector<uint64_t> rawSymbols = {
                uint64_t(-3438430427565543845LL),
                8686590606261860294LL,
                810438489069303389LL,
                0
        };
        EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &symbols));
        EXPECT_EQ(symbols.size(), expectedSymbols.size());
        EXPECT_EQ(symbols, expectedSymbols);
    }
}

TEST_F(equalityCodingTest, roundTripCoding){
    std::vector<uint64_t> symbols;
    std::vector<uint64_t> rawSymbols;
    std::vector<uint64_t> flags;
    std::vector<uint64_t> decodedSymbols;

    // A lot of input data - WordSize
    symbols.resize(1024 * 1024);
    fillVectorRandomGeometric<uint8_t>(&symbols);
    flags = {};
    decodedSymbols = {};
    rawSymbols = {};
    EXPECT_NO_THROW(gabac::transformEqualityCoding(symbols, &flags, &rawSymbols));
    EXPECT_NO_THROW(gabac::inverseTransformEqualityCoding(flags, rawSymbols, &decodedSymbols));
    EXPECT_EQ(decodedSymbols.size(), symbols.size());
    EXPECT_EQ(decodedSymbols, symbols);
    symbols.clear();
}
