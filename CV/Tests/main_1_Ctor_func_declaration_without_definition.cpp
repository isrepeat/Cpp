class ClassA {
public:
	ClassA();
};

class ClassB {
	ClassA();
};

int main() {
	ClassA classA;
	ClassB classB;

	return 0;
}