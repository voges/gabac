#ifndef GABAC_TEST_COMMON_H_
#define GABAC_TEST_COMMON_H_


#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>


inline void fillVectorRandomUniform(uint64_t min, uint64_t max, gabac::DataStream *const vector){
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    auto seed = rnd_device();
    std::mt19937_64 mersenne_engine{seed};  // Generates random integers
    std::cout << "fillVectorRandomInterval: min: " << min << "; max: " << max << "; seed: " << seed << std::endl;

    std::uniform_int_distribution<uint64_t > dist(min, max);

    std::generate(
            vector->begin(), vector->end(),
            [&dist, &mersenne_engine]()
            {
                return dist(mersenne_engine);  // to keep the diff values within the 64-bit signed range
            }
    );
}


inline void fillVectorRandomGeometric(gabac::DataStream *const vector){
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    auto seed = rnd_device();
    std::mt19937_64 mersenne_engine{seed};  // Generates random integers
    std::cout << "fillVectorRandomGeometric: type: " << vector->getWordSize() << "; seed: " << seed << std::endl;

    std::geometric_distribution<uint64_t > dist(0.05);

    std::generate(
            vector->begin(), vector->end(),
            [&dist, &mersenne_engine]()
            {
                return dist(mersenne_engine);  // to keep the diff values within the 64-bit signed range
            }
    );
}


#endif  // GABAC_TEST_COMMON_H_
