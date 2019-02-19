#ifndef PROJECT_STREAMSTEPPER_H
#define PROJECT_STREAMSTEPPER_H

namespace gabac {
struct BlockStepper {
    uint8_t* curr;
    uint8_t* end;
    uint8_t wordSize;
    bool isValid() const;
    void inc();
    uint64_t get() const;
    void set(uint64_t val) const;
};


inline bool BlockStepper::isValid() const{
    return curr != end;
}
inline void BlockStepper::inc() {
    curr += wordSize;
}
inline uint64_t BlockStepper::get() const {
    switch(wordSize) {
        case 1:
            return *curr;
        case 2:
            return *(uint16_t*) (curr);
        case 4:
            return *(uint32_t*) (curr);
        case 8:
            return *(uint64_t*) (curr);
    }
    return 0;
}

inline void BlockStepper::set(uint64_t val) const {
    switch(wordSize) {
        case 1:
            *curr = static_cast<uint8_t >(val);
            return;
        case 2:
            *(uint16_t*) (curr)= static_cast<uint16_t >(val);
            return;
        case 4:
            *(uint32_t*) (curr)= static_cast<uint32_t >(val);
            return;
        case 8:
            *(uint64_t*) (curr)= static_cast<uint64_t >(val);
            return;
    }

}
}


#endif //PROJECT_STREAMSTEPPER_H
