#define ADD_FP_IOSTREAM
#include "fixed_point.hpp"

#include <gtest/gtest.h>
//#include "google/benchmark.h"

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    emt::fixed_point<16, int> fp_16i;

    fp_16i = 25;
    fp_16i = 25.0f + 25;

    std::cout << fp_16i << "\n";
    return 0;
}
