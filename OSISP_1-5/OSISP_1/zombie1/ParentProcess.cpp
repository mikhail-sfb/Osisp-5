#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <sstream>

#pragma comment(lib, "user32.lib")

// Global variables
HINSTANCE hInst;
HWND hMainWnd, hCreateBtn, hCloseBtn, hReleaseBtn, hUpdateBtn, hTerminateBtn, hHandleCountLabel, hProcessList;
std::vector<PROCESS_INFORMATION> childProcesses;
std::vector<bool> handleClosed; // Track whether handles are closed
int processCount = 0;           // Track the number of processes created

// Function prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void UpdateHandleCount();
void CreateChildProcess();
void CloseSelectedProcess();
void ReleaseProcessHandles(int processIndex);
void TerminateSelectedProcess();

int APIENTRY _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    hInst = hInstance;

    // Register the window class
    const wchar_t CLASS_NAME[] = L"ZombieProcessSimulator";
    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the main window
    hMainWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Zombie Process Simulator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInst,
        NULL
    );

    if (hMainWnd == NULL)
    {
        return 0;
    }

    ShowWindow(hMainWnd, nCmdShow);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void UpdateHandleCount()
{
    DWORD handleCount = 0;
    GetProcessHandleCount(GetCurrentProcess(), &handleCount);

    std::wstringstream ss;
    ss << L"Current Handle Count: " << handleCount
        << L"\nNumber of Child Processes: " << childProcesses.size();

    SetWindowText(hHandleCountLabel, ss.str().c_str());
}

