#pragma once

#include <type_traits>
#include <list>
#include <set>


namespace clinq
{
namespace detail
{
template <typename ITERATOR>
struct iterator_traits
{
	typedef typename std::remove_reference<decltype(*std::declval<ITERATOR>())>::type value_type;
};
}

template <typename ITERATOR, typename PREDICATE>
class IteratorWithFilter
{
	ITERATOR current;
	ITERATOR end;
	PREDICATE predicate;

public:
	typedef typename detail::iterator_traits<ITERATOR>::value_type value_type;
	typedef std::forward_iterator_tag iterator_category;

	IteratorWithFilter(const ITERATOR& current, const ITERATOR& end, const PREDICATE& predicate)
		: current(current),
		  end(end),
		  predicate(predicate) {
	}

	bool operator!=(const IteratorWithFilter<ITERATOR, PREDICATE>& other) const {
		return current != other.current;
	}

	value_type operator*() const {
		return *current;
	}

	const IteratorWithFilter& operator++() {
		if (!(current != end))
			return *this;

		do {
			++current;
		} while (current != end && !predicate(*current));

		return *this;
	}
};


template <typename ITERATOR, typename TRANSFORM>
class IteratorWithTransform
{
	ITERATOR current;
	TRANSFORM transform;

public:
	typedef typename std::result_of<TRANSFORM(decltype(*std::declval<ITERATOR>()))>::type value_type;
	typedef std::forward_iterator_tag iterator_category;

	IteratorWithTransform(const ITERATOR& current, TRANSFORM transform)
		: current(current),
		  transform(transform) {
	}

	bool operator!=(const IteratorWithTransform<ITERATOR, TRANSFORM>& other) const {
		return current != other.current;
	}

	value_type operator*() const {
		return transform(*current);
	}

	const IteratorWithTransform& operator++() {
		++current;
		return *this;
	}
};


template <typename ITERATOR>
class IteratorWithTake
{
	ITERATOR current;
	ITERATOR end;
	std::size_t count;

public:
	typedef typename detail::iterator_traits<ITERATOR>::value_type value_type;
	typedef std::forward_iterator_tag iterator_category;

	IteratorWithTake(const ITERATOR& current, const ITERATOR& end, std::size_t count)
		: current(current),
		  end(end),
		  count(count) {
	}

	bool operator!=(const IteratorWithTake<ITERATOR>& other) const {
		if (count == 0)
			return end != other.current;
		else
			return current != other.current;
	}

	value_type operator*() const {
		return *current;
	}

	const IteratorWithTake& operator++() {
		if (count > 1)
			++current;

		--count;

		return *this;
	}
};


template <typename ITERATOR>
class IteratorWithSkip
{
	mutable ITERATOR current;
	ITERATOR end;
	mutable std::size_t count;

public:
	typedef typename detail::iterator_traits<ITERATOR>::value_type value_type;
	typedef std::forward_iterator_tag iterator_category;

	IteratorWithSkip(const ITERATOR& current, const ITERATOR& end, std::size_t count)
		: current(current),
		  end(end),
		  count(count) {
	}

	bool operator!=(const IteratorWithSkip<ITERATOR>& other) const {
		while (count > 0 && current != end) {
			++current;
			--count;
		}

		return current != other.current;
	}

	value_type operator*() const {
		return *current;
	}

	const IteratorWithSkip& operator++() {
		++current;
		return *this;
	}
};


template <typename ITERATOR>
class Enumerable
{
	typedef typename detail::iterator_traits<ITERATOR>::value_type value_type;

	ITERATOR itBegin;
	ITERATOR itEnd;

public:
	Enumerable(const ITERATOR& begin, const ITERATOR& end)
		: itBegin(begin),
		  itEnd(end) {
	}

	ITERATOR begin() {
		return begin;
	}

	ITERATOR end() {
		return end;
	}

	template <typename PREDICATE>
	Enumerable<IteratorWithFilter<ITERATOR, PREDICATE>> where(const PREDICATE& predicate) {
		static_assert(std::is_same<typename std::result_of<PREDICATE(value_type)>::type, bool>::value, "PREDICATE must be a function: bool(value_type)");

		return Enumerable<IteratorWithFilter<ITERATOR, PREDICATE>>(
			IteratorWithFilter<ITERATOR, PREDICATE>(itBegin, itEnd, predicate),
			IteratorWithFilter<ITERATOR, PREDICATE>(itEnd, itEnd, predicate)
		);
	}

	template <typename TRANSFORM>
	Enumerable<IteratorWithTransform<ITERATOR, TRANSFORM>> select(TRANSFORM transform) {
		return Enumerable<IteratorWithTransform<ITERATOR, TRANSFORM>>(
			IteratorWithTransform<ITERATOR, TRANSFORM>(itBegin, transform),
			IteratorWithTransform<ITERATOR, TRANSFORM>(itEnd, transform)
		);
	}

	Enumerable<IteratorWithTake<ITERATOR>> take(std::size_t count) {
		return Enumerable<IteratorWithTake<ITERATOR>>(
			IteratorWithTake<ITERATOR>(itBegin, itEnd, count),
			IteratorWithTake<ITERATOR>(itEnd, itEnd, count)
		);
	}

	Enumerable<IteratorWithSkip<ITERATOR>> skip(std::size_t count) {
		return Enumerable<IteratorWithSkip<ITERATOR>>(
			IteratorWithSkip<ITERATOR>(itBegin, itEnd, count),
			IteratorWithSkip<ITERATOR>(itEnd, itEnd, count)
		);
	}

	std::list<value_type> toList() {
		std::list<value_type> result;
		to(result);
		return result;
	}

	std::set<value_type> toSet() {
		std::set<value_type> result;
		to(result);
		return result;
	}

	template <typename LIST, typename LIST_value_type = decltype(*std::declval<LIST>().begin())>
	void to(LIST& l) {
		to(std::inserter(l, l.end()));
	}

	template <typename OUTPUT_ITERATOR, typename ITERATOR_value_type = decltype(*std::declval<OUTPUT_ITERATOR>())>
	void to(OUTPUT_ITERATOR result) {
		for (ITERATOR it = itBegin; it != itEnd; ++it) {
			*result = *it;
			++result;
		}
	}

	template <typename ACTION>
	void foreach(ACTION action) {
		for (ITERATOR it = itBegin; it != itEnd; ++it)
			action(*it);
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename LIST, typename ITERATOR = decltype(std::declval<LIST>().begin())>
Enumerable<ITERATOR> from(LIST& l) {
	return Enumerable<ITERATOR>(l.begin(), l.end());
}

template <typename value_type, int N>
Enumerable<value_type*> from(value_type (&l)[N]) {
	return Enumerable<value_type*, value_type>(l, l + N);
}

template <typename value_type>
Enumerable<value_type*> from(value_type* l, std::size_t len) {
	return Enumerable<value_type*>(l, l + len);
}
}
