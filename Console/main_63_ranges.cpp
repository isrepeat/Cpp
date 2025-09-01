#include <Helpers/Std/Extensions/rangesEx.h>
#include <Helpers/Logger.h>

namespace Test {
	//
	// std::ex::ranges::views::to<TContainer>
	//
	namespace ranges_to {
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
			LOG_ASSERT(got == expected, "to<vector> должен материализовать порядок элементов");
		}


		void Test_To_Set() {
			std::vector<int> v{ 5, 3, 5, 2, 3 };

			std::set<int> got =
				v
				| STD_EXT_NS::ranges::views::to<std::set<int>>();

			std::set<int> expected{ 2, 3, 5 };
			LOG_ASSERT(got == expected, "to<set> должен убрать дубликаты и отсортировать по правилам set");
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
			LOG_ASSERT(got == expected, "to<map> должен корректно копировать пары <K,V>");
		}
	} // namespace ranges_to


	//
	// std::ex::ranges::views::drop_last(<count>)
	//
	namespace ranges_drop_last {
		void Test_DropLast_Vector_Basic() {
			std::vector<int> v{ 1, 2, 3, 4, 5 };

			auto rng =
				v
				| STD_EXT_NS::ranges::views::drop_last(2);

			std::vector<int> got =
				rng
				| STD_EXT_NS::ranges::views::to<std::vector<int>>();

			std::vector<int> expected{ 1, 2, 3 };
			LOG_ASSERT(got == expected, "drop_last(2) на [1..5] должен дать [1..3]");
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
			LOG_ASSERT(got == expected, "drop_last(0) должен вернуть все элементы без изменений");
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
			LOG_ASSERT(got == expected, "drop_last(N>Nsize) должен вернуть пустой диапазон");
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
			LOG_ASSERT(got == expected, "drop_last на forward_list должен работать корректно");
		}
	} // namespace ranges_drop_last


	//
	// std::ex::ranges::views::concat(<TRange>)
	//
	namespace ranges_concat {
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
			LOG_ASSERT(got == expected, "concat(vec[1,2,3], vec[4,5]) должен дать [1..5]");
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
			LOG_ASSERT(got == expected, "concat(empty, [7,8]) должен дать [7,8]");
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
			LOG_ASSERT(got == expected, "concat([9], empty) должен вернуть [9]");
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
				LOG_ERROR_D("concat(empty, empty) должен быть пустым");
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
			LOG_ASSERT(got == expected, "concat должен сохранять порядок: [2,4] + [50,60,70]");
		}
	} // namespace ranges_concat


	//
	// std::ex::ranges::views::flatten_tree()
	//
	namespace ranges_flatten_tree {
		struct Node {
			int value{};
			std::vector<std::shared_ptr<Node>> children;

			explicit Node(int v) : value{ v } {}
		};


		void Test_FlattenTree_Empty() {
			std::vector<std::shared_ptr<Node>> roots; // пусто

			auto rng =
				roots
				| STD_EXT_NS::ranges::views::flatten_tree(
					[](const std::shared_ptr<Node>& node) {
						return node->children;
					});

			auto got =
				rng
				| STD_EXT_NS::ranges::views::to<std::vector<std::shared_ptr<Node>>>();

			LOG_ASSERT(got.empty(), "flatten_tree от пустого списка корней должен быть пустым");
		}


		void Test_FlattenTree_SingleNode() {
			auto root = std::make_shared<Node>(42);
			std::vector<std::shared_ptr<Node>> roots{ root };

			auto rng =
				roots
				| STD_EXT_NS::ranges::views::flatten_tree(
					[](const std::shared_ptr<Node>& node) {
						return node->children;
					});

			auto got =
				rng
				| STD_EXT_NS::ranges::views::to<std::vector<std::shared_ptr<Node>>>();

			LOG_ASSERT(
				got.size() == 1 &&
				got[0]->value == 42,
				"flatten_tree от одного узла должен вернуть его самого");
		}


		void Test_FlattenTree_LinearChain() {
			auto n1 = std::make_shared<Node>(1);
			auto n2 = std::make_shared<Node>(2);
			auto n3 = std::make_shared<Node>(3);
			n1->children = { n2 };
			n2->children = { n3 };
			std::vector<std::shared_ptr<Node>> roots{ n1 };

			auto rng =
				roots
				| STD_EXT_NS::ranges::views::flatten_tree(
					[](const std::shared_ptr<Node>& node) {
						return node->children;
					});

			auto got =
				rng
				| std::ranges::views::transform(
					[](const std::shared_ptr<Node>& node) {
						return node->value;
					})
				| STD_EXT_NS::ranges::views::to<std::vector<int>>();

			std::vector<int> expected{ 1, 2, 3 };
			LOG_ASSERT(got == expected, "flatten_tree должен обойти линейную цепочку по порядку");
		}


		void Test_FlattenTree_Branching() {
			auto root = std::make_shared<Node>(10);
			auto left = std::make_shared<Node>(20);
			auto right = std::make_shared<Node>(30);
			root->children = { left, right };
			std::vector<std::shared_ptr<Node>> roots{ root };

			auto rng =
				roots
				| STD_EXT_NS::ranges::views::flatten_tree(
					[](const std::shared_ptr<Node>& n) { return n->children; });

			auto got =
				rng
				| std::ranges::views::transform(
					[](const std::shared_ptr<Node>& node) {
						return node->value;
					})
				| STD_EXT_NS::ranges::views::to<std::vector<int>>();

			// порядок DFS: root, затем left, затем right
			std::vector<int> expected{ 10, 20, 30 };
			LOG_ASSERT(got == expected, "flatten_tree должен обойти root-left-right в DFS-порядке");
		}


		void Test_FlattenTree_MultiRoot() {
			auto r1 = std::make_shared<Node>(1);
			auto r2 = std::make_shared<Node>(2);
			std::vector<std::shared_ptr<Node>> roots{ r1, r2 };

			auto rng =
				roots
				| STD_EXT_NS::ranges::views::flatten_tree(
					[](const std::shared_ptr<Node>& n) { return n->children; });

			auto got =
				rng
				| std::ranges::views::transform(
					[](const std::shared_ptr<Node>& node) {
						return node->value;
					})
				| STD_EXT_NS::ranges::views::to<std::vector<int>>();

			std::vector<int> expected{ 1, 2 };
			LOG_ASSERT(got == expected, "flatten_tree должен обходить все корни по порядку");
		}
	} // namespace ranges_flatten_tree
}


int main() {
	Test::ranges_to::Test_To_Vector();
	Test::ranges_to::Test_To_Set();
	Test::ranges_to::Test_To_Map();

	Test::ranges_drop_last::Test_DropLast_Vector_Basic();
	Test::ranges_drop_last::Test_DropLast_Vector_Zero();
	Test::ranges_drop_last::Test_DropLast_Vector_MoreThanSize();
	Test::ranges_drop_last::Test_DropLast_ForwardList();

	Test::ranges_concat::Test_Concat_SimpleVectors();
	Test::ranges_concat::Test_Concat_EmptyFirst();
	Test::ranges_concat::Test_Concat_EmptySecond();
	Test::ranges_concat::Test_Concat_BothEmpty();
	Test::ranges_concat::Test_Concat_WithPipelines();

	Test::ranges_flatten_tree::Test_FlattenTree_Empty();
	Test::ranges_flatten_tree::Test_FlattenTree_SingleNode();
	Test::ranges_flatten_tree::Test_FlattenTree_LinearChain();
	Test::ranges_flatten_tree::Test_FlattenTree_Branching();
	Test::ranges_flatten_tree::Test_FlattenTree_MultiRoot();

	LOG_DEBUG_D("All tests passed.");
	return 0;
}