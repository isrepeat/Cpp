// builded with /c++20 (for use std::format)
#include <array>
#include <format>
#include <cassert>
#include <algorithm>
#include <functional>
#include "Printer.h"

namespace Js {    
    void PrintJSON(ISerialize* serializableObject) {
        auto jsonStr = serializableObject->ToString(2);
        std::replace(jsonStr.begin(), jsonStr.end(), '(', '{');
        std::replace(jsonStr.begin(), jsonStr.end(), ')', '}');
        auto res = std::format("{{\n{}\n}}", jsonStr);
        printf(res.c_str());
        printf("\n\n");
    }


    #define PADDING "{1:>{0}}"
    const int padding = 2;

    std::string ISerialize::scalarFormat(int nestingLevel, const std::string& fieldName, bool isEndField) {
        assert(nestingLevel > 0);
        return std::format(PADDING"\"{2}\": {{}}{3}", padding * nestingLevel, "", fieldName, isEndField ? "" : ", \n");
    }
    std::string ISerialize::objectFormat(int nestingLevel, const std::string& fieldName, bool isEndField) {
        assert(nestingLevel > 0);
        return std::format(
            PADDING"\"{2}\": (\n"
            "{{}}\n"
            PADDING"){3}", padding * nestingLevel, "", fieldName, isEndField ? "" : ", \n");
    }
    std::string ISerialize::arrayFormat(int nestingLevel, const std::string& fieldName, bool isEndField) {
        assert(nestingLevel > 0);
        return std::format(
            PADDING"\"{2}\": [\n"
            "{{}}\n"
            PADDING"]{3}", padding * nestingLevel, "", fieldName, isEndField ? "" : ", \n");
    }


    std::string ISerialize::scalarEmbeddedFromat(int nestingLevel, bool isEndField) {
        assert(nestingLevel > 0);
        return std::format(PADDING"{{}}{2}", padding * nestingLevel, "", isEndField ? "" : ", \n");
    }
    std::string ISerialize::objectEmbeddedFromat(int nestingLevel, bool isEndField) {
        assert(nestingLevel > 0);
        return  std::format(
            PADDING"(\n"
            "{{}}\n"
            PADDING"){2}", padding * nestingLevel, "", isEndField ? "" : ", \n");
    }
    std::string ISerialize::arrayEmbeddedFromat(int nestingLevel, bool isEndField) {
        assert(nestingLevel > 0);
        return std::format(
            PADDING"[\n"
            "{{}}\n"
            PADDING"]{2}", padding * nestingLevel, "", isEndField ? "" : ", \n");
    }

    



    
    template <typename T>
    struct is_std_array {
        static const bool value = false;
    };

    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N> > {
        static const bool value = true;
    };



    template <typename T, std::size_t N>
    std::string ExpandArray(int nestingLevel, const std::array<T, N>& arr) {        
        std::string(*formatFunction)(int, bool) = nullptr;

        if constexpr (is_std_array<T>::value) {
            formatFunction = &ISerialize::arrayEmbeddedFromat;
        }
        else if constexpr (std::is_base_of_v<ISerialize, T>) {
            formatFunction = &ISerialize::objectEmbeddedFromat;
        }
        else {
            formatFunction = &ISerialize::scalarEmbeddedFromat;
        }


        std::string format;
        for (int i = 0; i < N - 1; ++i) {
            format += formatFunction(nestingLevel, false);
        }
        format += formatFunction(nestingLevel, true);


        auto formatArrayHelper = [&] <std::size_t... I>(std::index_sequence<I...>) -> std::string {
            std::string result;
            if constexpr (is_std_array<T>::value) {
                result = std::vformat(format, std::make_format_args(ExpandArray(nestingLevel + 1, arr[I])...));
            }
            else if constexpr (std::is_base_of_v<ISerialize, T>) {
                result = std::vformat(format, std::make_format_args(arr[I].ToString(nestingLevel + 1)...));
            }
            else {
                result = std::vformat(format, std::make_format_args(arr[I]...));
            }
            return result;
        };
        return formatArrayHelper(std::make_index_sequence<N>());
    }

    

    // Test strutures
    struct Departments : ISerialize {
        std::string departmentName;
        int departmentId;
        int maxCount;

