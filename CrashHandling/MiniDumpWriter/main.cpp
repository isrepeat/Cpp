#include <iostream>
#include <Windows.h>

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        printf("arg [%d] = %s \n", i, argv[i]);
    }

    Sleep(4000);
    return 0;
}