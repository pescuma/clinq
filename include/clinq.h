#pragma once

#include <type_traits>
#include <list>
#include <set>
#include <utility>


namespace clinq
{
namespace detail
{
template <typename ENUMERATOR>
struct iterator_traits
{
	typedef decltype(*std::declval<ENUMERATOR>()) value_type;
};
}

template <typename ITERATOR>
class Enumerator
{
	bool first;
	ITERATOR current;
	ITERATOR end;

	Enumerator(const Enumerator& other);
	Enumerator& operator=(const Enumerator& other);

public:

	typedef typename detail::iterator_traits<ITERATOR>::value_type value_type;

	Enumerator(ITERATOR&& current, ITERATOR&& end)
		: current(std::move(current)),
		  end(std::move(end)) {
		first = true;
	}

	Enumerator(Enumerator&& other)
		: first(other.first),
		  current(std::move(other.current)),
		  end(std::move(other.end)) {
	}

	bool next() {
		if (first)
			first = false;
		else
			++current;

		return current != end;
	}

	value_type get() {
		return *current;
	}
};

template <typename ENUMERATOR, typename PREDICATE>
class EnumeratorWithFilter
{
	ENUMERATOR inner;
	PREDICATE predicate;

	EnumeratorWithFilter(const EnumeratorWithFilter& other);
	EnumeratorWithFilter& operator=(const EnumeratorWithFilter& other);

public:

	typedef typename ENUMERATOR::value_type value_type;

	EnumeratorWithFilter(ENUMERATOR&& inner, PREDICATE&& predicate)
		: inner(std::move(inner)),
		  predicate(std::move(predicate)) {
	}

	EnumeratorWithFilter(EnumeratorWithFilter&& other)
		: inner(std::move(other.inner)),
		  predicate(std::move(other.predicate)) {
	}

	bool next() {
		while (inner.next()) {
			if (predicate(inner.get()))
				return true;
		}

		return false;
	}

	value_type get() {
		return inner.get();
	}
};


template <typename ENUMERATOR, typename TRANSFORM>
class EnumeratorWithTransform
{
	ENUMERATOR inner;
	TRANSFORM transform;

	EnumeratorWithTransform(const EnumeratorWithTransform& other);
	EnumeratorWithTransform& operator=(const EnumeratorWithTransform& other);

public:

	typedef typename std::result_of<TRANSFORM(typename ENUMERATOR::value_type)>::type value_type;

	EnumeratorWithTransform(ENUMERATOR&& inner, TRANSFORM&& transform)
		: inner(std::move(inner)),
		  transform(std::move(transform)) {
	}

	EnumeratorWithTransform(EnumeratorWithTransform&& other)
		: inner(std::move(other.inner)),
		  transform(std::move(other.transform)) {
	}


	bool next() {
		return inner.next();
	}

	value_type get() {
		return transform(inner.get());
	}
};


template <typename ENUMERATOR, typename TRANSFORM>
class EnumeratorWithSelectMany
{
	ENUMERATOR inner;
	TRANSFORM transform;

public:

	typedef typename std::result_of<TRANSFORM(typename ENUMERATOR::value_type)>::type list_type;
	typedef decltype(std::declval<list_type>().begin()) list_iterator_type;
	typedef typename detail::iterator_traits<list_iterator_type>::value_type value_type;

private:

	struct SubList
	{
		list_type list;
		Enumerator<list_iterator_type> enumerator;

		SubList(list_type&& list)
			: list(std::move(list)),
			  enumerator(this->list.begin(), this->list.end()) {
		}

		SubList() {
		}
	};

	std::unique_ptr<SubList> sub;

	EnumeratorWithSelectMany(const EnumeratorWithSelectMany& other);
	EnumeratorWithSelectMany& operator=(const EnumeratorWithSelectMany& other);

public:

	EnumeratorWithSelectMany(ENUMERATOR&& inner, TRANSFORM&& transform)
		: inner(std::move(inner)),
		  transform(std::move(transform)) {
	}

	EnumeratorWithSelectMany(EnumeratorWithSelectMany&& other)
		: inner(std::move(other.inner)),
		  transform(std::move(other.transform)) {
		std::swap(sub, other.sub);
	}

	bool next() {
		if (sub != nullptr && sub->enumerator.next())
			return true;

		while (inner.next()) {
			sub = std::unique_ptr<SubList>(new SubList(transform(inner.get())));
			if (sub->enumerator.next())
				return true;
		}

		return false;
	}

	value_type get() {
		return sub->enumerator.get();
	}
};


template <typename ENUMERATOR>
class EnumeratorWithTake
{
	ENUMERATOR inner;
	std::size_t count;

	EnumeratorWithTake(const EnumeratorWithTake& other);
	EnumeratorWithTake& operator=(const EnumeratorWithTake& other);

public:

	typedef typename ENUMERATOR::value_type value_type;

	EnumeratorWithTake(ENUMERATOR&& inner, std::size_t count)
		: inner(std::move(inner)),
		  count(count) {
	}

	EnumeratorWithTake(EnumeratorWithTake&& other)
		: inner(std::move(other.inner)),
		  count(other.count) {
	}

	bool next() {
		if (count < 1)
			return false;

		--count;
		return inner.next();
	}

