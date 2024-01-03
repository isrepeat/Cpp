#include <Helpers/ThreadSafeObject.hpp>

struct Data {
	int x;
	int y;
	Data* operator->() { 
		return this;
	}
};


int main() {
	Data data;
	std::mutex mx;

	auto locked = ThreadSafeObject<Data>::Locked(mx, data);
	locked.templateNotes;
	locked.Get().x;
	locked->x;

	std::unique_ptr<Data> dataUptr;

	auto locked2 = ThreadSafeObject<std::unique_ptr<Data>>::Locked(mx, dataUptr);
	locked2.templateNotes;
	locked2.Get().get();
	locked2->x;

	return 0;
}