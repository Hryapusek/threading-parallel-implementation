#include "gtest/gtest.h"

#include <ranges>
#include <vector>
#include <numeric>

#include "s0_parallel_algorithms_threading.hpp"

TEST(reduceParallelAlgorithm, VectorOf500Elements)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    auto result = threading.reduce(arr.begin(), arr.end());
    ASSERT_EQ(result, std::reduce(arr.begin(), arr.end()));
}

TEST(reduceParallelAlgorithm, VectorOf500ElementsInitValuePassed)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto result = threading.reduce(arr.begin(), arr.end(), initValue);
    ASSERT_EQ(result, initValue + std::reduce(arr.begin(), arr.end()));
}

TEST(findIfParallelAlgorithm, SearchWithLambda)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading openMPI;
    int initValue = 200;
    constexpr int searchValue = 420;
    auto result = openMPI.find_if(arr.begin(), arr.end(), [](int value){return value == searchValue;});
    ASSERT_NE(result, arr.end());
    ASSERT_EQ(*result, searchValue);
}

TEST(findIfParallelAlgorithm, SearchWithLambdaNonExistValue)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading openMPI;
    int initValue = 200;
    auto result = openMPI.find_if(arr.begin(), arr.end(), [](int value){return false;});
    ASSERT_EQ(result, arr.end());
}
