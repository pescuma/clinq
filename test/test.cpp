#include <gtest/gtest.h>
#include <clinq.h>

using namespace clinq;
using namespace std;


class Helper
{
public:
	static int constructed;
	static int copied;
	static int moved;

	static void reset() {
		constructed = 0;
		copied = 0;
		moved = 0;
	}

	Helper() {
		constructed++;
	}

	Helper(const Helper& other) {
		copied++;
	}

	Helper(Helper&& other) {
		moved++;
	}

	Helper& operator=(const Helper& other) {
		copied++;
		return *this;
	}

	Helper& operator=(Helper&& other) {
		moved++;
		return *this;
	}
};

int Helper::constructed = 0;
int Helper::copied = 0;
int Helper::moved = 0;

TEST(clinq, fom_vector_to_vector) {
	vector<string> l;
	l.push_back("a");
	l.push_back("b");

	vector<string> b = from(l).to_vector();

	ASSERT_EQ(2, b.size())
				;
	ASSERT_EQ("a", b[0])
				;
	ASSERT_EQ("b", b[1])
				;
}

TEST(clinq, fom_list_to_vector) {
	list<string> l;
	l.push_back("a");
	l.push_back("b");

	vector<string> b = from(l).to_vector();

	ASSERT_EQ(2, b.size())
				;
	ASSERT_EQ("a", b[0])
				;
	ASSERT_EQ("b", b[1]);
}

TEST(clinq, select_int) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<size_t> b = from(l)
			.select([](string& i) {
				return i.length();
			})
			.to_vector();

	ASSERT_EQ(2, b.size());
	ASSERT_EQ(1, b[0]);
	ASSERT_EQ(2, b[1]);
}

TEST(clinq, where) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.where([](string& i) {
				return i.length() > 1;
			})
			.to_vector();

	ASSERT_EQ(1, b.size());
	ASSERT_EQ("bb", b[0]);
}

TEST(clinq, where_keeps_order) {
	list<string> l;
	l.push_back("xxx");
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.where([](string& i) {
				return i.length() > 1;
			})
			.to_vector();

	ASSERT_EQ(2, b.size());
	ASSERT_EQ("xxx", b[0]);
	ASSERT_EQ("bb", b[1]);
}

TEST(clinq, select_where) {
	list<string> l;
	l.push_back("xxx");
	l.push_back("a");
	l.push_back("bb");

	vector<size_t> b = from(l)
			.select([](string& i) {
				return i.length();
			})
			.where([](size_t i) {
				return i > 1;
			})
			.to_vector();

	ASSERT_EQ(2, b.size());
	ASSERT_EQ(3, b[0]);
	ASSERT_EQ(2, b[1]);
}

TEST(clinq, select_many) {
	vector<list<string>> l;
	l.push_back(list<string>());
	l[0].push_back("xxx");
	l.push_back(list<string>());
	l[1].push_back("a");
	l[1].push_back("bb");

	vector<string> b = from(l)
			.select_many([](list<string>& i) {
				return i;
			})
			.to_vector();

	ASSERT_EQ(3, b.size());
	ASSERT_EQ("xxx", b[0]);
	ASSERT_EQ("a", b[1]);
	ASSERT_EQ("bb", b[2]);
}

TEST(clinq, any_true) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.any([](string& i) {
				return i.length() > 1;
			});

	ASSERT_EQ(true, b);
}

TEST(clinq, any_false) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.any([](string& i) {
				return i.length() > 2;
			});

	ASSERT_EQ(false, b);
}

TEST(clinq, any_no_args_true) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.where([](string& i) {
				return i.length() > 1;
			})
			.any();

	ASSERT_EQ(true, b);
}

TEST(clinq, any_no_args_false) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.where([](string& i) {
				return i.length() > 2;
			})
			.any();

	ASSERT_EQ(false, b);
}

TEST(clinq, all_true) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.all([](string& i) {
				return i.length() < 10;
			});

	ASSERT_EQ(true, b);
}

TEST(clinq, all_false) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	bool b = from(l)
			.all([](string& i) {
				return i.length() > 1;
			});

	ASSERT_EQ(false, b);
}

TEST(clinq, take) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.take(1)
			.to_vector();

	ASSERT_EQ(1, b.size());
	ASSERT_EQ("a", b[0]);
}

TEST(clinq, take_more_than_exists) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.take(10)
			.to_vector();

	ASSERT_EQ(2, b.size());
	ASSERT_EQ("a", b[0]);
	ASSERT_EQ("bb", b[1]);
}

