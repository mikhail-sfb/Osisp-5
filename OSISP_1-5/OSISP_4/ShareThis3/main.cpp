// Main.cpp
#include <windows.h>
#include <iostream>
#include "SharedMemory.h"
#include "ReaderThread.h"
#include "WriterThread.h"

HANDLE hConsoleMutex = NULL;

int main() {
    const int numReaders = 2;
    const int numWriters = 4;
    const size_t sharedMemorySize = sizeof(int);

    try {
        hConsoleMutex = CreateMutex(NULL, FALSE, NULL);
        if (hConsoleMutex == NULL) {
            std::cerr << "Failed to create console mutex." << std::endl;
            return 1;
        }

        // ������� ����������� ������
        SharedMemory sharedMemory(L"MySharedMemory", sharedMemorySize);

        // �������������� ������
        int* data = (int*)sharedMemory.getData();
        *data = 0;

        // ������� ������� ��� ������������ �������
        HANDLE* readerThreads = new HANDLE[numReaders];
        HANDLE* writerThreads = new HANDLE[numWriters];

        // ������� ������-��������
        for (int i = 0; i < numReaders; ++i) {
            readerThreads[i] = CreateThread(
                NULL,
                0,
                ReaderThread,
                &sharedMemory,
                0,
                NULL
            );

            if (readerThreads[i] == NULL) {
                std::cerr << "Failed to create reader thread." << std::endl;
                return 1;
            }
        }

        // ������� ������-��������
        for (int i = 0; i < numWriters; ++i) {
            writerThreads[i] = CreateThread(
                NULL,
                0,
                WriterThread,
                &sharedMemory,
                0,
                NULL
            );

            if (writerThreads[i] == NULL) {
                std::cerr << "Failed to create writer thread." << std::endl;
                return 1;
            }
        }

        // ��������� ������ �� 10 ������
        Sleep(15000);

        // ������������� ���� ���������
        sharedMemory.setStopFlag(true);

        // ������� ���������� �������
        WaitForMultipleObjects(numReaders, readerThreads, TRUE, INFINITE);
        WaitForMultipleObjects(numWriters, writerThreads, TRUE, INFINITE);

        // ��������� ����������� �������
        for (int i = 0; i < numReaders; ++i) {
            CloseHandle(readerThreads[i]);
        }

        for (int i = 0; i < numWriters; ++i) {
            CloseHandle(writerThreads[i]);
        }

        // ����������� �������
        CloseHandle(hConsoleMutex);
        delete[] readerThreads;
        delete[] writerThreads;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime error occurred: " << e.what() << std::endl;
        if (hConsoleMutex) CloseHandle(hConsoleMutex);
        return 1;
    }

    return 0;
}
