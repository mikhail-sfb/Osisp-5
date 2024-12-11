// WriterThread.cpp
#include "WriterThread.h"
#include <windows.h>
#include <iostream>

// ���������� ������� ��� �������
extern HANDLE hConsoleMutex;

DWORD WINAPI WriterThread(LPVOID lpParam) {
    SharedMemory* sharedMemory = (SharedMemory*)lpParam;
    int* data = (int*)sharedMemory->getData();

    HANDLE hTurnstile = sharedMemory->getTurnstile();
    HANDLE hWritersMutex = sharedMemory->getWritersMutex();
    HANDLE hWritersCountMutex = sharedMemory->getWritersCountMutex();
    HANDLE hNoReadersEvent = sharedMemory->getNoReadersEvent();

    int value = 0;

    while (!sharedMemory->getStopFlag()) {
        // ����������� ������� ��������� ���������
        WaitForSingleObject(hWritersCountMutex, INFINITE);
        sharedMemory->incrementWaitingWriters();
        if (sharedMemory->getWaitingWriters() == 1) {
            // ������ ��������� �������� ��������� turnstile
            WaitForSingleObject(hTurnstile, INFINITE);
        }
        ReleaseMutex(hWritersCountMutex);

        // ����������� ������� ���������
        WaitForSingleObject(hWritersMutex, INFINITE);

        // ����, ���� ��� �������� �������� ������
        WaitForSingleObject(hNoReadersEvent, INFINITE);

        // ����� ������
        *data = value;

        // �������� ����� � �������
        WaitForSingleObject(hConsoleMutex, INFINITE);
        std::cout << "Writer Thread " << GetCurrentThreadId() << " wrote value: " << value << std::endl;
        ReleaseMutex(hConsoleMutex);

        value++;

        // ��������� ����� ������
        Sleep(1500);

        // ����������� ������� ���������
        ReleaseMutex(hWritersMutex);

        // ��������� ������� ��������� ���������
        WaitForSingleObject(hWritersCountMutex, INFINITE);
        sharedMemory->decrementWaitingWriters();
        if (sharedMemory->getWaitingWriters() == 0) {
            // ��������� �������� ����������� turnstile
            ReleaseSemaphore(hTurnstile, 1, NULL);
        }
        ReleaseMutex(hWritersCountMutex);

        // ����� ����� ��������� �������
        Sleep(1000);
    }

    return 0;
}
