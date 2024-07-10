#ifndef S0M4B0D4_PARALLEL_ALGORITHMS
#define S0M4B0D4_PARALLEL_ALGORITHMS

#include <type_traits>
#include <vector>
#include <thread>
#include <future>
#include <execution>

#include "CommonUtils/s0_type_traits.hpp"
#include "CommonUtils/s0_utils.hpp"
#include "s0_thread_pool.hpp"

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

	template <class Iterator_t, _helpers::Predicate<typename IteratorValueType<Iterator_t>::value_type> Predicate>
	Iterator_t find_if(Iterator_t begin, Iterator_t end, Predicate &&unaryFunction);

	template <class Iterator_t, _helpers::Predicate<typename IteratorValueType<Iterator_t>::value_type> Predicate>
	long long count_if(Iterator_t begin, Iterator_t end, Predicate &&unaryFunction);

	template <class InputIterator_t, class OutputIterator_t, class UnaryFunction,
	          class = std::enable_if_t<
				  std::is_assignable_v<
					  decltype(*std::declval<OutputIterator_t>()),
					  decltype(std::declval<UnaryFunction>()(
								   std::declval<typename IteratorValueType<InputIterator_t>::value_type>()
								   ))
					  >
				  >
	          >
	void transform(InputIterator_t begin, InputIterator_t end, OutputIterator_t output, UnaryFunction &&unaryFunction);

	/**
	 * @note You should not use it if OutputIterator_t is back_inserter.
	 */
	template <class InputIterator_t, class OutputIterator_t, class UnaryFunction,
	          class = std::enable_if_t<
				  std::is_assignable_v<
					  decltype(*std::declval<OutputIterator_t>()),
					  decltype(std::declval<UnaryFunction>()(
								   std::declval<typename IteratorValueType<InputIterator_t>::value_type>()
								   ))
					  >
				  >
	          >
	void transform_non_back_inserter(InputIterator_t begin, InputIterator_t end, OutputIterator_t output, UnaryFunction &&unaryFunction);

	template <class InputIterator1_t, class InputIterator2_t, class OutputIterator_t, class BinaryFunction,
	          class = std::enable_if_t<
				  std::is_assignable_v<
					  decltype(*std::declval<OutputIterator_t>()),
					  decltype(std::declval<BinaryFunction>()(
								   std::declval<typename IteratorValueType<InputIterator1_t>::value_type>(),
								   std::declval<typename IteratorValueType<InputIterator2_t>::value_type>()
								   ))
					  >
				  >
	          >
	void transform(InputIterator1_t begin1, InputIterator1_t end1, InputIterator2_t begin2, OutputIterator_t output, BinaryFunction &&binaryFunction);

	/**
	 * @note You should not use it if OutputIterator_t is back_inserter.
	 */
	template <class InputIterator1_t, class InputIterator2_t, class OutputIterator_t, class BinaryFunction,
	          class = std::enable_if_t<
				  std::is_assignable_v<
					  decltype(*std::declval<OutputIterator_t>()),
					  decltype(std::declval<BinaryFunction>()(
								   std::declval<typename IteratorValueType<InputIterator1_t>::value_type>(),
								   std::declval<typename IteratorValueType<InputIterator2_t>::value_type>()
								   ))
					  >
				  >
	          >
	void transform_non_back_inserter(InputIterator1_t begin1, InputIterator1_t end1, InputIterator2_t begin2, OutputIterator_t output, BinaryFunction &&binaryFunction);

	template < std::forward_iterator InputIterator_t, class HashFunction = std::hash<::_helpers::IteratorValueType_t<InputIterator_t> >, class Comparator = std::less<::_helpers::IteratorValueType_t<InputIterator_t> > >
	void bitonic_sort(InputIterator_t begin, InputIterator_t end, HashFunction hashFunction = HashFunction(), Comparator comparator = Comparator());

	template < std::forward_iterator InputIterator_t, class HashFunction = std::hash<::_helpers::IteratorValueType_t<InputIterator_t> >, class Comparator = std::less<::_helpers::IteratorValueType_t<InputIterator_t> > >
	void odd_even_sort(InputIterator_t begin, InputIterator_t end, HashFunction hashFunction = HashFunction(), Comparator comparator = Comparator());

private:
	template<class Hash_t, class Value_t, class Comparator>
	void bitonic_merge(std::vector<Hash_t>& hashValues, std::unordered_map<Hash_t, Value_t*>& hashTable, std::vector<Hash_t>::size_type low, std::vector<Hash_t>::size_type cnt, Comparator comparator, ThreadPool& pool);

};

