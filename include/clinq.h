#pragma once

#include <type_traits>
#include <list>
#include <set>

namespace clinq
{
template <typename ITERATOR, typename PREDICATE, typename ITEM>
class FitleredIterator
{
	ITERATOR current;
	ITERATOR end;
	PREDICATE predicate;

public:
	FitleredIterator(const ITERATOR& current, const ITERATOR& end, const PREDICATE& predicate)
		: current(current),
		  end(end),
		  predicate(predicate) {
	}

	bool operator!=(const FitleredIterator<ITERATOR, PREDICATE, ITEM>& other) const {
		return current != other.current;
	}

	ITEM operator*() const {
		return *current;
	}

	const FitleredIterator& operator++() {
		if (current == end)
			return *this;

		do {
			++current;
		} while (current != end && !predicate(*current));

		return *this;
	}
};


template <typename ITERATOR, typename TRANSFORM, typename ITEM>
class TransformedIterator
{
	ITERATOR current;
	ITERATOR end;
	TRANSFORM transform;

public:
	TransformedIterator(const ITERATOR& current, const ITERATOR& end, TRANSFORM transform)
		: current(current),
		  end(end),
		  transform(transform) {
	}

	bool operator!=(const TransformedIterator<ITERATOR, TRANSFORM, ITEM>& other) const {
		return current != other.current;
	}

	ITEM operator*() const {
		return transform(*current);
	}

	const TransformedIterator& operator++() {
		++current;
		return *this;
	}
};


template <typename ITERATOR, typename ITEM>
class Enumerable
{
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
	Enumerable<FitleredIterator<ITERATOR, PREDICATE, ITEM>, ITEM> where(const PREDICATE& predicate) {
		static_assert(std::is_integral<typename std::result_of<PREDICATE(ITEM)>::type>::value, "PREDICATE must be a function: bool(ITEM)");

		return Enumerable<FitleredIterator<ITERATOR, PREDICATE, ITEM>, ITEM>(
			FitleredIterator<ITERATOR, PREDICATE, ITEM>(itBegin, itEnd, predicate),
			FitleredIterator<ITERATOR, PREDICATE, ITEM>(itEnd, itEnd, predicate)
		);
	}

	template <typename TRANSFORM, typename NEW_ITEM = typename std::result_of<TRANSFORM(ITEM)>::type>
	Enumerable<TransformedIterator<ITERATOR, TRANSFORM, NEW_ITEM>, NEW_ITEM> select(TRANSFORM transform) {
		return Enumerable<TransformedIterator<ITERATOR, TRANSFORM, NEW_ITEM>, NEW_ITEM>(
			TransformedIterator<ITERATOR, TRANSFORM, NEW_ITEM>(itBegin, itEnd, transform),
			TransformedIterator<ITERATOR, TRANSFORM, NEW_ITEM>(itEnd, itEnd, transform)
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
		to(std::inserter(l, l.begin()));
	}

	template <typename OUTPUT_ITERATOR, typename ITERATOR_ITEM = decltype(*std::declval<OUTPUT_ITERATOR>())>
	void to(OUTPUT_ITERATOR result) {
		for (ITERATOR it = itBegin; it != itEnd; ++it) {
			*result = *it;
			++result;
		}
	}
};

template <typename LIST, typename ITERATOR = decltype(std::declval<LIST>().begin()), typename ITEM = typename std::remove_reference<decltype(*std::declval<LIST>().begin())>::type>
Enumerable<ITERATOR, ITEM> from(LIST& l) {
	return Enumerable<ITERATOR, ITEM>(l.begin(), l.end());
}

template <typename ITEM, int N>
Enumerable<ITEM*, ITEM> from(ITEM (&l)[N]) {
	return Enumerable<ITEM*, ITEM>(l, l + N);
}

template <typename ITEM>
Enumerable<ITEM*, ITEM> from(ITEM* l, std::size_t len) {
	return Enumerable<ITEM*, ITEM>(l, l + len);
}
}
