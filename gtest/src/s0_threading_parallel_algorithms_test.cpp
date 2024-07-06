#include "gtest/gtest.h"

#include <ranges>
#include <vector>
#include <numeric>

#include "s0_parallel_algorithms_threading.hpp"

TEST(reduce, VectorOf500Elements)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    auto result = threading.reduce(arr.begin(), arr.end());
    ASSERT_EQ(result, std::reduce(arr.begin(), arr.end()));
}

TEST(reduce, VectorOf500ElementsInitValuePassed)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto result = threading.reduce(arr.begin(), arr.end(), initValue);
    ASSERT_EQ(result, initValue + std::reduce(arr.begin(), arr.end()));
}

TEST(findIf, SearchWithLambda)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    constexpr int searchValue = 420;
    auto result = threading.find_if(arr.begin(), arr.end(), [](int value){return value == searchValue;});
    ASSERT_NE(result, arr.end());
    ASSERT_EQ(*result, searchValue);
}

TEST(findIf, SearchWithLambdaNonExistValue)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto result = threading.find_if(arr.begin(), arr.end(), [](int value){return false;});
    ASSERT_EQ(result, arr.end());
}

TEST(countIf, valueLess250Test)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto searchLambda = [](int value){return value < 250;};
    auto result = threading.count_if(arr.begin(), arr.end(), searchLambda);
    ASSERT_EQ(result, std::count_if(arr.begin(), arr.end(), searchLambda));
}

TEST(countIf, alwaysFalseTest)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto searchLambda = [](int value){return false;};
    auto result = threading.count_if(arr.begin(), arr.end(), searchLambda);
    ASSERT_EQ(result, 0);
}

TEST(countIf, alwaysTrueTest)
{
    auto range = std::ranges::views::iota(0, 500);
    std::vector<int> arr(range.begin(), range.end());
    s0m4b0dY::Threading threading;
    int initValue = 200;
    auto searchLambda = [](int value){return true;};
    auto result = threading.count_if(arr.begin(), arr.end(), searchLambda);
    ASSERT_EQ(result, arr.size());
}
