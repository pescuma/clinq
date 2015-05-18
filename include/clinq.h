#pragma once

#include <type_traits>
#include <list>
#include <set>

namespace clinq
{
template <typename ITERATOR, typename PREDICATE>
class IteratorWithFilter
{
	typedef typename std::remove_reference<decltype(*std::declval<ITERATOR>())>::type ITEM;

	ITERATOR current;
	ITERATOR end;
	PREDICATE predicate;

public:
	IteratorWithFilter(const ITERATOR& current, const ITERATOR& end, const PREDICATE& predicate)
		: current(current),
		  end(end),
		  predicate(predicate) {
	}

	bool operator!=(const IteratorWithFilter<ITERATOR, PREDICATE>& other) const {
		return current != other.current;
	}

	ITEM operator*() const {
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
	typedef typename std::result_of<TRANSFORM(decltype(*std::declval<ITERATOR>()))>::type ITEM;

	ITERATOR current;
	TRANSFORM transform;

public:
	IteratorWithTransform(const ITERATOR& current, TRANSFORM transform)
		: current(current),
		  transform(transform) {
	}

	bool operator!=(const IteratorWithTransform<ITERATOR, TRANSFORM>& other) const {
		return current != other.current;
	}

	ITEM operator*() const {
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
	typedef typename std::remove_reference<decltype(*std::declval<ITERATOR>())>::type ITEM;

	ITERATOR current;
	ITERATOR end;
	std::size_t count;

public:
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

	ITEM operator*() const {
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
	typedef typename std::remove_reference<decltype(*std::declval<ITERATOR>())>::type ITEM;

	mutable ITERATOR current;
	ITERATOR end;
	mutable std::size_t count;

public:
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

	ITEM operator*() const {
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
	typedef typename std::remove_reference<decltype(*std::declval<ITERATOR>())>::type ITEM;

protected:
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
		static_assert(std::is_same<typename std::result_of<PREDICATE(ITEM)>::type, bool>::value, "PREDICATE must be a function: bool(ITEM)");

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

	std::list<ITEM> toList() {
		std::list<ITEM> result;
		to(result);
		return result;
	}

	std::set<ITEM> toSet() {
		std::set<ITEM> result;
		to(result);
		return result;
	}

	template <typename LIST, typename LIST_ITEM = decltype(*std::declval<LIST>().begin())>
	void to(LIST& l) {
		to(std::inserter(l, l.end()));
	}

	template <typename OUTPUT_ITERATOR, typename ITERATOR_ITEM = decltype(*std::declval<OUTPUT_ITERATOR>())>
	void to(OUTPUT_ITERATOR result) {
		for (ITERATOR it = itBegin; it != itEnd; ++it) {
			*result = *it;
			++result;
		}
	}
};

template <typename LIST, typename ITERATOR = decltype(std::declval<LIST>().begin())>
Enumerable<ITERATOR> from(LIST& l) {
	return Enumerable<ITERATOR>(l.begin(), l.end());
}

template <typename ITEM, int N>
Enumerable<ITEM*> from(ITEM (&l)[N]) {
	return Enumerable<ITEM*, ITEM>(l, l + N);
}

template <typename ITEM>
Enumerable<ITEM*> from(ITEM* l, std::size_t len) {
	return Enumerable<ITEM*>(l, l + len);
}
}
