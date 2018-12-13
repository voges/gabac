#include "gabac/lut_transform.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>

#include "gabac/return_codes.h"


// ----------------------------------------------------------------------------
// C wrapper BEGIN
// ----------------------------------------------------------------------------

int gabac_transformLutTransform0(
        const uint64_t *const symbols,
        const size_t symbolsSize,
        uint64_t **const transformedSymbols,
        uint64_t **const inverseLUT,
        size_t *const inverseLUTSize
){
    if (symbols == nullptr ||
        transformedSymbols == nullptr ||
        inverseLUT == nullptr ||
        inverseLUTSize == nullptr)
    {
        return GABAC_FAILURE;
    }

    std::vector<uint64_t> symbolsVecCpp(symbols, symbols + symbolsSize);
    std::vector<uint64_t> transformedVec;
    std::vector<uint64_t> invLutVec;

    gabac::transformLutTransform0(symbolsVecCpp, &transformedVec, &invLutVec);

    (*transformedSymbols) = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * transformedVec.size()));
    (*inverseLUT) = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * invLutVec.size()));

    std::copy(transformedVec.begin(), transformedVec.end(), *transformedSymbols);
    std::copy(invLutVec.begin(), invLutVec.end(), *inverseLUT);

    *inverseLUTSize = invLutVec.size();

    return GABAC_SUCCESS;
}

// ----------------------------------------------------------------------------

int gabac_inverseTransformLutTransform0(
        const uint64_t *transformedSymbols,
        size_t transformedSymbolsSize,
        const uint64_t *inverseLUT,
        size_t inverseLUTSize,
        uint64_t **symbols
){
    if (transformedSymbols == nullptr ||
        inverseLUT == nullptr ||
        symbols == nullptr)
    {
        return GABAC_FAILURE;
    }

    std::vector<uint64_t> transSymVec(transformedSymbols, transformedSymbols + transformedSymbolsSize);
    std::vector<uint64_t> invLutVec(inverseLUT, inverseLUT + inverseLUTSize);
    std::vector<uint64_t> symVec;

    gabac::inverseTransformLutTransform0(transSymVec, invLutVec, &symVec);

    (*symbols) = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * symVec.size()));

    std::copy(symVec.begin(), symVec.end(), *symbols);

    return GABAC_SUCCESS;
}

// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------

