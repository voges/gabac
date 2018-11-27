#ifndef GABAC_CONTEXT_SELECTOR_H_
#define GABAC_CONTEXT_SELECTOR_H_


namespace gabac {


class ContextSelector
{
 public:
    ContextSelector();

    ~ContextSelector();

    static unsigned int getContextForBi(
            unsigned int contextSetIdx,
            unsigned int binIdx
    );

    static unsigned int getContextForTu(
            unsigned int contextSetIdx,
            unsigned int binIdx
    );

    static unsigned int getContextForEg(
            unsigned int contextSetIdx,
            unsigned int binIdx
    );
};


}  // namespace gabac


#endif  // GABAC_CONTEXT_SELECTOR_H_
