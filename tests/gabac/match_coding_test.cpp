#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>
#include <limits>
#include <vector>
#include <random>

#include "gabac/match_coding.h"
#include "./test_common.h"

#include "gtest/gtest.h"


class matchCodingTest : public ::testing::Test
{
 protected:
    const unsigned int largeTestSize = 1 * 1024 * 1024;
    std::vector<int>
            windowSizes = {1024};  // minimum value: 0,
            // which should not perform a search, maximum reasonable value: 32768

    void SetUp() override{
        // Code here will be called immediately before each test
    }

    void TearDown() override{
        // Code here will be called immediately after each test
    }
};
/*

TEST_F(matchCodingTest, transformMatchCoding){
    std::vector<uint64_t> symbols;
    std::vector<uint64_t> pointers;
    std::vector<uint64_t> expectedPointers;
    std::vector<uint64_t> lengths;
    std::vector<uint64_t> expectedLengths;
    std::vector<uint64_t> rawValues;
    std::vector<uint64_t> expectedRawValues;
    int windowSize;

    // Void input
    symbols = {};
    pointers = {};
    lengths = {};
    rawValues = {};
    for (auto& windowSizeIt : windowSizes)
    {
        EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSizeIt, &pointers, &lengths, &rawValues));
        EXPECT_EQ(pointers.size(), 0);
        EXPECT_EQ(lengths.size(), 0);
        EXPECT_EQ(rawValues.size(), 0);
    }

    // windowSize == 0 - should return raw_values
    // note: windowSize 1 will also result in no match as our implementation
    // requires length to be >2, but according to the standard is allowed
    symbols = {uint64_t(-1), 2, uint64_t(-3), 4, 4, uint64_t(-3), 2, uint64_t(-1)};
    pointers = {};
    lengths = {};
    rawValues = {};
    expectedRawValues = {uint64_t(-1), 2, uint64_t(-3), 4, 4, uint64_t(-3), 2, uint64_t(-1)};
    windowSize = 0;
    EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues));
    EXPECT_EQ(pointers.size(), 0);
    EXPECT_EQ(lengths.size(), symbols.size());
    EXPECT_EQ(rawValues.size(), 8);
    EXPECT_EQ(rawValues, expectedRawValues);

    // windowSize == 4
    symbols = {uint64_t(-1), 2, uint64_t(-3), 4, 2, uint64_t(-3), 4, uint64_t(-1)};
    pointers = {};
    expectedPointers = {3};
    lengths = {};
    expectedLengths = {0, 0, 0, 0, 3, 0};
    rawValues = {};
    expectedRawValues = {uint64_t(-1), 2, uint64_t(-3), 4, uint64_t(-1)};
    windowSize = 4;
    EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues));
    EXPECT_EQ(pointers.size(), 1);
    EXPECT_EQ(pointers, expectedPointers);
    EXPECT_EQ(lengths.size(), 6);
    EXPECT_EQ(lengths, expectedLengths);
    EXPECT_EQ(rawValues.size(), 5);
    EXPECT_EQ(rawValues, expectedRawValues);

    // // test large file size word size 1
    // if (largeTestSize>1*1024*1024)
    // {
    //   symbols.resize(1*1024*1024);//set number of symbols//NOTE: this is a very costly test,
    //   //will not be performed in the roundtrip test and therefore test size is decreased
    // }
    // else
    // {
    //   symbols.resize(largeTestSize);
    // }
    // fillVectorRandomByte(&symbols);
    // for (auto &windowSize : windowSizes)
    // {
    //   EXPECT_NO_THROW(gabac::transformMatchCoding(symbols,windowSize,&pointers,&lengths,&rawValues));
    //   size_t pointerI=0;
    //   size_t lengthI=0;
    //   int max=0;
    //   for (lengthI=0;lengthI<lengths.size();lengthI++){
    //     if(lengths[lengthI]>0){
    //       if(pointers[pointerI]+lengths[lengthI] > max)
    //       {
    //         max=pointers[pointerI]+lengths[lengthI];
    //         EXPECT_LT(max,windowSize+1);//the sum of pointer and length should never exceed the windowSize//
    //       }
    //       pointerI++;
    //     }
    //   }
    // }
    //
    // // test large file size word size 2
    // if (largeTestSize>1*1024*1024)
    // {
    //   symbols.resize(1*1024*1024);//set number of symbols//NOTE: this is a very costly test,
    //  // will not be performed in the roundtrip test and therefore test size is decreased
    // }
    // else
    // {
    //   symbols.resize(largeTestSize);
    // }
    // fillVectorRandomShort(&symbols);
    // for (auto &windowSize : windowSizes)
    // {
    //   EXPECT_NO_THROW(gabac::transformMatchCoding(symbols,windowSize,&pointers,&lengths,&rawValues));
    //   size_t pointerI=0;
    //   size_t lengthI=0;
    //   int max=0;
    //   for (lengthI=0;lengthI<lengths.size();lengthI++){
    //     if(lengths[lengthI]>0){
    //       if(pointers[pointerI]+lengths[lengthI] > max)
    //       {
    //         max=pointers[pointerI]+lengths[lengthI];
    //         EXPECT_LT(max,windowSize+1);//the sum of pointer and length should never exceed the windowSize//
    //       }
    //       pointerI++;
    //     }
    //   }
    //   //EXPECT_NO_THROW(gabac::transformMatchCoding(symbols,windowSize,&pointers,&lengths,&rawValues));
    //   //symbols.clear();
    // }
    // // test large file size word size 4
    // if (largeTestSize>1*1024*1024)
    // {
    //   symbols.resize(1*1024*1024);//set number of symbols//NOTE: this is a very costly test,
    //  //will not be performed in the roundtrip test and therefore test size is decreased
    // }
    // else
    // {
    //   symbols.resize(largeTestSize);
    // }
    // fillVectorRandomInt(&symbols);
    // for (auto &windowSize : windowSizes)
    // {
    //   EXPECT_NO_THROW(gabac::transformMatchCoding(symbols,windowSize,&pointers,&lengths,&rawValues));
    //   size_t pointerI=0;
    //   size_t lengthI=0;
    //   int max=0;
    //   for (lengthI=0;lengthI<lengths.size();lengthI++){
    //     if(lengths[lengthI]>0){
    //       if(pointers[pointerI]+lengths[lengthI] > max)
    //       {
    //         max=pointers[pointerI]+lengths[lengthI];
    //         EXPECT_LT(max,windowSize+1);//the sum of pointer and length should never exceed the windowSize//
    //       }
    //       pointerI++;
    //     }
    //   }
    //   // EXPECT_NO_THROW(gabac::transformMatchCoding(symbols,windowSize,&pointers,&lengths,&rawValues));
    //   // symbols.clear();
    // }
}


TEST_F(matchCodingTest, inverseTransformMatchCoding){
    std::vector<uint64_t> symbols;
    std::vector<uint64_t> expectedSymbols;
    std::vector<uint64_t> pointers;
    std::vector<uint64_t> lengths;
    std::vector<uint64_t> rawValues;

    // Void input shall lead to void output
    pointers = {};
    lengths = {};
    rawValues = {};
    symbols = {1};
    EXPECT_NO_THROW(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &symbols));
    EXPECT_EQ(symbols.size(), 0);


    // void rawValues (is not allowed) and lengths is not
    pointers = {};
    lengths = {0, 0, 0, 0};
    rawValues = {};
    symbols = {1};

    // void rawValues (is not allowed) and lengths is not
    EXPECT_DEATH(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &symbols), "");
    pointers = {0, 0, 0, 0};
    lengths = {};
    rawValues = {};
    symbols = {1};
    EXPECT_DEATH(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &symbols), "");

    // windowSize == 4
    expectedSymbols = {uint64_t(-1), 2, uint64_t(-3), 4, 2, uint64_t(-3), 4, uint64_t(-1)};
    pointers = {3};
    lengths = {0, 0, 0, 0, 3, 0};
    rawValues = {uint64_t(-1), 2, uint64_t(-3), 4, uint64_t(-1)};
    EXPECT_NO_THROW(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &symbols));
    EXPECT_EQ(symbols.size(), expectedSymbols.size());
    EXPECT_EQ(symbols, expectedSymbols);
}


TEST_F(matchCodingTest, roundTripCoding){
    std::vector<uint64_t> symbols;
    std::vector<uint64_t> decodedSymbols;
    std::vector<uint64_t> pointers;
    std::vector<uint64_t> lengths;
    std::vector<uint64_t> rawValues;


    // test large file size word size 1
    symbols.resize(largeTestSize);
    fillVectorRandomUniform<uint64_t>(
            std::numeric_limits<uint8_t>::min(),
            std::numeric_limits<uint8_t>::max(),
            &symbols
    );
    for (auto& windowSize : windowSizes)
    {
        EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues));
        EXPECT_NO_THROW(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &decodedSymbols));
        EXPECT_EQ(symbols.size(), decodedSymbols.size());
        EXPECT_EQ(symbols, decodedSymbols);
    }

    // test large file size word size 2
    symbols.resize(largeTestSize);
    fillVectorRandomUniform<uint64_t>(
            std::numeric_limits<uint16_t>::min(),
            std::numeric_limits<uint16_t>::max(),
            &symbols
    );
    for (auto& windowSize : windowSizes)
    {
        EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues));
        EXPECT_NO_THROW(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &decodedSymbols));
        EXPECT_EQ(symbols.size(), decodedSymbols.size());
        EXPECT_EQ(symbols, decodedSymbols);
    }
    // test large file size word size 4
    symbols.resize(largeTestSize);
    fillVectorRandomUniform<uint64_t>(
            std::numeric_limits<uint32_t>::min(),
            std::numeric_limits<uint32_t>::max(),
            &symbols
    );
    for (auto& windowSize : windowSizes)
    {
        EXPECT_NO_THROW(gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues));
        EXPECT_NO_THROW(gabac::inverseTransformMatchCoding(pointers, lengths, rawValues, &decodedSymbols));
        EXPECT_EQ(symbols.size(), decodedSymbols.size());
        EXPECT_EQ(symbols, decodedSymbols);
    }
    symbols.clear();
}*/
