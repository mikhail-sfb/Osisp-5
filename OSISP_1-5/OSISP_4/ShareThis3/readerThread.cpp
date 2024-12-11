// ReaderThread.cpp
#include "ReaderThread.h"
#include <windows.h>
#include <iostream>

// ���������� ������� ��� �������
extern HANDLE hConsoleMutex;

DWORD WINAPI ReaderThread(LPVOID lpParam) {
    SharedMemory* sharedMemory = (SharedMemory*)lpParam;
    int* data = (int*)sharedMemory->getData();

    HANDLE hTurnstile = sharedMemory->getTurnstile();
    HANDLE hReadersMutex = sharedMemory->getReadersMutex();
    HANDLE hNoReadersEvent = sharedMemory->getNoReadersEvent();

    while (!sharedMemory->getStopFlag()) {
        // ���� ������� turnstile
        WaitForSingleObject(hTurnstile, INFINITE);
        ReleaseSemaphore(hTurnstile, 1, NULL);

        // ������ � ������ ��� ���������� �������� ���������
        WaitForSingleObject(hReadersMutex, INFINITE);

        if (sharedMemory->getActiveReaders() == 0) {
            // ���������� �������, ����� ��������������� ��������� � ������� ���������
            ResetEvent(hNoReadersEvent);
        }

        sharedMemory->incrementActiveReaders();

        ReleaseMutex(hReadersMutex);

        // ������ ������
        int value = *data;

        // �������� ����� � �������
        WaitForSingleObject(hConsoleMutex, INFINITE);
        std::cout << "Reader Thread " << GetCurrentThreadId() << " read value: " << value << std::endl;
        ReleaseMutex(hConsoleMutex);

        // ��������� ����� ������
        Sleep(1000);

        // ����� ��������� ������� ��� ���������� ��������
        WaitForSingleObject(hReadersMutex, INFINITE);

        sharedMemory->decrementActiveReaders();

        if (sharedMemory->getActiveReaders() == 0) {
            // ������������� �������, ������������, ��� ������ ��� ���������
            SetEvent(hNoReadersEvent);
        }

        ReleaseMutex(hReadersMutex);

        // ����� ����� ��������� �������
        Sleep(1000);
    }

    return 0;
}
