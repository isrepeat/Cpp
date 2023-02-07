#define ImplDef		\
void Foo() {		\
	int x = 1;		\
	int a = x * 5;	\
	return;			\
}					\



class Temp {
public:
	Temp() = default;
	ImplDef
};

int main() {

	Temp temp;
	temp.Foo();

	return 0;
}
