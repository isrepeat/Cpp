//#include <iostream>
//#include <utility>
//#include <string>
//#include <tuple>
//
//int main() {
//	printf("Old %d %s %d \n\n", 11, "THE OLD", 22);
//	
//	std::string format = "New %d %s %d \n\n";
//	auto tupleArgs = std::make_tuple(33, "THE NEW", 44);
//	auto tupleAllParams = std::tuple_cat(std::make_tuple(format.c_str()), tupleArgs);
//
//	std::apply(printf, tupleAllParams);
//
//	system("pause");
//	return 0;
//}