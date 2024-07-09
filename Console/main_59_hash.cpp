#include <unordered_map>
#include <iostream>
#include <string>

int main() {
	std::string str = "Meet the new boss...";
	std::size_t str_hash = std::hash<std::string>{}(str);
	std::cout << "hash('" << str << "') =\t" << str_hash << '\n';

	system("pause");
	return 0;
}