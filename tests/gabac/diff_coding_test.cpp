#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>
#include <limits>
#include <vector>


#include "gabac/diff_coding.h"
#include "./test_common.h"

#include "gtest/gtest.h"


class DiffCodingTest : public ::testing::Test
{
 protected:
    DiffCodingTest() = default;

    ~DiffCodingTest() override = default;

    void SetUp() override{
    }

    void TearDown() override{
    }
};
/*´

TEST_F(DiffCodingTest, transformDiffCoding){
    {
        // Void input
        gabac::DataStream symbols(0, 8);
        gabac::DataStream transformedSymbols(8);
        transformedSymbols.set(0,1);
        EXPECT_NO_THROW(gabac::transformDiffCoding(symbols, &transformedSymbols));
        EXPECT_EQ(transformedSymbols.size(), 0);
    }

    {
        // Single positive-valued symbol
        std::vector<uint64_t> symbols = {42};
        std::vector<int64_t> transformedSymbols = {};
        EXPECT_NO_THROW(gabac::transformDiffCoding(symbols, &transformedSymbols));
        EXPECT_EQ(transformedSymbols.size(), 1);
        EXPECT_EQ(transformedSymbols[0], symbols[0]);
    }

    {
        // Corner cases
        std::vector<uint64_t> symbols = {
                0,
                static_cast<uint64_t>(std::numeric_limits<int64_t>::max()),
                static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1,
                0
        };
        std::vector<int64_t> transformedSymbols = {};
        std::vector<int64_t> expectedTransformedSymbols = {
                0,
                std::numeric_limits<int64_t>::max(),
                1,
                std::numeric_limits<int64_t>::min(),
        };
        EXPECT_NO_THROW(gabac::transformDiffCoding(symbols, &transformedSymbols));
        EXPECT_EQ(transformedSymbols.size(), expectedTransformedSymbols.size());
        EXPECT_EQ(transformedSymbols, expectedTransformedSymbols);
    }
    {
        // Large input
        std::vector<uint64_t> symbols;
        std::vector<int64_t> transformedSymbols;
        size_t largeTestSize = 1024 * 1024;
        symbols.resize(largeTestSize);
        fillVectorRandomUniform<uint64_t>(
                0,
                std::numeric_limits<int64_t>::max(),
                &symbols
        );
        EXPECT_NO_THROW(gabac::transformDiffCoding(symbols, &transformedSymbols));
    }

    {
        // Too small symbol diff
        std::vector<uint64_t> symbols = {uint64_t(std::numeric_limits<int64_t>::max()) + 2, 0};
        std::vector<int64_t> transformedSymbols = {};
        ASSERT_DEATH(gabac::transformDiffCoding(symbols, &transformedSymbols), "");
    }

    {
        // Too great symbol diff
        std::vector<int64_t> transformedSymbols = {};
        std::vector<uint64_t> symbols = {0, uint64_t(std::numeric_limits<int64_t>::max()) + 1};
        ASSERT_DEATH(gabac::transformDiffCoding(symbols, &transformedSymbols), "");
    }
}


TEST_F(DiffCodingTest, inverseTransformDiffCoding){
    {
        // Void input shall lead to void output
        std::vector<int64_t> transformedSymbols;
        std::vector<uint64_t> symbols;
        transformedSymbols = {};
        symbols = {1};
        EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 0);
    }

    {
        // Correct coding of a single positive-valued symbol
        std::vector<int64_t> transformedSymbols = {42};
        std::vector<uint64_t> symbols = {};
        EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 1);
        EXPECT_EQ(symbols[0], transformedSymbols[0]);
    }
    {
        // Correct coding of a negative-valued symbol
        std::vector<int64_t> transformedSymbols = {43, -42};
        std::vector<uint64_t> symbols = {};
        EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 2);
        EXPECT_EQ(symbols[1], 1);
    }
    {
        // pos/neg sequence
        std::vector<uint64_t> expected = {100, 90, 80, 70, 60, 50};
        std::vector<uint64_t> symbols = {};
        std::vector<int64_t> transformedSymbols = {100, -10,
                                                   -10, -10,
                                                   -10, -10};
        EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 6);
        EXPECT_EQ(symbols, expected);
    }
    {
        // Cornercases
        // Edges of symbol range
        std::vector<uint64_t> expected = {std::numeric_limits<int64_t>::max(),
                                          uint64_t(std::numeric_limits<int64_t>::max()) + 1,
                                          0,
                                          std::numeric_limits<int64_t>::max(),
                                          std::numeric_limits<uint64_t>::max() - 1,
                                          uint64_t(std::numeric_limits<int64_t>::max()) - 1};
        std::vector<uint64_t> symbols = {};
        std::vector<int64_t>
                transformedSymbols = {std::numeric_limits<int64_t>::max(), 1, std::numeric_limits<int64_t>::min(),
                                      std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::max(),
                                      std::numeric_limits<int64_t>::min()};
        EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &symbols));
        EXPECT_EQ(symbols.size(), 6);
        EXPECT_EQ(symbols, expected);
    }
}

TEST_F(DiffCodingTest, roundTripCoding){
    std::vector<uint64_t> symbols;
    std::vector<int64_t> transformedSymbols;
    std::vector<uint64_t> decodedSymbols;

    size_t largeTestSize = 1024 * 1024;

    symbols.resize(largeTestSize);
    fillVectorRandomUniform<uint64_t>(0, std::numeric_limits<int64_t>::max(), &symbols);
    transformedSymbols = {};
    decodedSymbols = {};
    EXPECT_NO_THROW(gabac::transformDiffCoding(symbols, &transformedSymbols));
    EXPECT_NO_THROW(gabac::inverseTransformDiffCoding(transformedSymbols, &decodedSymbols));
    EXPECT_EQ(decodedSymbols.size(), symbols.size());
    EXPECT_EQ(decodedSymbols, symbols);
    symbols.clear();
}

*/