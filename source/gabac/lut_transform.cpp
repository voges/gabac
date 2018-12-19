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

/*int gabac_transformLutTransform0(
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
}*/

// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------

namespace gabac {

const size_t MAX_LUT_SIZE = 1u << 20u; // 8MB table

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


    for (const auto& symbol : symbols)
    {
        freq[symbol]++;
        if (freq.size() >= MAX_LUT_SIZE)
        {
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

void inferLut(
        const size_t ORDER,
        const std::vector<uint64_t>& symbols,
        std::vector<std::pair<uint64_t, uint64_t>> *const lut0,
        std::vector<uint64_t> *const inverseLut0,
        std::vector<uint64_t> *const lut1,
        std::vector<uint64_t> *const inverseLut1
){
    // Clear
    lut1->clear();
    inverseLut1->clear();

    // Step 1: basic mapping for order 0. All symbols are now in a dense interval starting [0...N]
    inferLut0(symbols, lut0, inverseLut0);

    if (symbols.empty())
    {
        return;
    }

    if (ORDER == 0)
    {
        return;
    }

    size_t size = 1;
    for (size_t i = 0; i < ORDER + 1; ++i)
    {
        size *= inverseLut0->size();
    }

    if (size >= MAX_LUT_SIZE)
    {
        lut0->clear();
        return;
    }

    std::vector<std::pair<uint64_t, uint64_t>> ctr(size, {std::numeric_limits<uint64_t>::max(), 0});
    std::vector<uint64_t> lastSymbols(ORDER + 1, 0);

    for (const auto& symbol : symbols)
    {
        // Update history
        for (size_t i = ORDER; i > 0; --i)
        {
            lastSymbols[i] = lastSymbols[i - 1];
        }

        // Translate symbol into order1 symbol
        uint64_t narrowedSymbol = symbol;
        lastSymbols[0] = lut0SingleTransform(*lut0, narrowedSymbol);

        // Compute position
        size_t index = 0;
        for (size_t i = ORDER; i > 0; --i)
        {
            index *= inverseLut0->size();
            index += lastSymbols[i];
        }
        index *= inverseLut0->size();
        index += lastSymbols[0];


        // Count
        ctr[index].second++;
    }

    // Step through all single LUTs
    for (size_t i = 0; i < ctr.size(); i += inverseLut0->size())
    {
        uint64_t counter = 0;
        for (auto it = ctr.begin() + i; it != ctr.begin() + i + inverseLut0->size(); ++it)
        {
            it->first = counter;
            counter++;
        }

        // Sort single LUT for frequency
        std::sort(
                ctr.begin() + i, ctr.begin() + i + inverseLut0->size(),
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

        // Fill inverseLUT and write rank into second field
        counter = 0;
        bool placed = false;
        for (auto it = ctr.begin() + i; it != ctr.begin() + i + inverseLut0->size(); ++it)
        {
            if (it->second == 0 && !placed)
            {
                placed = true;
            }
            if (!placed)
            {
                inverseLut1->emplace_back(it->first);
            }
            else
            {
                inverseLut1->emplace_back(0);
            }
            it->second = counter;
            counter++;
        }


        // Sort single LUT for symbol value
        std::sort(
                ctr.begin() + i, ctr.begin() + i + inverseLut0->size(),
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

        // Use previously set second field to fill LUT
        for (auto it = ctr.begin() + i; it != ctr.begin() + i + inverseLut0->size(); ++it)
        {
            lut1->emplace_back(it->second);
        }
    }
}
// ----------------------------------------------------------------------------

void transformLutTransform0(
        unsigned order,
        const std::vector<uint64_t>& symbols,
        std::vector<uint64_t> *transformedSymbols,
        std::vector<uint64_t> *inverseLUT,
        std::vector<uint64_t> *inverseLUT1
){
    std::vector<std::pair<uint64_t, uint64_t>> lut;
    std::vector<uint64_t> lut1;
    inferLut(order, symbols, &lut, inverseLUT, &lut1, inverseLUT1);
    if (lut.empty())
    {
        inverseLUT->clear();
        inverseLUT1->clear();
        return;
    }
    transformLutTransform_core(order, symbols, lut, lut1, transformedSymbols);
}

// ----------------------------------------------------------------------------

void inverseTransformLutTransform0(
        unsigned order,
        const std::vector<uint64_t>& transformedSymbols,
        const std::vector<uint64_t>& inverseLUT,
        const std::vector<uint64_t>& inverseLUT1,
        std::vector<uint64_t> *symbols
){
    inverseTransformLutTransform_core(order, transformedSymbols, inverseLUT, inverseLUT1, symbols);
}

// ----------------------------------------------------------------------------

}  // namespace gabac

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
