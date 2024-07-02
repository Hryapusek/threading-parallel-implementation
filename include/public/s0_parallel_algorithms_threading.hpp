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
  public:
    template <_helpers::AddableIterator Iterator_t>
    _helpers::IteratorValueType<Iterator_t>::value_type reduce(Iterator_t begin, Iterator_t end);
  };

  template <_helpers::AddableIterator Iterator_t>
  inline _helpers::IteratorValueType<Iterator_t>::value_type Threading::reduce(Iterator_t begin, Iterator_t end)
  {
    using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
    const value_type init_value = 0;
    std::vector<std::pair<Iterator_t, Iterator_t>> ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
    std::vector<std::future<value_type>> results;
    for (auto i = 0; i < ranges.size(); ++i)
    {
      results.push_back(std::async(
        [&range=ranges[i],init_value]()
        {
          value_type result = init_value;
          for (auto it = range.first; it != range.second; it++)
          {
            result += *it;
          }
          return result;
        })
      );
    }
    auto result = init_value;
    for (std::future<value_type> &future : results)
    {
      result += future.get();
    }
    return result;
  }
} // namespace s0m4b0dY

#endif
