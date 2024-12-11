// ParentProcess.cpp
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

struct ChildProcessInfo {
    PROCESS_INFORMATION procInfo;
    bool handleSaved;
};

std::vector<ChildProcessInfo> childProcesses;

void LaunchChildProcess(bool saveHandle) {
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    WCHAR childProcessPath[] = L"D:\\uni\\5sem\\osisp\\parent\\x64\\Debug\\child.exe";

    // Запуск процесса-потомка
    if (!CreateProcess(
        childProcessPath,
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        std::wcerr << L"Не удалось запустить процесс-потомка.\n";
        return;
    }

    // Сохранение информации о процессе
    ChildProcessInfo info;
    info.procInfo = pi;
    info.handleSaved = saveHandle;
    childProcesses.push_back(info);

    // Если дескриптор не сохраняется, сразу его закрываем
    if (!saveHandle) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    std::wcout << L"Процесс-потомок запущен. PID: " << pi.dwProcessId << L"\n";
}

void DisplayProcesses() {
    std::wcout << L"Список запущенных процессов:\n";
    for (size_t i = 0; i < childProcesses.size(); ++i) {
        DWORD exitCode;
        if (GetExitCodeProcess(childProcesses[i].procInfo.hProcess, &exitCode)) {
            std::wcout << L"[" << i << L"] PID: " << childProcesses[i].procInfo.dwProcessId
                << L" | Состояние: " << ((exitCode == STILL_ACTIVE) ? L"Активен" : L"Завершен")
                << L" | Дескриптор сохранен: " << (childProcesses[i].handleSaved ? L"Да" : L"Нет") << L"\n";
        }
    }
}

void CloseHandles() {
    for (auto& proc : childProcesses) {
        DWORD exitCode;
        if (GetExitCodeProcess(proc.procInfo.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
            CloseHandle(proc.procInfo.hProcess);
            CloseHandle(proc.procInfo.hThread);
            proc.handleSaved = false;
            std::wcout << L"Дескрипторы процесса с PID " << proc.procInfo.dwProcessId << L" закрыты.\n";
        }
    }
}

int wmain() {
    while (true) {
        std::wcout << L"\nМеню:\n";
        std::wcout << L"1. Запустить процесс-потомок (сохранить дескриптор)\n";
        std::wcout << L"2. Запустить процесс-потомок (не сохранять дескриптор)\n";
        std::wcout << L"3. Отобразить состояние процессов\n";
        std::wcout << L"4. Закрыть дескрипторы завершенных процессов\n";
        std::wcout << L"5. Выход\n";
        std::wcout << L"Выберите опцию: ";

        int choice;
        std::wcin >> choice;

        switch (choice) {
        case 1:
            LaunchChildProcess(true);
            break;
        case 2:
            LaunchChildProcess(false);
            break;
        case 3:
            DisplayProcesses();
            break;
        case 4:
            CloseHandles();
            break;
        case 5:
            // Закрытие всех открытых дескрипторов перед выходом
            for (auto& proc : childProcesses) {
                if (proc.handleSaved) {
                    CloseHandle(proc.procInfo.hProcess);
                    CloseHandle(proc.procInfo.hThread);
                }
            }
            return 0;
        default:
            std::wcout << L"Некорректный выбор. Попробуйте снова.\n";
            break;
        }
    }
}
