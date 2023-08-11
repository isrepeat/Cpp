#include <map>
#include <array>
#include <string>
#include <utility>
#include <json_struct/json_struct.h>

//template<typename T>
//struct Vec3
//{
//    T data[3];
//};

//Nested types are fully supported and the limitation of the data structures
//that can be expressed is limited by C++
//namespace JS
//{
//    template<typename T>
//    struct TypeHandler<Vec3<T>>
//    {
//        static inline Error to(Vec3<T>& to_type, ParseContext& context)
//        {
//            return TypeHandler<T[3]>::to(to_type.data, context);
//        }
//
//        static inline void from(const Vec3<T>& from_type, Token& token, Serializer& serializer)
//        {
//            return TypeHandler<T[3]>::from(from_type.data, token, serializer);
//        }
//    };
//}

//struct InnerJsonData
//{
//    int key;
//    double value;
//    JS_OBJ(key, value);
//};

//const char json[] = R"json(
//{
//    "key" : "value",
//    "vec" : [
//        { "key" : 4, "value": 1.0 },
//        { "key" : 5, "value": 2.0 },
//        { "key" : 6, "value": 3.0 }
//    ]
//}
//)json";


//namespace JS
//{
//    template<typename T>
//    struct TypeHandler<Vec3<T>>
//    {
//        static inline Error to(Vec3<T>& to_type, ParseContext& context)
//        {
//            return TypeHandler<T[3]>::to(to_type.data, context);
//        }
//
//        static inline void from(const Vec3<T>& from_type, Token& token, Serializer& serializer)
//        {
//            return TypeHandler<T[3]>::from(from_type.data, token, serializer);
//        }
//    };
//}

//template<std::size_t N, class T>
//constexpr std::size_t countof(T(&)[N]) { return N; }
//


//template<std::size_t N>
//constexpr const char* JsKey(const char key[N]) {
//    if (N != 0) {
//        //key[0] = 
//    }
//}
//
//#define JS_KEY(key)

struct InnerJsonData
{
    std::string key;
    double value;
    JS_OBJ(key, value);
};

const char json[] = R"json(
{
  "Key" : "wow",
  "vec" : [
    { "key" : 4, "value": 1.0 },
    { "key" : 5, "value": 2.0 },
    { "key" : 6, "value": 3.0 }
  ]
}
)json";

//std::string myKey;
const char myKey[] = "asdasda";

struct JsonData
{
    std::string key = "hi";
    //Vec3<InnerJsonData> vec;
    InnerJsonData vec[3];

    JS_OBJECT(JS_MEMBER_WITH_NAME(key, myKey),
              JS_MEMBER(vec));
};



    
int main()
{
    //int value[findValue(map_items, 10)];

    JsonData dataStruct;
    JS::ParseContext parseContext(json);
    if (parseContext.parseTo(dataStruct) != JS::Error::NoError)
    {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return -1;
    }

    //fprintf(stdout, "Key is: %s, number is %d - %f, %d - %f, %d - %f\n",
    //    dataStruct.key.c_str(),
    //    dataStruct.vec.data[0].key,
    //    dataStruct.vec.data[0].value,
    //    dataStruct.vec.data[1].key,
    //    dataStruct.vec.data[1].value,
    //    dataStruct.vec.data[2].key,
    //    dataStruct.vec.data[2].value);

    fprintf(stdout, "Key is: %s, number is %s - %f, %d - %f, %d - %f\n",
        dataStruct.key.c_str(),
        dataStruct.vec[0].key.c_str(),
        dataStruct.vec[0].value,
        dataStruct.vec[1].key,
        dataStruct.vec[1].value,
        dataStruct.vec[2].key,
        dataStruct.vec[2].value);

    system("pause");
    return 0;
}