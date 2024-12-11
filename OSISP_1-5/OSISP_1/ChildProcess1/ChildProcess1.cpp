#include <windows.h>
#include <iostream>

int main()
{
    MessageBox(NULL, L"Дочерний процесс работает...", L"Child Process", MB_OK);
    Sleep(3000);
    MessageBox(NULL, L"Дочерний процесс завершен.", L"Child Process", MB_OK);
    return 0;
}
