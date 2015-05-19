# clinq

Dummy c++ linq implementation. 

No external dependencies, no exceptions and no chance that the code can be undestood.

Currently it suports: where, select, select_many, take, skip, to_vector, to_list, to_set, to (container or output iterator), foreach, any, all, first, first_or_default.

```cpp
#include <clinq.h>
using namespace clinq;

auto a = from(list)
		.where([](MyType& c) {
			return c.works;
		})
		.select([](MyType& c) {
			return c.name;
		})
		.to_list();


for(auto b : from(list).where([](MyType& c) { return c.works; }) {
}

```