        std::string ToString(int nestingLevel) const override {
            return std::vformat(
                scalarFormat(nestingLevel, "departmentName") +
                scalarFormat(nestingLevel, "departmentId") +
                scalarFormat(nestingLevel, "maxCount", true),
                std::make_format_args(
                    departmentName,
                    departmentId,
                    maxCount
                ));
        }

    };


    struct Education : ISerialize {
        std::string organization;
        std::array<Departments, 2> departments;
        int duration;

        std::string ToString(int nestingLevel) const override {
            return std::vformat(
                scalarFormat(nestingLevel, "organization") +
                arrayFormat(nestingLevel, "departments") +
                scalarFormat(nestingLevel, "duration", true),
                std::make_format_args(
                    organization,
                    ExpandArray(nestingLevel + 1, departments),
                    duration
                ));
        }
    };

    struct Person : ISerialize {
        std::string firstName = "Alex";
        std::string secondName = "Cross";
        std::array<Education, 3> educations;
        std::string sex = "male";
        std::array<std::string, 3> cars = { "BMW", "Toyota", "Lancer" };
        std::array<std::array<std::string, 2>, 3> carsGroups = { 
            std::array<std::string, 2>{"BMW", "Mitsubitshi"},
            std::array<std::string, 2>{"Deo", "Lancer"},
            std::array<std::string, 2>{"Tesla", "Bugatti"} };
        int age = 37;

        std::string ToString(int nestingLevel) const override {
            assert(nestingLevel > 0);

            return std::vformat(
                scalarFormat(nestingLevel, "firstName") +
                scalarFormat(nestingLevel, "secondName") +
                arrayFormat(nestingLevel, "educations") +
                scalarFormat(nestingLevel, "sex") +
                arrayFormat(nestingLevel, "cars") +
                scalarFormat(nestingLevel, "age", true),
                std::make_format_args(
                    firstName,
                    secondName,
                    ExpandArray(nestingLevel + 1, educations),
                    sex,
                    ExpandArray(nestingLevel + 1, carsGroups),
                    age
                ));
        }
    };



    void TestPrintJson() {
        //std::string fmt_str = std::format("{:*>10}{}", "", myStr);    
        //auto testScalarFormat = scalarFormat(2, "scalarField");
        //auto testObjectFormat = objectFormat(4, "objectField");

        //auto testObjectEmbeddedFormat = "objectField = " + 
        //    objectEmbeddedFromat(4) + 
        //    objectEmbeddedFromat(4, true);

        //auto testObjectEmbeddedFormat2 = std::vformat(
        //    scalarFormat(2, "name") +
        //    objectEmbeddedFromat(2 * 2, true),
        //    std::make_format_args(
        //        "Alex",
        //        "obj_data_1\nobj_data_2\nobj_data_3"
        //    )
        //);

        //printf("testObjectEmbeddedFormat2 = \n");
        //printf(testObjectEmbeddedFormat2.c_str());
        //printf("\n\n");

        Person person;
        person.educations[0].organization = "School";
        person.educations[0].duration = 11;
        person.educations[0].departments[0].departmentName = "Middle";
        person.educations[0].departments[0].departmentId = 1001;
        person.educations[0].departments[0].maxCount = 20;
        person.educations[0].departments[1].departmentName = "High";
        person.educations[0].departments[1].departmentId = 1002;
        person.educations[0].departments[1].maxCount = 13;


        person.educations[1].organization = "University A";
        person.educations[1].duration = 4;
        person.educations[1].departments[0].departmentName = "Bacalavr";
        person.educations[1].departments[0].departmentId = 3011;
        person.educations[1].departments[0].maxCount = 25;
        //person.educations[1].departments[1].departmentName = "Magistr";
        //person.educations[1].departments[1].departmentId = 3022;
        //person.educations[1].departments[1].maxCount = 18;




        person.educations[2].organization = "University B";
        person.educations[2].duration = 6;
        person.educations[2].departments[0].departmentName = "Bacalavr";
        person.educations[2].departments[0].departmentId = 7044;
        person.educations[2].departments[0].maxCount = 25;
        person.educations[2].departments[1].departmentName = "Magistr";
        person.educations[2].departments[1].departmentId = 7048;
        person.educations[2].departments[1].maxCount = 18;

        printf("Person json:\n");
        PrintJSON(&person);
    }
}