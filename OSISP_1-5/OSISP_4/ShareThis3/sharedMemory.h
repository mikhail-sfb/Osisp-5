// SharedMemory.h
#pragma once
#include <windows.h>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::wstring& name, size_t size);
    ~SharedMemory();

    void* getData();

    // �������������
    HANDLE getReadersMutex();
    HANDLE getWritersMutex();
    HANDLE getNoReadersEvent();
    HANDLE getTurnstile();
    HANDLE getWritersCountMutex();

    // ��������
    int getActiveReaders();
    void incrementActiveReaders();
    void decrementActiveReaders();

    void incrementWaitingWriters();
    void decrementWaitingWriters();
    int getWaitingWriters();

    // ���������� ��������
    void setStopFlag(bool value);
    bool getStopFlag();

private:
    HANDLE hMapFile;
    void* pBuf;

    // �������������
    HANDLE hReadersMutex;
    HANDLE hWritersMutex;
    HANDLE hNoReadersEvent;
    HANDLE hTurnstile;
    HANDLE hWritersCountMutex;

    // ��������
    int activeReaders;
    int waitingWriters;

    // ���� ���������
    volatile bool stopFlag;

    // ��� ����������� ������
    std::wstring sharedName;
};