	value_type get() {
		return inner.get();
	}
};


template <typename ENUMERATOR>
class EnumeratorWithSkip
{
	ENUMERATOR inner;
	std::size_t count;

	EnumeratorWithSkip(const EnumeratorWithSkip& other);
	EnumeratorWithSkip& operator=(const EnumeratorWithSkip& other);

public:

	typedef typename ENUMERATOR::value_type value_type;

	EnumeratorWithSkip(ENUMERATOR&& inner, std::size_t count)
		: inner(std::move(inner)),
		  count(count) {
	}

	EnumeratorWithSkip(EnumeratorWithSkip&& other)
		: inner(std::move(other.inner)),
		  count(other.count) {
	}

	bool next() {
		while (count > 0) {
			if (!inner.next())
				return false;

			--count;
		}

		return inner.next();
	}

	value_type get() {
		return inner.get();
	}
};


template <typename ENUMERATOR>
class Query
{
	ENUMERATOR enumerator;

	Query(const Query& other);
	Query& operator=(const Query& other);

public:

	typedef typename ENUMERATOR::value_type value_type;
	typedef typename std::remove_cv<typename std::remove_reference<value_type>::type>::type simple_value_type;

	explicit Query(ENUMERATOR&& enumerator)
		: enumerator(std::move(enumerator)) {
	}

	Query(Query&& other)
		: enumerator(std::move(other.enumerator)) {
	}

	class iterator
	{
		ENUMERATOR* enumerator;
		mutable bool end;
		mutable bool first;

	public:

		typedef typename ENUMERATOR::value_type value_type;
		typedef std::forward_iterator_tag iterator_category;

		iterator(ENUMERATOR* enumerator, bool end)
			: enumerator(enumerator),
			  end(end),
			  first(true) {
		}

		bool operator!=(const iterator& other) const {
			if (first && !end) {
				first = false;
				end = !enumerator->next();
			}

			return enumerator != other.enumerator || end != other.end;
		}

		value_type operator*() const {
			return enumerator->get();
		}

		const iterator& operator++() {
			end = !enumerator->next();
			return *this;
		}
	};

	iterator begin() {
		return iterator(&enumerator, false);
	}

	iterator end() {
		return iterator(&enumerator, true);
	}

	template <typename PREDICATE>
	Query<EnumeratorWithFilter<ENUMERATOR, PREDICATE>> where(PREDICATE&& predicate) {
		static_assert(std::is_same<typename std::result_of<PREDICATE(value_type)>::type, bool>::value, "PREDICATE must be a function: bool(value_type)");

		return Query<EnumeratorWithFilter<ENUMERATOR, PREDICATE>>(
			EnumeratorWithFilter<ENUMERATOR, PREDICATE>(std::move(enumerator), std::move(predicate))
		);
	}

	template <typename TRANSFORM>
	Query<EnumeratorWithTransform<ENUMERATOR, TRANSFORM>> select(TRANSFORM&& transform) {
		return Query<EnumeratorWithTransform<ENUMERATOR, TRANSFORM>>(
			EnumeratorWithTransform<ENUMERATOR, TRANSFORM>(std::move(enumerator), std::move(transform))
		);
	}

	template <typename TRANSFORM>
	Query<EnumeratorWithSelectMany<ENUMERATOR, TRANSFORM>> select_many(TRANSFORM&& transform) {
		return Query<EnumeratorWithSelectMany<ENUMERATOR, TRANSFORM>>(
			EnumeratorWithSelectMany<ENUMERATOR, TRANSFORM>(std::move(enumerator), std::move(transform))
		);
	}

	Query<EnumeratorWithTake<ENUMERATOR>> take(std::size_t count) {
		return Query<EnumeratorWithTake<ENUMERATOR>>(
			EnumeratorWithTake<ENUMERATOR>(std::move(enumerator), count)
		);
	}

	Query<EnumeratorWithSkip<ENUMERATOR>> skip(std::size_t count) {
		return Query<EnumeratorWithSkip<ENUMERATOR>>(
			EnumeratorWithSkip<ENUMERATOR>(std::move(enumerator), count)
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

	template <typename LIST>
	void to(LIST& l) {
		to(std::inserter(l, l.end()));
	}

	template <typename OUTPUT_ITERATOR, typename OUTPUT_VALUE_TYPE = typename detail::iterator_traits<OUTPUT_ITERATOR>::value_type>
	void to(OUTPUT_ITERATOR result) {
		while (enumerator.next()) {
			*result = enumerator.get();
			++result;
		}
	}

	template <typename ACTION>
	void foreach(ACTION action) {
		while (enumerator.next()) {
			action(enumerator.get());
		}
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename LIST, typename ITERATOR = decltype(std::declval<LIST>().begin())>
Query<Enumerator<ITERATOR>> from(LIST& l) {
	return Query<Enumerator<ITERATOR>>(Enumerator<ITERATOR>(l.begin(), l.end()));
}

template <typename value_type, int N>
Query<Enumerator<value_type*>> from(value_type (&l)[N]) {
	return Query<Enumerator<value_type*>>(Enumerator<value_type*>(l, l + N));
}

template <typename value_type>
Query<Enumerator<value_type*>> from(value_type* l, std::size_t len) {
	return Query<Enumerator<value_type*>>(Enumerator<value_type*>(l, l + len));
}
}