template <_helpers::AddableIterator Iterator_t>
inline _helpers::IteratorValueType<Iterator_t>::value_type Threading::reduce(Iterator_t begin, Iterator_t end)
{
	using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
	std::vector<std::pair<Iterator_t, Iterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<std::optional<value_type> > > results;
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
	for (std::future<std::optional<value_type> > &future : results)
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
	std::vector<std::pair<Iterator_t, Iterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<std::optional<value_type> > > results;
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
	for (std::future<std::optional<value_type> > &future : results)
	{
		std::optional<value_type> value = future.get();
		if(value.has_value())
			result += std::move(value).value();
	}
	return result;
}

template <class Iterator_t, _helpers::Predicate<typename _helpers::IteratorValueType<Iterator_t>::value_type> Predicate>
inline Iterator_t Threading::find_if(Iterator_t begin, Iterator_t end, Predicate &&unaryFunction)
{
	using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
	std::vector<std::pair<Iterator_t, Iterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<std::optional<Iterator_t> > > results;
	std::atomic_bool found = false;
	for (auto i = 0; i < ranges.size(); ++i)
	{
		const auto &range = ranges[i];
		results.push_back(std::async(
							  [&range=range, &found, unaryFunction]() -> std::optional<Iterator_t>
			{
				for (auto it = range.first; it != range.second; it++)
				{
					if (found)
						break;
					if (unaryFunction(*it))
					{
						found = true;
						return it;
					}
				}
				return std::nullopt;
			}
							  ));
	}
	for (std::future<std::optional<Iterator_t> > &localResult : results)
	{
		auto optional = localResult.get();
		if (optional.has_value())
			return std::move(optional).value();
	}
	return end;

}
template <class Iterator_t, _helpers::Predicate<typename _helpers::IteratorValueType<Iterator_t>::value_type> Predicate>
inline long long Threading::count_if(Iterator_t begin, Iterator_t end, Predicate &&unaryFunction)
{
	using Count_t = long long;
	using value_type = _helpers::IteratorValueType<Iterator_t>::value_type;
	std::vector<std::pair<Iterator_t, Iterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<Count_t> > results;
	for (auto i = 0; i < ranges.size(); ++i)
	{
		const auto &range = ranges[i];
		results.push_back(std::async(
							  [&range=range, unaryFunction]()
			{
				Count_t count = 0;
				for (auto it = range.first; it != range.second; it++)
				{
					if (unaryFunction(*it))
						count++;
				}
				return count;
			}
							  ));
	}
	Count_t count = 0;
	for (auto &localResult : results)
		count += localResult.get();
	return count;
}

template <class InputIterator_t, class OutputIterator_t, class UnaryFunction, class>
inline void Threading::transform(InputIterator_t begin, InputIterator_t end, OutputIterator_t output, UnaryFunction &&unaryFunction)
{
	using InputValue_t = ::_helpers::IteratorValueType<InputIterator_t>::value_type;
	using UnaryFunctionReturn_t = std::invoke_result_t<UnaryFunction, InputValue_t>;
	std::vector<std::pair<InputIterator_t, InputIterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<std::vector<UnaryFunctionReturn_t> > > results;
	try
	{

		for (auto i = 0; i < ranges.size(); ++i)
		{
			results.push_back(std::async([&range = ranges[i], unaryFunction]()
				{
					std::vector<UnaryFunctionReturn_t> localResult;
					for (auto it = range.first; it != range.second; it++)
					{
						localResult.push_back(unaryFunction(*it));
					}
					return localResult;
				}));
		}
	}
	catch(const std::exception &e)
	{
		throw;
	}
	for (auto &future : results)
	{
		auto localResult = future.get();
		for (auto &value : localResult)
		{
			if constexpr (std::is_move_assignable_v<UnaryFunctionReturn_t>)
			{
				*output++ = std::move(value);
			}
			else
			{
				*output++ = value;
			}
		}
	}
}

