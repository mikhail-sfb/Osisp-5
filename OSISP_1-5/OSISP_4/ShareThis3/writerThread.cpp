// WriterThread.cpp
#include "WriterThread.h"
#include <windows.h>
#include <iostream>

// Глобальный мьютекс для консоли
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
        // Увеличиваем счетчик ожидающих писателей
        WaitForSingleObject(hWritersCountMutex, INFINITE);
        sharedMemory->incrementWaitingWriters();
        if (sharedMemory->getWaitingWriters() == 1) {
            // Первый ожидающий писатель блокирует turnstile
            WaitForSingleObject(hTurnstile, INFINITE);
        }
        ReleaseMutex(hWritersCountMutex);

        // Захватываем мьютекс писателей
        WaitForSingleObject(hWritersMutex, INFINITE);

        // Ждем, пока все читатели завершат чтение
        WaitForSingleObject(hNoReadersEvent, INFINITE);

        // Пишем данные
        *data = value;

        // Защищаем вывод в консоль
        WaitForSingleObject(hConsoleMutex, INFINITE);
        std::cout << "Writer Thread " << GetCurrentThreadId() << " wrote value: " << value << std::endl;
        ReleaseMutex(hConsoleMutex);

        value++;

        // Имитируем время записи
        Sleep(1500);

        // Освобождаем мьютекс писателей
        ReleaseMutex(hWritersMutex);

        // Уменьшаем счетчик ожидающих писателей
        WaitForSingleObject(hWritersCountMutex, INFINITE);
        sharedMemory->decrementWaitingWriters();
        if (sharedMemory->getWaitingWriters() == 0) {
            // Последний писатель освобождает turnstile
            ReleaseSemaphore(hTurnstile, 1, NULL);
        }
        ReleaseMutex(hWritersCountMutex);

        // Пауза перед следующей записью
        Sleep(1000);
    }

    return 0;
}
