#include <span>
#include <vector>
#include <iostream>


struct Temp {
	int a;
	wchar_t* text;
	float b;
};

void TestVectorTempStructToBytesAndBack() {
	std::vector<uint8_t> bytes;
	int testVal = 0;
	{
		wchar_t* wstr1 = new wchar_t[] {L"AAA BBB CCC DDD"};
		wchar_t* wstr2 = new wchar_t[] {L"1111 2222"};

		std::wstring stdWstr(wstr1);
		wchar_t* stdWstrPtr = stdWstr.data();

		std::vector<Temp> origTemps{ {5, wstr1, 3.14f}, {7, wstr2, 9.11f} };
		auto ptr1 = reinterpret_cast<uint8_t*>(origTemps.data());
		bytes = std::vector<uint8_t>(ptr1, ptr1 + sizeof(Temp) * origTemps.size());

		//Temp origTemps[2]{ {5, 3.14f}, {7, 9.11f} };
		//auto ptr1 = reinterpret_cast<uint8_t*>(&(origTemps[0]));
		//auto bytes = std::vector<uint8_t>(ptr1, ptr1 + sizeof(origTemps));

		//std::vector<Temp> tmp(2);

		//using aaa = Temp[2];
		//aaa* tempPtr = reinterpret_cast<aaa*>(bytes.data());


		Temp* tempPtr;
		tempPtr = reinterpret_cast<Temp*>(bytes.data());

		auto t0 = tempPtr[0];
		auto t1 = tempPtr[1];


		//std::vector<Temp> tempVec;
		//tempVec.resize(2);

		//auto tempVecPtr = tempVec.data();

		//tempPtr = reinterpret_cast<Temp*>(bytes.data());

		//auto t0 = tempPtr[0];
		//auto t1 = tempPtr[1];


		//using aaa = std::vector<Temp>;
		//aaa* tempPtr = reinterpret_cast<aaa*>(bytes.data());

		//Temp* tempPtr[2] = reinterpret_cast<Temp*[]>(bytes.data());

		//auto newPtr = tmp.data();
		//newPtr = origTemps.data();

		////reinterpret_cast<>
	}

	std::vector<Temp> newTemp(2);
	auto ptr2 = reinterpret_cast<uint8_t*>(newTemp.data());
	std::copy(bytes.begin(), bytes.end(), ptr2);

	return;
}



enum class MessageType {
	None,
	Connect,
	FrameData,
	Stop,
};

//struct Message {
//	MessageType type;
//	std::vector<uint8_t> data;
//	//std::span<uint8_t> data;
//	//int size;
//	//uint8_t* data;
//};

//void TestMessageStructToBytesAndBack() {
//	std::vector<uint8_t> networkBytes;
//	{
//		std::vector<uint8_t> someData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
//
//		//Message message;
//		//message.type = MessageType::FrameData;
//		//message.data = std::move(someData);
//
//		//auto ptr1 = reinterpret_cast<uint8_t*>(&message);
//		////bytes = std::vector<uint8_t>(ptr1, ptr1 + sizeof(Message));
//		//std::span<uint8_t> spanMessage(ptr1, sizeof(Message));
//		//bytes = std::vector<uint8_t>(spanMessage.begin(), spanMessage.end());
//		
//		
//		uint8_t* storage = new uint8_t[sizeof(Message) + someData.size()];
//		Message* message = reinterpret_cast<Message*>(storage);
//		message->type = MessageType::FrameData;
//		//message->data = std::move(someData);
//		//message->data = someData;
//		message->size = someData.size();
//		message->data = someData.data();
//
//		//std::span<uint8_t> spanMessage(storage, sizeof(Message) + someData.size());
//		//std::vector<uint8_t> serializedBytes = std::vector<uint8_t>(spanMessage.begin(), spanMessage.end());
//
//		//networkBytes.resize(spanMessage.size());
//		//std::copy(spanMessage.begin(), spanMessage.end(), networkBytes.data());
//		networkBytes.resize(sizeof(Message) + someData.size());
//		std::copy(storage, storage + sizeof(Message) + someData.size(), networkBytes.data());
//
//		Message* message__ = reinterpret_cast<Message*>(networkBytes.data());
//		message__->data = someData.size();
//
//		Message* tempPtr;
//		tempPtr = reinterpret_cast<Message*>(networkBytes.data());
//
//		auto t0 = tempPtr[0];
//	}
//
//	//Message newMessage = *reinterpret_cast<Message*>(bytes.data());
//	Message newMessage;
//	auto ptr2 = reinterpret_cast<uint8_t*>(&newMessage);
//	std::copy(networkBytes.begin(), networkBytes.end(), ptr2);
//
//	return;
//}