template <class InputIterator_t, class OutputIterator_t, class UnaryFunction, class>
inline void Threading::transform_non_back_inserter(InputIterator_t begin, InputIterator_t end, OutputIterator_t output, UnaryFunction &&unaryFunction)
{
	std::vector<std::pair<InputIterator_t, InputIterator_t> > ranges = generateRanges(begin, end, std::thread::hardware_concurrency());
	std::vector<std::future<void> > tasks;
	try
	{
		for (auto i = 0; i < ranges.size(); ++i)
		{
			tasks.push_back(std::async([&range = ranges[i], unaryFunction, begin, output](){
					auto localOutput = output;
					std::advance(localOutput, range.first - begin);
					for (auto it = range.first; it != range.second; it++)
					{
						*localOutput++ = unaryFunction(*it);
					}
				}));
		}
	}
	catch(const std::exception &e)
	{
		throw;
	}
	for (auto &future : tasks)
	{
		future.get();
	}
}

template <class InputIterator1_t, class InputIterator2_t, class OutputIterator_t, class BinaryFunction, class>
inline void Threading::transform(InputIterator1_t begin1, InputIterator1_t end1, InputIterator2_t begin2, OutputIterator_t output, BinaryFunction &&binaryFunction)
{
	using InputValue1_t = ::_helpers::IteratorValueType<InputIterator1_t>::value_type;
	using InputValue2_t = ::_helpers::IteratorValueType<InputIterator2_t>::value_type;
	using BinaryFunctionReturn_t = std::invoke_result_t<BinaryFunction, InputValue1_t, InputValue2_t>;
	std::vector<std::pair<InputIterator1_t, InputIterator1_t> > ranges = generateRanges(begin1, end1, std::thread::hardware_concurrency());
	std::vector<std::future<std::vector<BinaryFunctionReturn_t> > > results;
	try
	{
		for (auto i = 0; i < ranges.size(); ++i)
		{
			results.push_back(std::async([&range = ranges[i], begin1, begin2, binaryFunction](){
					std::vector<BinaryFunctionReturn_t> localResult;
					auto localBegin2 = begin2;
					std::advance(localBegin2, std::distance(range.first, begin1));
					for (auto it = range.first; it != range.second; it++, localBegin2++)
					{
						localResult.push_back(binaryFunction(*it, *localBegin2));
					}
					return localResult;
				}));
		}
	}
	catch(const std::exception &e)
	{
		throw;
	}
	for (auto &future : results)
	{
		auto localResult = future.get();
		for (auto &value : localResult)
		{
			if constexpr (std::is_move_assignable_v<BinaryFunctionReturn_t>)
			{
				*output++ = std::move(value);
			}
			else
			{
				*output++ = value;
			}
		}
	}
}

template <class InputIterator1_t, class InputIterator2_t, class OutputIterator_t, class BinaryFunction, class>
inline void Threading::transform_non_back_inserter(InputIterator1_t begin1, InputIterator1_t end1, InputIterator2_t begin2, OutputIterator_t output, BinaryFunction &&binaryFunction)
{
	std::vector<std::pair<InputIterator1_t, InputIterator1_t> > ranges = generateRanges(begin1, end1, std::thread::hardware_concurrency());
	std::vector<std::future<void> > tasks;
	try
	{
		for (auto i = 0; i < ranges.size(); ++i)
		{
			tasks.push_back(std::async([&range = ranges[i], begin1, begin2, output, binaryFunction](){
					auto localOutput = output;
					auto localBegin2 = begin2;
					std::advance(localBegin2, range.first - begin1);
					std::advance(localOutput, range.first - begin1);
					for (auto it = range.first; it != range.second; it++, localBegin2++)
					{
						*localOutput++ = binaryFunction(*it, *localBegin2);
					}
				}));
		}
	}
	catch(const std::exception &e)
	{
		throw;
	}
}

template<std::forward_iterator InputIterator_t, class HashFunction, class Comparator>
inline void Threading::bitonic_sort(InputIterator_t begin, InputIterator_t end, HashFunction hashFunction, Comparator comparator)
{
	using value_type = ::_helpers::IteratorValueType_t<InputIterator_t>;
	using hash_t = std::invoke_result_t<HashFunction, value_type>;

	auto length = std::distance(begin, end);
	assert(("Array length must be a power of 2", ((length - 1) & length) == 0));

	auto [hashValues, hashTable] = hash_sequence< InputIterator_t, hash_t, value_type >(begin, end, hashFunction, comparator);

	// Sort first half in ascending order
	std::sort(std::execution::par_unseq, hashValues.begin(), hashValues.begin() + length,
	          [&hashTable, &comparator](hash_t lhs, hash_t rhs)
		{
			return comparator(*hashTable[lhs], *hashTable[rhs]);
		});

	// Sort second half in descending order
	std::sort(std::execution::par_unseq, hashValues.begin() + length, hashValues.end(),
	          [&hashTable, &comparator](hash_t lhs, hash_t rhs)
		{
			return not comparator(*hashTable[lhs], *hashTable[rhs]);
		});

	{
		ThreadPool pool(12);
		bitonic_merge(hashValues, hashTable, 0, hashValues.size(), comparator, pool);
	}

	placeElementsInCorrectPositions(begin, end, hashFunction, hashValues, hashTable);
}

