//#include <iostream>
//#include <functional>
//#include <unordered_map>
//
//struct A {
//    uint32_t a;
//    uint8_t  b;
//    uint16_t c;
//};
//
////class Temp {
////public:
////    void Foo();
////}
//
//struct ValueWrapper {
//    using value_type = uint64_t;
//
//    template <typename Obj, typename T>
//    ValueWrapper(Obj& obj, T Obj::* member) {
//        get = [&, member]() { return obj.*member; };
//        set = [&, member](value_type value) mutable { obj.*member = value; };
//    }
//
//    ValueWrapper() = default;
//
//    ValueWrapper& operator=(value_type value) {
//        set(value);
//        return *this;
//    }
//
//    operator value_type() {
//        return get();
//    }
//
//    std::function<value_type()> get;
//    std::function<void(value_type)> set;
//};
//
//std::unordered_map<std::string, ValueWrapper> make_map(A& a) {
//    std::unordered_map<std::string, ValueWrapper> map;
//
//    map["field1"] = ValueWrapper(a, &A::a);
//    map["field2"] = ValueWrapper(a, &A::b);
//    map["field3"] = ValueWrapper(a, &A::c);
//
//    return map;
//}
//
////std::unordered_map<ValueWrapper, std::string> make_map_2(A& a) {
////    std::unordered_map<ValueWrapper, std::string> map;
////
////    map[ValueWrapper(a, &A::a)] = "aaa";
////    map[ValueWrapper(a, &A::b)] = "bbb";
////    map[ValueWrapper(a, &A::c)] = "ccc";
////
////    return map;
////}
//
//
//
//
//class Temp {
//public:
//    void some_function() {
//    }
//    void some_function_2() {
//    }
//};
//
////typedef void (*ScriptFunction)(); // function pointer type
//typedef std::unordered_map<void (Temp::*)(), std::string> script_map;
//
//int main() {
//    script_map m;
//    m.emplace(&Temp::some_function, "000");
//    m.emplace(&Temp::some_function_2, "111");
//
//
//    auto res = m[&Temp::some_function];
//
//    //A a{ 1,2,3 };
//
//    //auto map = make_map(a);
//
//    //map["field2"] = 67;
//    ////map[ValueWrapper(a, &A::b)] = "xxx";
//
//    ////std::cout << a.a << " " << static_cast<int>(a.b) << " " << a.c << std::endl;
//    ////std::cout << map["field1"] << " " << map["field2"] << " " << map["field3"] << std::endl;
//    return 0;
//}