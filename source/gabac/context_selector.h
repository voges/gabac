#ifndef GABAC_CONTEXT_SELECTOR_H_
#define GABAC_CONTEXT_SELECTOR_H_

#include "gabac/context_tables.h"

#include <cassert>


namespace gabac {


class ContextSelector
{
 public:
    ContextSelector() = default;

    ~ContextSelector() = default;

    static unsigned int getContextForBi(
            unsigned int contextSetIdx,
            unsigned int binIdx
    ) {
        assert(binIdx < contexttables::CONTEXT_SET_LENGTH);
        // TODO(Tom): add explanation for this assertion
        assert(contextSetIdx < 16);

        return (contexttables::OFFSET_BINARY_0
                + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH)
                + binIdx
        );
    }

    static unsigned int getContextForTu(
            unsigned int contextSetIdx,
            unsigned int binIdx
    ) {
        assert(binIdx < contexttables::CONTEXT_SET_LENGTH_TU);
        // TODO(Tom): add explanation for this assertion
        assert(contextSetIdx < 16);

        return (contexttables::OFFSET_TRUNCATED_UNARY_0
                + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH_TU)
                + binIdx
        );
    }

    static unsigned int getContextForEg(
            unsigned int contextSetIdx,
            unsigned int binIdx
    ) {
        assert(binIdx < contexttables::CONTEXT_SET_LENGTH_EG);
        // TODO(Tom): add explanation for this assertion
        assert(contextSetIdx < 16);

        return (contexttables::OFFSET_EXPONENTIAL_GOLOMB_0
                + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH_EG)
                + binIdx
        );
    }
};


}  // namespace gabac


#endif  // GABAC_CONTEXT_SELECTOR_H_