//void TestMessageStructToBytesAndBack() {
//	std::vector<uint8_t> networkBytes;
//	{
//		std::vector<uint8_t> someData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
//		auto someDataItBegin = someData.begin();
//		auto someDataItEnd = someData.end();
//
//		uint8_t* storage = new uint8_t[sizeof(Message) + someData.size()]{};
//		Message* message = reinterpret_cast<Message*>(storage);
//		message->type = MessageType::FrameData;
//		message->data = std::move(someData);
//
//		auto MessageDataItBegin = message->data.begin();
//		auto MessageDataItEnd = message->data.end();
//
//		std::span<uint8_t> spanMessage(storage, sizeof(Message) + someData.size());
//		//std::vector<uint8_t> serializedBytes = std::vector<uint8_t>(spanMessage.begin(), spanMessage.end());
//
//		networkBytes.resize(spanMessage.size());
//		std::copy(spanMessage.begin(), spanMessage.end(), networkBytes.data());
//
//		delete[] storage;
//		//networkBytes.resize(sizeof(Message) + someData.size());
//		//std::copy(storage, storage + sizeof(Message) + someData.size(), networkBytes.data());
//
//		Message* tempPtr;
//		tempPtr = reinterpret_cast<Message*>(networkBytes.data());
//
//		auto t0 = tempPtr[0];
//	}
//
//	//Message newMessage = *reinterpret_cast<Message*>(bytes.data());
//	Message newMessage;
//	auto ptr2 = reinterpret_cast<uint8_t*>(&newMessage);
//	std::copy(networkBytes.begin(), networkBytes.end(), ptr2);
//
//	return;
//}



struct Message {
	MessageType type;
	int msgSize;
	uint8_t* msgData;
};

void foo(std::span<uint8_t> sp) {
}

void TestMessageStructToBytesAndBack() {
	std::vector<uint8_t> networkBytes;
	{
		std::vector<uint8_t> someData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

		uint8_t* storage = new uint8_t[sizeof(Message) + someData.size()]{};
		Message* message = reinterpret_cast<Message*>(storage);
		message->type = MessageType::FrameData;
		message->msgSize = someData.size();
		message->msgData = storage + sizeof(Message);
		std::copy(someData.begin(), someData.end(), storage + sizeof(Message));

		std::span<uint8_t> spanMessage(storage, sizeof(Message) + someData.size());

	    std::vector<uint8_t>&& refVec = std::move(networkBytes);

		foo(refVec);

		networkBytes.resize(spanMessage.size());
		std::copy(spanMessage.begin(), spanMessage.end(), networkBytes.data());

		delete[] storage;
		
		Message* tempPtr;
		tempPtr = reinterpret_cast<Message*>(networkBytes.data());
	}

	Message newMessage;
	auto ptr2 = reinterpret_cast<uint8_t*>(&newMessage);
	std::copy(networkBytes.begin(), networkBytes.end(), ptr2);
	newMessage.msgData = networkBytes.data() + sizeof(Message);

	auto d1 = newMessage.msgData[0];
	auto d5 = newMessage.msgData[5];

	return;
}



//struct Message {
//	MessageType type;
//	std::vector<uint8_t> msgData;
//};
//
//void TestMessageStructToBytesAndBack() {
//	std::vector<uint8_t> networkBytes;
//	{
//		std::vector<uint8_t> someData = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
//		auto someDataItBegin = someData.begin();
//		auto someDataItEnd = someData.end();
//
//		uint8_t* storage = new uint8_t[sizeof(Message) + someData.size()]{};
//		Message* message = reinterpret_cast<Message*>(storage);
//		message->type = MessageType::FrameData;
//		message->msgData = storage + sizeof(Message);
//		std::copy(someData.begin(), someData.end(), storage + sizeof(Message));
//
//		auto MessageDataItBegin = message->msgData;
//		auto MessageDataItEnd = message->msgData + message->msgSize;
//
//		std::span<uint8_t> spanMessage(storage, sizeof(Message) + someData.size());
//		//std::vector<uint8_t> serializedBytes = std::vector<uint8_t>(spanMessage.begin(), spanMessage.end());
//
//		networkBytes.resize(spanMessage.size());
//		std::copy(spanMessage.begin(), spanMessage.end(), networkBytes.data());
//
//		delete[] storage;
//		//networkBytes.resize(sizeof(Message) + someData.size());
//		//std::copy(storage, storage + sizeof(Message) + someData.size(), networkBytes.data());
//
//		Message* tempPtr;
//		tempPtr = reinterpret_cast<Message*>(networkBytes.data());
//
//		auto t0 = tempPtr[0];
//	}
//
//	Message newMessage;
//	auto ptr2 = reinterpret_cast<uint8_t*>(&newMessage);
//	std::copy(networkBytes.begin(), networkBytes.end(), ptr2);
//	newMessage.msgData = networkBytes.data() + sizeof(Message);
//
//	auto d1 = newMessage.msgData[0];
//	auto d5 = newMessage.msgData[5];
//
//	return;
//}

int main() {
	//TestVectorTempStructToBytesAndBack();
	TestMessageStructToBytesAndBack();
	return 0;
}