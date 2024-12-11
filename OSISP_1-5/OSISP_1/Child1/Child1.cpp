#include <windows.h>
#include <iostream>

class ProcessWatcher {
public:
    static int RunChildProcess() {
        HANDLE exitEventHandle = OpenOrCreateExitEvent();
        if (exitEventHandle == NULL) {
            ShowError(L"Failed to create or open exit event.");
            return 1;
        }

        DisplayMessage(L"Child process is running...");

        // Set up a timer for 3 seconds
        HANDLE timerHandle = CreateTimer();
        if (timerHandle == NULL) {
            ShowError(L"Failed to create timer.");
            CloseHandle(exitEventHandle);
            return 1;
        }

        // Wait for either the timer or the exit event
        HANDLE eventHandles[] = { timerHandle, exitEventHandle };
        DWORD waitResult = WaitForMultipleObjects(2, eventHandles, FALSE, INFINITE);

        ProcessWaitResult(waitResult);

        // Clean up
        CloseHandle(timerHandle);
        CloseHandle(exitEventHandle);

        return 0;
    }

private:
    static HANDLE OpenOrCreateExitEvent() {
        HANDLE hExitEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"Global\\ChildExitEvent");
        if (hExitEvent == NULL) {
            hExitEvent = CreateEvent(NULL, TRUE, FALSE, L"Global\\ChildExitEvent");
        }
        return hExitEvent;
    }

    static HANDLE CreateTimer() {
        HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (hTimer != NULL) {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = -30000000LL; // 3 seconds in 100-nanosecond intervals
            SetWaitableTimer(hTimer, &dueTime, 0, NULL, NULL, 0);
        }
        return hTimer;
    }

    static void ShowError(const wchar_t* message) {
        MessageBox(NULL, message, L"Error", MB_OK);
    }

    static void DisplayMessage(const wchar_t* message) {
        MessageBox(NULL, message, L"Child Process", MB_OK);
    }

    static void ProcessWaitResult(DWORD waitResult) {
        if (waitResult == WAIT_OBJECT_0) {
            // Timer expired
            DisplayMessage(L"Child process completed.");
        }
        else if (waitResult == WAIT_OBJECT_0 + 1) {
            // Exit event was signaled
            DisplayMessage(L"Child process is exiting upon request.");
        }
    }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    return ProcessWatcher::RunChildProcess();
}