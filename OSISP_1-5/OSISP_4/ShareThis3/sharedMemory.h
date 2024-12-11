// SharedMemory.h
#pragma once
#include <windows.h>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::wstring& name, size_t size);
    ~SharedMemory();

    void* getData();

    // Синхронизация
    HANDLE getReadersMutex();
    HANDLE getWritersMutex();
    HANDLE getNoReadersEvent();
    HANDLE getTurnstile();
    HANDLE getWritersCountMutex();

    // Счетчики
    int getActiveReaders();
    void incrementActiveReaders();
    void decrementActiveReaders();

    void incrementWaitingWriters();
    void decrementWaitingWriters();
    int getWaitingWriters();

    // Управление потоками
    void setStopFlag(bool value);
    bool getStopFlag();

private:
    HANDLE hMapFile;
    void* pBuf;

    // Синхронизация
    HANDLE hReadersMutex;
    HANDLE hWritersMutex;
    HANDLE hNoReadersEvent;
    HANDLE hTurnstile;
    HANDLE hWritersCountMutex;

    // Счетчики
    int activeReaders;
    int waitingWriters;

    // Флаг остановки
    volatile bool stopFlag;

    // Имя разделяемой памяти
    std::wstring sharedName;
};
