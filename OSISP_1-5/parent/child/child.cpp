// ChildProcess.cpp
#include <windows.h>
#include <iostream>

int wmain() {
    std::wcout << L"Процесс-потомок запущен. PID: " << GetCurrentProcessId() << L"\n";
    std::wcout << L"Выберите способ завершения:\n";
    std::wcout << L"1. Завершить по команде\n";
    std::wcout << L"2. Завершить по времени (10 секунд)\n";
    std::wcout << L"3. Ожидать завершения через Диспетчер задач\n";
    std::wcout << L"Выберите опцию: ";

    int choice;
    std::wcin >> choice;

    switch (choice) {
    case 1:
        std::wcout << L"Нажмите Enter для завершения процесса.\n";
        std::wcin.ignore();
        std::wcin.get();
        break;
    case 2:
        std::wcout << L"Процесс завершится через 10 секунд...\n";
        Sleep(10000);
        break;
    case 3:
        std::wcout << L"Процесс ожидает завершения через Диспетчер задач.\n";
        while (true) {
            Sleep(1000);
        }
        break;
    default:
        std::wcout << L"Некорректный выбор. Процесс завершится.\n";
        break;
    }

    std::wcout << L"Процесс-потомок завершается.\n";
    return 0;
}
