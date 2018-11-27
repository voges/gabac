#ifndef TESTS_GABAC_TEST_COMMON_H_
#define TESTS_GABAC_TEST_COMMON_H_

#include <algorithm>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

template<typename T>
inline void fillVectorRandomUniform(T min, T max, std::vector<T> *const vector){
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    auto seed = rnd_device();
    std::mt19937_64 mersenne_engine{seed};  // Generates random integers
    std::cout << "fillVectorRandomInterval: min: " << min << "; max: " << max << "; seed: " << seed << std::endl;

    std::uniform_int_distribution<T> dist(min, max);

    std::generate(
            vector->begin(), vector->end(),
            [&dist, &mersenne_engine]()
            {
                return dist(mersenne_engine);  // to keep the diff values within the 64-bit signed range
            }
    );
}

template<typename T>
inline void fillVectorRandomGeometric(std::vector<uint64_t> *const vector){
    // First create an instance of an engine.
    std::random_device rnd_device;
    // Specify the engine and distribution.
    auto seed = rnd_device();
    std::mt19937_64 mersenne_engine{seed};  // Generates random integers
    std::cout << "fillVectorRandomGeometric: type: " << sizeof(T) << "; seed: " << seed << std::endl;

    std::geometric_distribution<T> dist(0.05);

    std::generate(
            vector->begin(), vector->end(),
            [&dist, &mersenne_engine]()
            {
                return dist(mersenne_engine);  // to keep the diff values within the 64-bit signed range
            }
    );
}

#endif  // TESTS_GABAC_TEST_COMMON_H_
