#include "gabac/context_selector.h"

#include <cassert>

#include "gabac/context_tables.h"


namespace gabac {


ContextSelector::ContextSelector() = default;


ContextSelector::~ContextSelector() = default;


unsigned int ContextSelector::getContextForBi(
        unsigned int contextSetIdx,
        unsigned int binIdx
){
    assert(binIdx < contexttables::CONTEXT_SET_LENGTH);
    // TODO(Tom): add explanation for this assertion
    assert(contextSetIdx < 16);

    return (contexttables::OFFSET_BINARY_0
            + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH)
            + binIdx
    );
}


unsigned int ContextSelector::getContextForTu(
        unsigned int contextSetIdx,
        unsigned int binIdx
){
    assert(binIdx < contexttables::CONTEXT_SET_LENGTH);
    // TODO(Tom): add explanation for this assertion
    assert(contextSetIdx < 68);

    return (contexttables::OFFSET_TRUNCATED_UNARY_0
            + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH)
            + binIdx
    );
}


unsigned int ContextSelector::getContextForEg(
        unsigned int contextSetIdx,
        unsigned int binIdx
){
    assert(binIdx < contexttables::CONTEXT_SET_LENGTH);
    // TODO(Tom): add explanation for this assertion
    assert(contextSetIdx < 16);

    return (contexttables::OFFSET_EXPONENTIAL_GOLOMB_0
            + (contextSetIdx * contexttables::CONTEXT_SET_LENGTH)
            + binIdx
    );
}


}  // namespace gabac
