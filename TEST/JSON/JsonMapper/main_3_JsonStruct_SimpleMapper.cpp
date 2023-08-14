//#define WIN32_LEAN_AND_MEAN
#include <map>
#include <array>
#include <string>
#include <thread>
#include <utility>
//#include <Windows.h>
#include <json_struct/json_struct.h>
#include "Printer.h"


struct Purchase {
    int id = -1;
    float price;
    std::string name = "...";

    //JS_OBJ(id, price, name);
    JS_OBJECT(
        JS_MEMBER_ALIASES(id, "Id"),
        JS_MEMBER_ALIASES(name, "Name"),
        JS_MEMBER_ALIASES(price, "Price")
        );
};


struct Picture {
    std::vector<uint8_t> imageData = { 16, 32 };
};


namespace JS
{
    template<>
    struct TypeHandler<Picture>
    {
        static inline Error to(Picture& to_type, ParseContext& context)
        {
            //There exists a TypeHandler for T[N] already, so all we have to do is unwrap the
            //data and call the other TypeHandler specialisation
            //return TypeHandler<std::vector<uint8_t>>::to(to_type.imageData, context);
            
            auto error = JS::Error::NoError;

            std::string bytes;
            error = TypeHandler<std::string>::to(bytes, context);
            
            if (error != JS::Error::NoError) {
                return error;
            }
            
            to_type.imageData = std::vector<uint8_t>(bytes.begin(), bytes.end());
            return error;
        }

        static inline void from(const Picture& from_type, Token& token, Serializer& serializer)
        {
            //return TypeHandler<std::vector<uint8_t>>::from(from_type.imageData, token, serializer);

            auto bytes = std::string(from_type.imageData.begin(), from_type.imageData.end());
            return TypeHandler<std::string>::from(bytes, token, serializer);
        }
    };
}




struct Person {
    //Purchase purchases[4];
    std::vector<Purchase> purchases;
    std::string firstName = "First Name";
    std::string secondName = "Second Name";
    //std::vector<uint8_t> userPicture{ 11, 22, 33, 44 };
    Picture userPicture;

    JS_OBJECT(
        JS_MEMBER_ALIASES(purchases, "Purchases"),
        JS_MEMBER_ALIASES(firstName, "FirstName"),
        JS_MEMBER_ALIASES(secondName, "SecondName"),
        JS_MEMBER_ALIASES(userPicture, "UserPicture")
    );
};



void TestParseScalars() {
    const char jsonPerson[] = R"json(
    {
        "FirstName": "Alexandr",
        "secondName": "Bondarenko"
    }
    )json";

    const char jsonPersonPurchase[] = R"json(
    {
        "firstName": "Alexandr",
        "secondName": "Bondarenko",
        "purchase": { 
            "id": 1011,
            "name": "1 month",
            "price": 6.99
        }
    }
    )json";

    const char jsonPersonPurchases[] = R"json(
    {
        "firstName": "Alexandr",
        "secondName": "Bondarenko",
        "purchases": [
            { "id": 1011, "price": 6.99, "name": "1 month" },
            { "id": 1044, "price": 14.99, "name": "6 month" },
            { "id": 1077, "price": 21.65, "name": "12 month" }
        ],
        "userPicture": "\x10 €ÿ€@ \x10"
    }
    )json";


    Person person;
    printf("Person [src]: \n%s", JS::serializeStruct(person).c_str());

    //JS::ParseContext parseContext(jsonPerson);
    //JS::ParseContext parseContext(jsonPersonPurchase);
    JS::ParseContext parseContext(jsonPersonPurchases);

    if (parseContext.parseTo(person) != JS::Error::NoError) {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return;
    }

    printf("\n");
    printf("\n");
    printf("Person [parsed]: \n%s", JS::serializeStruct(person).c_str());
}



    
int main() {
    
    std::vector<uint8_t> vec{ 16, 32, 128, 255, 128, 64, 32, 16 };
    auto vecToStr = std::string(vec.begin(), vec.end());
    
    std::string str = "\x10 €ÿ€@ \x10"; // 16 32 64 128 255 128 64 32 16
    auto strToVec = std::vector<uint8_t>(str.begin(), str.end());

    TestParseScalars();
    
    //Js::TestPrintJson();


    printf("\n");
    std::this_thread::sleep_for(std::chrono::milliseconds{8'000});
    //system("pause");
    return 0;
}