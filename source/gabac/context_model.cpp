#include "gabac/context_model.h"

#include <cassert>
#include <limits>

#include "gabac/cabac_tables.h"


namespace gabac {


ContextModel::ContextModel(
        unsigned char state
)
        : m_state(state)
{
    // Nothing to do here
}


ContextModel::~ContextModel() = default;


unsigned char ContextModel::getState() const
{
    return (m_state >> static_cast<unsigned char> (1));
}


unsigned char ContextModel::getMps() const
{
    return (m_state & static_cast<unsigned char> (0x1u));
}


void ContextModel::updateLps()
{
    assert(cabactables::nextStateLps[m_state] <= std::numeric_limits<unsigned char>::max());
    m_state = static_cast<unsigned char>(cabactables::nextStateLps[m_state]);
}


void ContextModel::updateMps()
{
    assert(cabactables::nextStateLps[m_state] <= std::numeric_limits<unsigned char>::max());
    m_state = static_cast<unsigned char>(cabactables::nextStateMps[m_state]);
}


}  // namespace gabac