template<class Hash_t, class Value_t, class Comparator>
inline void Threading::bitonic_merge(std::vector<Hash_t>& hashValues,
                                     std::unordered_map<Hash_t, Value_t*>& hashTable,
                                     std::vector<Hash_t>::size_type low,
                                     std::vector<Hash_t>::size_type cnt,
                                     Comparator comparator,
                                     ThreadPool& pool)
{
    if (cnt <= 1)
        return;

    auto k = cnt / 2;
    auto inplaceComparator = createInplaceComparator(comparator, hashTable);

    size_t numThreads = std::thread::hardware_concurrency();
    size_t chunkSize = (k + numThreads - 1) / numThreads;

    std::vector<std::future<void>> results;
    results.reserve(numThreads);

    for (size_t chunkStart = low; chunkStart < low + k; chunkStart += chunkSize)
    {
        results.push_back(pool.submit([chunkStart, chunkSize, &hashValues, k, &inplaceComparator, low]()
        {
            for (size_t i = chunkStart; i < std::min(chunkStart + chunkSize, low + k); ++i)
            {
                inplaceComparator(hashValues[i], hashValues[i + k]);
            }
        }));
    }

    for (auto& result : results)
    {
        result.get();
    }

    bitonic_merge(hashValues, hashTable, low, k, comparator, pool);
    bitonic_merge(hashValues, hashTable, low + k, k, comparator, pool);
}

template <std::forward_iterator InputIterator_t, class HashFunction, class Comparator>
inline void Threading::odd_even_sort(InputIterator_t begin, InputIterator_t end,
                                     HashFunction hashFunction,
                                     Comparator comparator)
{
    using value_type = ::_helpers::IteratorValueType_t<InputIterator_t>;
    using hash_t = std::invoke_result_t<HashFunction, value_type>;

    if (begin == end)
        return;

    auto [hashValues, hashTable] = hash_sequence< InputIterator_t, hash_t, value_type >(begin, end, hashFunction, comparator);

    using size_type = decltype(hashValues.size());
    size_type swapCount = 0;
    auto inplaceComparator = createInplaceComparator(comparator, hashTable);

    ThreadPool pool(std::thread::hardware_concurrency());
    size_t numThreads = std::thread::hardware_concurrency();
    size_t chunkSize = (hashValues.size() + numThreads - 1) / numThreads;

    do
    {
        swapCount = 0;
        std::vector<std::future<size_type>> results;

        for (size_t chunkStart = 1; chunkStart < hashValues.size(); chunkStart += 2 * chunkSize)
        {
            results.push_back(pool.submit([chunkStart, chunkSize, &hashValues, &inplaceComparator]()
            {
                size_type localSwapCount = 0;
                for (size_t i = chunkStart; i < std::min(chunkStart + 2*chunkSize, hashValues.size()); i += 2)
                {
                    if (i < hashValues.size() && inplaceComparator(hashValues[i - 1], hashValues[i]))
                    {
                        localSwapCount++;
                    }
                }
                return localSwapCount;
            }));
        }

        for (auto &result : results)
            swapCount += result.get();

        results.clear();

        for (size_t chunkStart = 2; chunkStart < hashValues.size(); chunkStart += 2 * chunkSize)
        {
            results.push_back(pool.submit([chunkStart, chunkSize, &hashValues, &inplaceComparator]()
            {
                size_type localSwapCount = 0;
                for (size_t i = chunkStart; i < std::min(chunkStart + 2*chunkSize, hashValues.size()); i += 2)
                {
                    if (i < hashValues.size() && inplaceComparator(hashValues[i - 1], hashValues[i]))
                    {
                        localSwapCount++;
                    }
                }
                return localSwapCount;
            }));
        }

        for (auto &result : results)
            swapCount += result.get();

        results.clear();
    } while (swapCount > 0);

    placeElementsInCorrectPositions(begin, end, hashFunction, hashValues, hashTable);
}

} // namespace s0m4b0dY

#endif
