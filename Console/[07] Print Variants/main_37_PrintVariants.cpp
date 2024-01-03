#include <Windows.h>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <array>
#include <tuple>
#include <set>
#include <Helpers/Scope.h>
#include <Helpers/Helpers.h>

#include "main_37_Printer.h"


//int g_vecItemCounter = 0;
//template<typename T>
//int& ExpandVectorType(std::vector<T> vec) {
//    return vec.at(g_vecItemCounter);
//}
//
////template<typename... Args>
////void PrintVectorHelper(const xCHAR* format, Args... args) {
////    printf(format, ExpandSomeType(args)...);
////}

//struct Visitor {
//    int operator() (int& val) {
//        std::cout << "INT \n";
//        return val;
//    }
//
//    double operator() (double& val) {
//        std::cout << "DOUBLE \n";
//        return val;
//    }
//};












int main() {
    //printf("TmpStr = %s", ReturnTmpCStr().c_str());
    //PrintVector("Args =  %d, %f, %d \n", 111, 1.2, 33);
    PrintVector("Args =  %d, %f, %d \n", 111, 3.14, 333);


    std::variant<int, double> variant;
    variant = 3.14;

    //int varINT = variant_unwrapper{ variant };
    //double varDOUBLE = variant_unwrapper{ variant };


    printf("\n\n");
    Sleep(4000);
    return 0;
}