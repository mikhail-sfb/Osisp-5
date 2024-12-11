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

        // Создаем разделяемую память
        SharedMemory sharedMemory(L"MySharedMemory", sharedMemorySize);

        // Инициализируем данные
        int* data = (int*)sharedMemory.getData();
        *data = 0;

        // Создаем массивы для дескрипторов потоков
        HANDLE* readerThreads = new HANDLE[numReaders];
        HANDLE* writerThreads = new HANDLE[numWriters];

        // Создаем потоки-читатели
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

        // Создаем потоки-писатели
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

        // Запускаем таймер на 10 секунд
        Sleep(15000);

        // Устанавливаем флаг остановки
        sharedMemory.setStopFlag(true);

        // Ожидаем завершения потоков
        WaitForMultipleObjects(numReaders, readerThreads, TRUE, INFINITE);
        WaitForMultipleObjects(numWriters, writerThreads, TRUE, INFINITE);

        // Закрываем дескрипторы потоков
        for (int i = 0; i < numReaders; ++i) {
            CloseHandle(readerThreads[i]);
        }

        for (int i = 0; i < numWriters; ++i) {
            CloseHandle(writerThreads[i]);
        }

        // Освобождаем ресурсы
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
