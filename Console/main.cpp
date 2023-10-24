#include <Windows.h>
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <vector>
#include <string>
#include <regex>


int main() {
	
	std::vector<uint8_t> writeData = { 1,2,3,4,5,6,7,8 };

	std::vector<uint8_t> msgData(writeData.size() + 10);
	//msgData.resize(msgData.size());
	msgData = std::move(writeData);


	//msgData.reserve(25); // optional
	//std::move(writeData.begin(), writeData.end(), std::back_inserter(msgData));

	//msgData.insert(msgData.begin(), make_move_iterator(writeData.begin()), make_move_iterator(writeData.end()));
	msgData.push_back(777);
	return 0;
}