namespace gabac {

static void inferLut0(
        const std::vector<uint64_t>& symbols,
        std::vector<std::pair<uint64_t, uint64_t>> *const lut,
        std::vector<uint64_t> *const inverseLut
){
    // Clear
    lut->clear();
    inverseLut->clear();
    if (symbols.empty())
    {
        return;
    }

    std::unordered_map<uint64_t, uint64_t> freq;


    const size_t MAX_LUT_SIZE = 1u << 20u; // 8MB table
    for (const auto& symbol : symbols)
    {
        freq[symbol]++;
        if(freq.size() >= MAX_LUT_SIZE) {
            return;
        }
    }

    std::vector<std::pair<uint64_t, uint64_t>> freqVec;
    std::copy(freq.begin(), freq.end(), std::back_inserter(freqVec));

    // Sort symbol frequencies in descending order
    std::sort(
            freqVec.begin(), freqVec.end(),
            [](const std::pair<uint64_t, uint64_t>& a,
               const std::pair<uint64_t, uint64_t>& b
            )
            {
                if (a.second > b.second)
                {
                    return true;
                }
                if (a.second < b.second)
                {
                    return false;
                }
                return a.first < b.first;
            }
    );

    for (const auto& symbol : freqVec)
    {
        lut->emplace_back(symbol.first, inverseLut->size());
        inverseLut->emplace_back(symbol.first);
    }

    // Sort symbols
    std::sort(
            lut->begin(), lut->end(),
            [](const std::pair<uint64_t, uint64_t>& a,
               const std::pair<uint64_t, uint64_t>& b
            )
            {
                if (a.first < b.first)
                {
                    return true;
                }
                if (a.first > b.first)
                {
                    return false;
                }
                return a.second > b.second;
            }
    );
}

// ----------------------------------------------------------------------------

static uint64_t lut0SingleTransform(
        const std::vector<std::pair<uint64_t, uint64_t>>& lut0,
        uint64_t symbol
){
    auto it = std::lower_bound(
            lut0.begin(), lut0.end(), std::make_pair(symbol, uint(0)),
            [](const std::pair<uint64_t, uint64_t>& a,
               const std::pair<uint64_t, uint64_t>& b
            )
            {
                return a.first < b.first;
            }
    );
    assert(it != lut0.end());
    assert(it->first == symbol);
    return it->second;
}

// ----------------------------------------------------------------------------

static void transformLutTransform_core(
        const size_t ORDER,
        const std::vector<uint64_t>& symbols,
        const std::vector<std::pair<uint64_t, uint64_t>>& lut0,
        const std::vector<uint64_t>& lut,
        std::vector<uint64_t> *const transformedSymbols
){
    assert(transformedSymbols != nullptr);

    // Prepare the output vector
    transformedSymbols->clear();

    if (symbols.empty())
    {
        return;
    }

    std::vector<uint64_t> lastSymbols(ORDER + 1, 0);

    // Do the LUT transform
    for (const auto& symbol : symbols)
    {
        // Update history
        for (size_t i = ORDER; i > 0; --i)
        {
            lastSymbols[i] = lastSymbols[i - 1];
        }
        lastSymbols[0] = lut0SingleTransform(lut0, symbol);


        // Compute position
        size_t index = 0;
        for (size_t i = ORDER; i > 0; --i)
        {
            index *= lut0.size();
            index += lastSymbols[i];
        }
        index *= lut0.size();
        index += lastSymbols[0];

        // Transform
        uint64_t transformed = lastSymbols[0];
        if (ORDER > 0)
        {
            transformed = lut[index];
        }
        transformedSymbols->emplace_back(transformed);
    }
}

// ----------------------------------------------------------------------------

static void inverseTransformLutTransform_core(
        const size_t ORDER,
        const std::vector<uint64_t>& transformedSymbols,
        const std::vector<uint64_t>& inverseLut0,
        const std::vector<uint64_t>& inverseLut,
        std::vector<uint64_t> *const symbols
){
    assert(symbols != nullptr);

    // Prepare the output vector
    symbols->clear();

    if (transformedSymbols.empty())
    {
        return;
    }

    std::vector<uint64_t> lastSymbols(ORDER + 1, 0);

    // Do the LUT transform
    for (const auto& symbol : transformedSymbols)
    {
        // Update history
        for (size_t i = ORDER; i > 0; --i)
        {
            lastSymbols[i] = lastSymbols[i - 1];
        }
        lastSymbols[0] = static_cast<uint64_t>(symbol);

        if (ORDER == 0)
        {
            symbols->emplace_back(inverseLut0[lastSymbols[0]]);
            continue;
        }

        // Compute position
        size_t index = 0;
        for (size_t i = ORDER; i > 0; --i)
        {
            index *= inverseLut0.size();
            index += lastSymbols[i];
        }
        index *= inverseLut0.size();
        index += lastSymbols[0];

        // Transform
        uint64_t unTransformed = inverseLut[index];
        lastSymbols[0] = unTransformed;
        symbols->emplace_back(inverseLut0[unTransformed]);
    }
}

// ----------------------------------------------------------------------------

void transformLutTransform0(
        const std::vector<uint64_t>& symbols,
        std::vector<uint64_t> *transformedSymbols,
        std::vector<uint64_t> *inverseLUT
){
    std::vector<std::pair<uint64_t, uint64_t>> lut;
    inferLut0(symbols, &lut, inverseLUT);
    transformLutTransform_core(0, symbols, lut, std::vector<uint64_t>(), transformedSymbols);
}

// ----------------------------------------------------------------------------

void inverseTransformLutTransform0(
        const std::vector<uint64_t>& transformedSymbols,
        const std::vector<uint64_t>& inverseLUT,
        std::vector<uint64_t> *symbols
){
    inverseTransformLutTransform_core(0, transformedSymbols, inverseLUT, std::vector<uint64_t>(), symbols);
}

// ----------------------------------------------------------------------------

}  // namespace gabac

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
