#ifndef GABACIFY_ANALYSIS_H_
#define GABACIFY_ANALYSIS_H_


#include <vector>

#include "gabacify/configuration.h"


namespace gabacify {


void defineConfigurations(
        const std::vector<uint64_t>& symbols,
        unsigned int wordSize,
        std::vector<Configuration> *configurations
);


}  // namespace gabacify


#endif  // GABACIFY_ANALYSIS_H_