void CreateChildProcess()
{
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    // Path to your child process executable
    LPCWSTR childPath = L"D:\\uni\\5sem\\osisp\\zombie1\\x64\\Debug\\Child1.exe";

    // Create the child process
    if (!CreateProcess(
        childPath,    // Application name
        NULL,         // Command line arguments
        NULL,         // Process security attributes
        NULL,         // Primary thread security attributes
        FALSE,        // Handles are not inherited
        0,            // Creation flags
        NULL,         // Use parent's environment
        NULL,         // Use parent's current directory
        &si,          // Pointer to STARTUPINFO
        &pi))         // Pointer to PROCESS_INFORMATION
    {
        MessageBox(hMainWnd, L"Failed to create child process.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Increment process count to use in listbox labeling
    processCount++;

    // Do not close the handles to simulate a zombie process
    childProcesses.push_back(pi);
    handleClosed.push_back(false); // Track if handles are still open

    // Add numbered process entry to list
    std::wstringstream ss;
    ss << L"Child Process " << processCount;
    int index = SendMessage(hProcessList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    SendMessage(hProcessList, LB_SETITEMDATA, index, (LPARAM)(childProcesses.size() - 1));

    UpdateHandleCount();
}

void CloseSelectedProcess()
{
    int selIndex = (int)SendMessage(hProcessList, LB_GETCURSEL, 0, 0);
    if (selIndex == LB_ERR)
    {
        MessageBox(hMainWnd, L"No process selected.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get the process index
    int processIndex = (int)SendMessage(hProcessList, LB_GETITEMDATA, selIndex, 0);

    // Ensure that the processIndex is valid
    if (processIndex < 0 || processIndex >= childProcesses.size())
    {
        MessageBox(hMainWnd, L"Invalid process selected.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    PROCESS_INFORMATION pi = childProcesses[processIndex];

    // Simulate the process being closed but don't close the handles yet
    // Handles are kept open to simulate a "zombie" state
    if (!handleClosed[processIndex])
    {
        MessageBox(hMainWnd, L"Process closed, but handles are still open (zombie state).", L"Info", MB_OK);
    }

    UpdateHandleCount();
}

void ReleaseProcessHandles(int processIndex)
{
    if (processIndex < 0 || processIndex >= childProcesses.size() || handleClosed[processIndex])
    {
        MessageBox(hMainWnd, L"Handles are already closed or invalid process selected.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Close the process handles
    CloseHandle(childProcesses[processIndex].hProcess);
    CloseHandle(childProcesses[processIndex].hThread);
    handleClosed[processIndex] = true;

    MessageBox(hMainWnd, L"Process handles released.", L"Info", MB_OK);

    UpdateHandleCount();
}

void TerminateSelectedProcess()
{
    int selIndex = (int)SendMessage(hProcessList, LB_GETCURSEL, 0, 0);
    if (selIndex == LB_ERR)
    {
        MessageBox(hMainWnd, L"No process selected.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Get the process index
    int processIndex = (int)SendMessage(hProcessList, LB_GETITEMDATA, selIndex, 0);

    // Ensure that the processIndex is valid
    if (processIndex < 0 || processIndex >= childProcesses.size())
    {
        MessageBox(hMainWnd, L"Invalid process selected.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    PROCESS_INFORMATION pi = childProcesses[processIndex];

    // Terminate the process and then close the handles
    TerminateProcess(pi.hProcess, 0);
    ReleaseProcessHandles(processIndex);

    // Remove from the process list
    SendMessage(hProcessList, LB_DELETESTRING, selIndex, 0);

    // Erase from the vector
    childProcesses.erase(childProcesses.begin() + processIndex);
    handleClosed.erase(handleClosed.begin() + processIndex);

    UpdateHandleCount();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Create controls
        hHandleCountLabel = CreateWindow(
            L"STATIC", L"Current Handle Count: ",
            WS_VISIBLE | WS_CHILD,
            10, 10, 480, 20,  // Increased width for label
            hwnd, NULL, hInst, NULL);

        hCreateBtn = CreateWindow(
            L"BUTTON", L"Create Process",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            10, 40, 140, 30,  // Increased button width
            hwnd, (HMENU)1, hInst, NULL);

        hCloseBtn = CreateWindow(
            L"BUTTON", L"Close Process",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            160, 40, 140, 30,  // Adjusted position and size
            hwnd, (HMENU)2, hInst, NULL);

        hReleaseBtn = CreateWindow(
            L"BUTTON", L"Release Handles",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            310, 40, 140, 30,  // Adjusted position and size
            hwnd, (HMENU)3, hInst, NULL);

        hTerminateBtn = CreateWindow(
            L"BUTTON", L"Terminate Process",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            10, 80, 140, 30,  // New line, adjusted position and size
            hwnd, (HMENU)4, hInst, NULL);

        hUpdateBtn = CreateWindow(
            L"BUTTON", L"Update",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            160, 80, 140, 30,  // Adjusted position and size
            hwnd, (HMENU)5, hInst, NULL);

        hProcessList = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"LISTBOX", NULL,
            WS_VISIBLE | WS_CHILD | LBS_STANDARD,
            10, 120, 460, 260,  // ListBox width fits window
            hwnd, NULL, hInst, NULL);

        UpdateHandleCount();
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case 1: // Create Process button
            CreateChildProcess();
            break;

        case 2: // Close Process button
            CloseSelectedProcess();
            break;

        case 3: // Release Handles button (actually closes the handles)
        {
            int selIndex = (int)SendMessage(hProcessList, LB_GETCURSEL, 0, 0);
            if (selIndex != LB_ERR)
            {
                int processIndex = (int)SendMessage(hProcessList, LB_GETITEMDATA, selIndex, 0);
                ReleaseProcessHandles(processIndex);
            }
        }
        break;

        case 4: // Terminate Process button (fully terminate and release)
            TerminateSelectedProcess();
            break;

        case 5: // Update button (refresh handle count and process count)
            UpdateHandleCount();
            break;
        }
    }
    break;

    case WM_DESTROY:
    {
        // Clean up any remaining open handles
        for (size_t i = 0; i < childProcesses.size(); ++i)
        {
            if (!handleClosed[i])
            {
                CloseHandle(childProcesses[i].hProcess);
                CloseHandle(childProcesses[i].hThread);
            }
        }
        PostQuitMessage(0);
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
