#include <vector>

#include "gabac/rle_coding.h"
#include "iostream"

#include "gtest/gtest.h"
#include "./test_common.h"


class rleCodingTest : public ::testing::Test
{
 protected:
    void SetUp() override{
    }

    void TearDown() override{
        // Code here will be called immediately after each test
    }
};


TEST_F(rleCodingTest, transformRleCoding){
    {
        // Void input
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> rawSymbols = {1};
        std::vector<uint64_t> lengths = {1};
        EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
        EXPECT_EQ(rawSymbols.size(), 0);
        EXPECT_EQ(lengths.size(), 0);
    }
    {
        // Single positive-valued symbol
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {42};
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> lengths = {};
        EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
        EXPECT_EQ(rawSymbols.size(), 1);
        EXPECT_EQ(rawSymbols[0], symbols[0]);
        EXPECT_EQ(lengths.size(), 1);
        EXPECT_EQ(lengths[0], 0);
    }
    {
        // Single negative-valued symbol
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {uint64_t(-42)};
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> lengths = {};
        EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
        EXPECT_EQ(rawSymbols.size(), 1);
        EXPECT_EQ(rawSymbols[0], symbols[0]);
        EXPECT_EQ(lengths.size(), 1);
        EXPECT_EQ(lengths[0], 0);
    }
    {
        // Guard triggered
        uint64_t guard = 2;
        std::vector<uint64_t> symbols = {1, 1, 1, 1, 1};
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> lengths = {};
        std::vector<uint64_t> expectedLengths = {2, 2, 0};
        EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
        EXPECT_EQ(rawSymbols.size(), 1);
        EXPECT_EQ(rawSymbols[0], symbols[0]);
        EXPECT_EQ(lengths.size(), 3);
        EXPECT_EQ(lengths, expectedLengths);
    }
    {
        // Random sequence with positive and negative values
        std::vector<uint64_t> symbols = {
                uint64_t(-3438430427565543845LL),
                uint64_t(-3438430427565543845LL),
                8686590606261860295LL,
                810438489069303389LL,
                810438489069303389LL,
                810438489069303389LL,
                0
        };
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> lengths = {};
        std::vector<uint64_t> expectedRawSymbols = {
                uint64_t(-3438430427565543845LL),
                8686590606261860295LL,
                810438489069303389LL,
                0
        };
        std::vector<uint64_t> expectedLengths = {
                1, 0, 2, 0
        };
        uint64_t guard = 42;
        EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
        EXPECT_EQ(rawSymbols.size(), expectedRawSymbols.size());
        EXPECT_EQ(rawSymbols, expectedRawSymbols);

        EXPECT_EQ(lengths.size(), expectedLengths.size());
        EXPECT_EQ(lengths, expectedLengths);
    }
}

TEST_F(rleCodingTest, inverseTransformRleCoding){
    {
        // Void input
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> rawSymbols = {};
        std::vector<uint64_t> lengths = {3};
        EXPECT_DEATH(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &symbols), "");
    }
    {
        // Void input
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> rawSymbols = {5};
        std::vector<uint64_t> lengths = {};
        EXPECT_DEATH(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &symbols), "");
    }
    {
        // Single positive-valued symbol
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> rawSymbols = {42};
        std::vector<uint64_t> lengths = {0};
        EXPECT_NO_THROW(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &symbols));
        EXPECT_EQ(symbols.size(), 1);
        EXPECT_EQ(symbols[0], 42);
    }
    {
        // Two negative-valued symbols
        uint64_t guard = 42;
        std::vector<uint64_t> symbols = {};
        std::vector<uint64_t> rawSymbols = {uint64_t(-42)};
        std::vector<uint64_t> lengths = {2};
        std::vector<uint64_t> expected = {uint64_t(-42), uint64_t(-42), uint64_t(-42)};
        EXPECT_NO_THROW(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &symbols));
        EXPECT_EQ(symbols.size(), 3);
        EXPECT_EQ(symbols, expected);
    }
    {
        std::vector<uint64_t> symbols;
        uint64_t guard = 42;
        // Random sequence with positive and negative values
        std::vector<uint64_t> expected = {
                uint64_t(-3438430427565543845LL),
                uint64_t(-3438430427565543845LL),
                8686590606261860295LL,
                810438489069303389LL,
                810438489069303389LL,
                810438489069303389LL,
                0
        };

        std::vector<uint64_t> rawSymbols = {
                uint64_t(-3438430427565543845LL),
                8686590606261860295LL,
                810438489069303389LL,
                0
        };
        std::vector<uint64_t> lengths = {
                1, 0, 2, 0
        };
        EXPECT_NO_THROW(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &symbols));
        EXPECT_EQ(symbols.size(), expected.size());
        EXPECT_EQ(symbols, expected);
    }
}

TEST_F(rleCodingTest, roundTripCoding){
    std::vector<uint64_t> symbols;
    std::vector<uint64_t> rawSymbols;
    std::vector<uint64_t> lengths;
    std::vector<uint64_t> decodedSymbols;
    uint64_t guard = 42;

    // A lot of input data - WordSize
    symbols.resize(1 * 1024 * 1024);  // 256M symbols -> 1GB
    fillVectorRandomGeometric<uint64_t>(&symbols);
    rawSymbols = {};
    lengths = {};
    decodedSymbols = {};
    EXPECT_NO_THROW(gabac::transformRleCoding(symbols, guard, &rawSymbols, &lengths));
    EXPECT_NO_THROW(gabac::inverseTransformRleCoding(rawSymbols, lengths, guard, &decodedSymbols));
    EXPECT_EQ(decodedSymbols.size(), symbols.size());
    EXPECT_EQ(decodedSymbols, symbols);
    symbols.clear();
}
