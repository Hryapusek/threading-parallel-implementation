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


} // namespace s0m4b0dY

#endif
