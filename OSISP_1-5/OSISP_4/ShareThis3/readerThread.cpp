// ReaderThread.cpp
#include "ReaderThread.h"
#include <windows.h>
#include <iostream>

// Глобальный мьютекс для консоли
extern HANDLE hConsoleMutex;

DWORD WINAPI ReaderThread(LPVOID lpParam) {
    SharedMemory* sharedMemory = (SharedMemory*)lpParam;
    int* data = (int*)sharedMemory->getData();

    HANDLE hTurnstile = sharedMemory->getTurnstile();
    HANDLE hReadersMutex = sharedMemory->getReadersMutex();
    HANDLE hNoReadersEvent = sharedMemory->getNoReadersEvent();

    while (!sharedMemory->getStopFlag()) {
        // Ждем семафор turnstile
        WaitForSingleObject(hTurnstile, INFINITE);
        ReleaseSemaphore(hTurnstile, 1, NULL);

        // Входим в секцию для обновления счетчика читателей
        WaitForSingleObject(hReadersMutex, INFINITE);

        if (sharedMemory->getActiveReaders() == 0) {
            // Сбрасываем событие, чтобы сигнализировать писателям о наличии читателей
            ResetEvent(hNoReadersEvent);
        }

        sharedMemory->incrementActiveReaders();

        ReleaseMutex(hReadersMutex);

        // Читаем данные
        int value = *data;

        // Защищаем вывод в консоль
        WaitForSingleObject(hConsoleMutex, INFINITE);
        std::cout << "Reader Thread " << GetCurrentThreadId() << " read value: " << value << std::endl;
        ReleaseMutex(hConsoleMutex);

        // Имитируем время чтения
        Sleep(1000);

        // Вновь блокируем мьютекс для обновления счетчика
        WaitForSingleObject(hReadersMutex, INFINITE);

        sharedMemory->decrementActiveReaders();

        if (sharedMemory->getActiveReaders() == 0) {
            // Устанавливаем событие, сигнализируя, что больше нет читателей
            SetEvent(hNoReadersEvent);
        }

        ReleaseMutex(hReadersMutex);

        // Пауза перед следующим чтением
        Sleep(1000);
    }

    return 0;
}
