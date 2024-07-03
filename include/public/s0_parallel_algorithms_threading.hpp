#ifndef S0M4B0D4_PARALLEL_ALGORITHMS
#define S0M4B0D4_PARALLEL_ALGORITHMS

#include <type_traits>
#include <vector>
#include <thread>
#include <future>

#include "CommonUtils/s0_type_traits.hpp"
#include "CommonUtils/s0_utils.hpp"

namespace s0m4b0dY
{
  class Threading
  {
    template < class T >
    using IteratorValueType = _helpers::IteratorValueType<T>;
  public:
    template <_helpers::AddableIterator Iterator_t>
    IteratorValueType<Iterator_t>::value_type reduce(Iterator_t begin, Iterator_t end);

    template <_helpers::AddableIterator Iterator_t>
    IteratorValueType<Iterator_t>::value_type reduce(Iterator_t begin, Iterator_t end, IteratorValueType<Iterator_t>::value_type initValue);
  };

  template <_helpers::AddableIterator Iterator_t>
  inline _helpers::IteratorValueType<Iterator_t>::value_type Threading::reduce(Iterator_t begin, Iterator_t end)
  {
    using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
    std::vector<std::pair<Iterator_t, Iterator_t>> ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
    std::vector<std::future<std::optional<value_type>>> results;
    for (auto i = 0; i < ranges.size(); ++i)
    {
      results.push_back(std::async(
        [&range=ranges[i]]() -> std::optional<value_type>
        {
          auto it = range.first;
          if (it == range.second)
          {
            return std::nullopt;
          }
          value_type result = *it++;
          for (; it != range.second; it++)
          {
            result += *it;
          }
          return result;
        }
      ));
    }
    std::optional<value_type> result;
    for (std::future<std::optional<value_type>> &future : results)
    {
      std::optional<value_type> value = future.get();
      if (value.has_value())
      {
        if (result.has_value())
          *result += std::move(value).value();
        else
          result = std::move(value).value();
      }
    }
    if (not result.has_value())
      throw std::logic_error("Zero values in reduce found");
    return *result;
  }

  template <_helpers::AddableIterator Iterator_t>
  inline _helpers::IteratorValueType<Iterator_t>::value_type Threading::reduce(Iterator_t begin, Iterator_t end, IteratorValueType<Iterator_t>::value_type initValue)
  {
    using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
    std::vector<std::pair<Iterator_t, Iterator_t>> ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
    std::vector<std::future<std::optional<value_type>>> results;
    for (auto i = 0; i < ranges.size(); ++i)
    {
      results.push_back(std::async(
        [&range=ranges[i]]() -> std::optional<value_type>
        {
          auto it = range.first;
          if (it == range.second)
          {
            return std::nullopt;
          }
          value_type result = *it++;
          for (; it != range.second; it++)
          {
            result += *it;
          }
          return result;
        }
      ));
    }
    auto result = initValue;
    for (std::future<std::optional<value_type>> &future : results)
    {
      std::optional<value_type> value = future.get();
      if(value.has_value())
        result += std::move(value).value();
    }
    return result;
  }
} // namespace s0m4b0dY

#endif