TEST(clinq, skip) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.skip(1)
			.to_vector();

	ASSERT_EQ(1, b.size());
	ASSERT_EQ("bb", b[0]);
}

TEST(clinq, skip_more_than_exists) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	vector<string> b = from(l)
			.skip(10)
			.to_vector();

	ASSERT_EQ(0, b.size());
}

TEST(clinq, cast_static) {
	list<int> l;
	l.push_back(10);

	vector<long> b = from(l)
			.cast_static<long>()
			.to_vector();

	ASSERT_EQ(1, b.size());
	ASSERT_EQ(10l, b[0]);
}

TEST(clinq, first) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	string b = from(l)
			.first();

	ASSERT_EQ("a", b);
}

TEST(clinq, first_or_default_string_char) {
	list<string> l;
	l.push_back("a");
	l.push_back("bb");

	string b = from(l)
			.first_or_default("x");

	ASSERT_EQ("a", b);
}

TEST(clinq, first_or_default_string_char_no_item) {
	list<string> l;

	string b = from(l)
			.first_or_default("x");

	ASSERT_EQ("x", b);
}

TEST(clinq, first_or_default_int) {
	list<int> l;
	l.push_back(10);
	l.push_back(11);

	int b = from(l)
			.first_or_default(5);

	ASSERT_EQ(10, b);
}

TEST(clinq, first_or_default_int_no_item) {
	list<int> l;

	int b = from(l)
			.first_or_default(5);

	ASSERT_EQ(5, b);
}

TEST(clinq, first_or_default_object) {
	list<Helper> l;
	l.push_back(Helper());
	l.push_back(Helper());

	Helper tmp;

	Helper::reset();

	Helper& b = from(l)
			.first_or_default(tmp);

	EXPECT_EQ(0, Helper::constructed);
	EXPECT_EQ(0, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}

TEST(clinq, first_or_default_object_no_item) {
	list<Helper> l;

	Helper tmp;

	Helper::reset();

	Helper& b = from(l)
			.first_or_default(tmp);

	EXPECT_EQ(0, Helper::constructed);
	EXPECT_EQ(0, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}

TEST(clinq, first_or_default_tmp_object) {
	list<Helper> l;
	l.push_back(Helper());
	l.push_back(Helper());

	Helper::reset();

	Helper b = from(l)
			.first_or_default(Helper());

	EXPECT_EQ(1, Helper::constructed);
	EXPECT_EQ(1, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}

TEST(clinq, first_or_default_tmp_object_no_item) {
	list<Helper> l;

	Helper::reset();

	Helper b = from(l)
			.first_or_default(Helper());

	EXPECT_EQ(1, Helper::constructed);
	EXPECT_EQ(0, Helper::copied);
	EXPECT_EQ(1, Helper::moved);
}

TEST(clinq, first_or_default_non_ref_list_tmp_object) {
	list<Helper> l;
	l.push_back(Helper());
	l.push_back(Helper());

	Helper::reset();

	Helper b = from(l)
			.cast_static<Helper>()
			.first_or_default(Helper());

	EXPECT_EQ(1, Helper::constructed);
	EXPECT_EQ(1, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}

TEST(clinq, first_or_default_non_ref_list_tmp_object_no_item) {
	list<Helper> l;

	Helper::reset();

	Helper b = from(l)
			.cast_static<Helper>()
			.first_or_default(Helper());

	EXPECT_EQ(1, Helper::constructed);
	EXPECT_EQ(0, Helper::copied);
	EXPECT_EQ(1, Helper::moved);
}

TEST(clinq, first_or_default_non_ref_list_object) {
	list<Helper> l;
	l.push_back(Helper());
	l.push_back(Helper());

	Helper tmp;

	Helper::reset();

	Helper b = from(l)
			.cast_static<Helper>()
			.first_or_default(tmp);

	EXPECT_EQ(0, Helper::constructed);
	EXPECT_EQ(1, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}

TEST(clinq, first_or_default_non_ref_list_object_no_item) {
	list<Helper> l;

	Helper tmp;

	Helper::reset();

	Helper b = from(l)
			.cast_static<Helper>()
			.first_or_default(tmp);

	EXPECT_EQ(0, Helper::constructed);
	EXPECT_EQ(1, Helper::copied);
	EXPECT_EQ(0, Helper::moved);
}
