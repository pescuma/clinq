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
	typedef decltype(*std::declval<ITERATOR>()) value_type;
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
	typedef typename std::result_of<TRANSFORM(typename detail::iterator_traits<ITERATOR>::value_type)>::type value_type;
	typedef std::forward_iterator_tag iterator_category;

	IteratorWithTransform(const ITERATOR& current, const TRANSFORM& transform)
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


template <typename ITERATOR, typename TRANSFORM>
class IteratorWithSelectMany
{
public:
	typedef typename std::result_of<TRANSFORM(decltype(*std::declval<ITERATOR>()))>::type list_type;
	typedef decltype(std::declval<list_type>().begin()) list_iterator_type;
	typedef typename detail::iterator_traits<list_iterator_type>::value_type value_type;
	typedef std::forward_iterator_tag iterator_category;

private:
	mutable ITERATOR current;
	ITERATOR end;
	TRANSFORM transform;

	struct Inner
	{
		list_type&& list;
		list_iterator_type current;

		Inner(list_type&& list)
			: list(std::move(list)) {
			current = this->list.begin();
		}

		Inner() {
		}

		bool hasMoreData() {
			return current != list.end();
		}
	};

	mutable std::unique_ptr<Inner> inner;

	void moveNextOuter() const {
		if (!(current != end))
			return;

		do {
			inner = std::unique_ptr<Inner>(new Inner(transform(*current)));
			++current;
		} while (current != end && !inner->hasMoreData());
	}

public:
	IteratorWithSelectMany(const ITERATOR& current, const ITERATOR& end, const TRANSFORM& transform)
		: current(current),
		  end(end),
		  transform(transform) {
		list_iterator_type i;

	}

	IteratorWithSelectMany(const IteratorWithSelectMany& other)
		: current(other.current),
		  end(other.end),
		  transform(other.transform) {
	}

	IteratorWithSelectMany(IteratorWithSelectMany&& other)
		: current(std::move(other.current)),
		  end(std::move(other.end)),
		  transform(std::move(other.transform)),
		  inner(std::move(other.inner)) {
	}

	~IteratorWithSelectMany() {
	}

	bool operator!=(const IteratorWithSelectMany<ITERATOR, TRANSFORM>& other) const {
		if (inner == nullptr)
			moveNextOuter();

		if (inner->hasMoreData())
			return false;

		return current != other.current;
	}

	value_type operator*() const {
		return *inner->current;
	}

	const IteratorWithSelectMany& operator++() {
		++inner->current;

		if (!inner->hasMoreData())
			moveNextOuter();

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
	typedef typename std::remove_cv<typename std::remove_reference<typename detail::iterator_traits<ITERATOR>::value_type>::type>::type simple_value_type;

	ITERATOR itBegin;
	ITERATOR itEnd;

public:
	Enumerable(ITERATOR&& begin, ITERATOR&& end)
		: itBegin(std::move(begin)),
		  itEnd(std::move(end)) {
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
	Enumerable<IteratorWithTransform<ITERATOR, TRANSFORM>> select(const TRANSFORM& transform) {
		return Enumerable<IteratorWithTransform<ITERATOR, TRANSFORM>>(
			IteratorWithTransform<ITERATOR, TRANSFORM>(itBegin, transform),
			IteratorWithTransform<ITERATOR, TRANSFORM>(itEnd, transform)
		);
	}

	template <typename TRANSFORM>
	Enumerable<IteratorWithSelectMany<ITERATOR, TRANSFORM>> select_many(const TRANSFORM& transform) {
		return Enumerable<IteratorWithSelectMany<ITERATOR, TRANSFORM>>(
			IteratorWithSelectMany<ITERATOR, TRANSFORM>(itBegin, itEnd, transform),
			IteratorWithSelectMany<ITERATOR, TRANSFORM>(itEnd, itEnd, transform)
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

	std::list<simple_value_type> to_list() {
		std::list<simple_value_type> result;
		to(result);
		return result;
	}

	std::set<simple_value_type> to_set() {
		std::set<simple_value_type> result;
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
