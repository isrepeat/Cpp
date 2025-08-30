#include <Helpers/Std/Extensions/rangesEx.h>
#include <Helpers/Logger.h>

namespace Test {
	//
	// std::ex::ranges::views::to<TContainer>
	//
	void Test_To_Vector() {
		std::list<int> lst{ 3, 1, 4 };

		auto rng =
			lst
			| std::views::transform([](int x) {
			return x * 2;
				});

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 6, 2, 8 };
		LOG_ASSERT(got == expected, "to<vector> ������ ��������������� ������� ���������");
	}


	void Test_To_Set() {
		std::vector<int> v{ 5, 3, 5, 2, 3 };

		std::set<int> got =
			v
			| STD_EXT_NS::ranges::views::to<std::set<int>>();

		std::set<int> expected{ 2, 3, 5 };
		LOG_ASSERT(got == expected, "to<set> ������ ������ ��������� � ������������� �� �������� set");
	}


	void Test_To_Map() {
		std::vector<std::pair<std::string, int>> items{
			{ "one", 1 },
			{ "two", 2 },
			{ "three", 3 }
		};

		std::map<std::string, int> got =
			items
			| STD_EXT_NS::ranges::views::to<std::map<std::string, int>>();

		std::map<std::string, int> expected{
			{ "one", 1 },
			{ "three", 3 },
			{ "two", 2 }
		};
		LOG_ASSERT(got == expected, "to<map> ������ ��������� ���������� ���� <K,V>");
	}


	//
	// std::ex::ranges::views::drop_last(<count>)
	//
	void Test_DropLast_Vector_Basic() {
		std::vector<int> v{ 1, 2, 3, 4, 5 };

		auto rng =
			v
			| STD_EXT_NS::ranges::views::drop_last(2);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 1, 2, 3 };
		LOG_ASSERT(got == expected, "drop_last(2) �� [1..5] ������ ���� [1..3]");
	}


	void Test_DropLast_Vector_Zero() {
		std::vector<int> v{ 1, 2, 3 };

		auto rng =
			v
			| STD_EXT_NS::ranges::views::drop_last(0);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 1, 2, 3 };
		LOG_ASSERT(got == expected, "drop_last(0) ������ ������� ��� �������� ��� ���������");
	}


	void Test_DropLast_Vector_MoreThanSize() {
		std::vector<int> v{
			10, 20
		};

		auto rng =
			v
			| STD_EXT_NS::ranges::views::drop_last(5);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{/* empty */ };
		LOG_ASSERT(got == expected, "drop_last(N>Nsize) ������ ������� ������ ��������");
	}


	void Test_DropLast_ForwardList() {
		std::forward_list<int> fl{
			1, 2, 3, 4
		};

		auto rng =
			fl
			| STD_EXT_NS::ranges::views::drop_last(1);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 1, 2, 3 };
		LOG_ASSERT(got == expected, "drop_last �� forward_list ������ �������� ���������");
	}


	//
	// std::ex::ranges::views::concat(<TRange>)
	//
	void Test_Concat_SimpleVectors() {
		std::vector<int> a{ 1, 2, 3 };
		std::vector<int> b{ 4, 5 };

		auto rng =
			a
			| STD_EXT_NS::ranges::views::concat(b);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 1, 2, 3, 4, 5 };
		LOG_ASSERT(got == expected, "concat(vec[1,2,3], vec[4,5]) ������ ���� [1..5]");
	}


	void Test_Concat_EmptyFirst() {
		std::vector<int> a{ /* empty */ };
		std::vector<int> b{ 7, 8 };

		auto rng =
			a
			| STD_EXT_NS::ranges::views::concat(b);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 7, 8 };
		LOG_ASSERT(got == expected, "concat(empty, [7,8]) ������ ���� [7,8]");
	}


	void Test_Concat_EmptySecond() {
		std::vector<int> a{ 9 };
		std::vector<int> b{ /* empty */ };

		auto rng =
			a
			| STD_EXT_NS::ranges::views::concat(b);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 9 };
		LOG_ASSERT(got == expected, "concat([9], empty) ������ ������� [9]");
	}


	void Test_Concat_BothEmpty() {
		std::vector<int> a{ /* empty */ };
		std::vector<int> b{ /* empty */ };

		auto rng =
			a
			| STD_EXT_NS::ranges::views::concat(b);

		auto it = std::ranges::begin(rng);
		auto ed = std::ranges::end(rng);

		if (!(it == ed)) {
			LOG_ERROR_D("concat(empty, empty) ������ ���� ������");
			assert(false);
		}
	}


	void Test_Concat_WithPipelines() {
		std::vector<int> a{ 1, 2, 3, 4 };
		std::vector<int> b{ 5, 6, 7 };

		auto left =
			a
			| std::views::filter([](int x) {
			return x % 2 == 0;
				}); // 2,4

		auto right =
			b
			| std::views::transform([](int x) {
			return x * 10;
				}); // 50,60,70

		auto rng =
			left
			| STD_EXT_NS::ranges::views::concat(right);

		std::vector<int> got =
			rng
			| STD_EXT_NS::ranges::views::to<std::vector<int>>();

		std::vector<int> expected{ 2, 4, 50, 60, 70 };
		LOG_ASSERT(got == expected, "concat ������ ��������� �������: [2,4] + [50,60,70]");
	}
}


int main() {
	// to<T>
	Test::Test_To_Vector();
	Test::Test_To_Set();
	Test::Test_To_Map();

	// drop_last
	Test::Test_DropLast_Vector_Basic();
	Test::Test_DropLast_Vector_Zero();
	Test::Test_DropLast_Vector_MoreThanSize();
	Test::Test_DropLast_ForwardList();

	// concat
	Test::Test_Concat_SimpleVectors();
	Test::Test_Concat_EmptyFirst();
	Test::Test_Concat_EmptySecond();
	Test::Test_Concat_BothEmpty();
	Test::Test_Concat_WithPipelines();

	LOG_DEBUG_D("All tests passed.");
	return 0;
}