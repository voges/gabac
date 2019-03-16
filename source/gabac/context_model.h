#ifndef GABAC_CONTEXT_MODEL_H_
#define GABAC_CONTEXT_MODEL_H_

#include "cabac_tables.h"

namespace gabac {


class ContextModel
{
 public:
    explicit ContextModel(
            unsigned char state
    );

    ~ContextModel();

    unsigned char getState() const{
        return m_state >> 1u;
    }

    unsigned char getMps() const{
        return m_state & static_cast<unsigned char>(0x1u);
    }

    void updateLps(){
        m_state = cabactables::nextStateLps[m_state];
    }

    void updateMps(){
        m_state = cabactables::nextStateMps[m_state];
    }

 private:
    unsigned char m_state;
};


}  // namespace gabac


#endif  // GABAC_CONTEXT_MODEL_H_
