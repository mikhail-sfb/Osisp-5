#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

class ProcessLauncher {
public:
    static bool LaunchExecutable(const std::wstring& exeName) {
        std::wstring exePath = GetExecutablePath(exeName);
        return CreateAndRunProcess(exePath);
    }

private:
    static std::wstring GetExecutablePath(const std::wstring& exeName) {
        wchar_t pathBuffer[MAX_PATH];
        GetModuleFileNameW(NULL, pathBuffer, MAX_PATH);
        std::wstring fullPath(pathBuffer);
        size_t lastSlashPosition = fullPath.find_last_of(L"\\/");
        return fullPath.substr(0, lastSlashPosition + 1) + exeName + L".exe";
    }

    static bool CreateAndRunProcess(const std::wstring& fullExePath) {
        STARTUPINFO startupInfo = { sizeof(startupInfo) };
        PROCESS_INFORMATION procInfo;

        BOOL result = CreateProcess(
            fullExePath.c_str(),   // ���� � exe
            NULL,                  // ��������� ������
            NULL,                  // ������������ ��������
            NULL,                  // ������������ ������
            FALSE,                 // ����������� �������� � ������ �� �����������
            0,                     // ����� ��������
            NULL,                  // ���������� ���������
            NULL,                  // ����������
            &startupInfo,          // ���������� � �������
            &procInfo              // ���������� � ��������
        );

        if (!result) {
            DisplayErrorMessage(GetLastError());
            return false;
        }

        // �������� ������������
        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
        return true;
    }

    static void DisplayErrorMessage(DWORD errorCode) {
        LPVOID messageBuffer;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            0,
            (LPWSTR)&messageBuffer,
            0,
            NULL
        );

        std::wcerr << L"������ ��� �������: " << errorCode << L". ���������: " << (LPWSTR)messageBuffer << std::endl;
        LocalFree(messageBuffer);
    }
};

int main() {
    std::vector<std::wstring> executables = { L"DataGenerator", L"Sorter", L"OutputProcess" };

    for (const auto& exeName : executables) {
        bool success = ProcessLauncher::LaunchExecutable(exeName);
        if (!success) {
            std::wcerr << L"�� ������� ��������� �������: " << exeName << std::endl;
            return 1;
        }
        else {
            std::wcout << L"������� ������� �������: " << exeName << std::endl;
        }

        // �������� ��� ������������� �������
        Sleep(500);
    }

    return 0;
}