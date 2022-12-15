#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

HANDLE CanWrite;
HANDLE CanRead;
HANDLE Mutex;

CONST DWORD WritersCount = 3;
CONST DWORD ReadersCount = 4;

DWORD count = 0;

DWORD ActiveReaders = 0;
DWORD WaitingReaders = 0;
DWORD WaitingWriters = 0;

BOOL WriterActive = 0;

VOID StartRead(VOID)
{
    InterlockedIncrement(&WaitingReaders);
    if (WriterActive != 0 || (WaitForSingleObject(CanWrite, 0) == WAIT_OBJECT_0 && WaitingWriters > 0))
    {
        WaitForSingleObject(CanRead, INFINITE);
    }
    WaitForSingleObject(Mutex, INFINITE);
    InterlockedDecrement(&WaitingReaders);
    InterlockedIncrement(&ActiveReaders);
    SetEvent(CanRead);
    ReleaseMutex(Mutex);
}

VOID StopRead(VOID)
{
    InterlockedDecrement(&ActiveReaders);
    if (ActiveReaders == 0)
    {
        ResetEvent(CanWrite);
        SetEvent(CanRead);
    }
}

VOID StartWrite(VOID)
{
    InterlockedIncrement(&WaitingWriters);
    if (WriterActive != 0 || ActiveReaders > 0)
    {
        WaitForSingleObject(CanWrite, INFINITE);
    }

    InterlockedDecrement(&WaitingWriters);
    WriterActive = 1;
}

VOID StopWrite(VOID)
{
    WriterActive = 0;
    if (WaitingReaders > 0)
    {
        SetEvent(CanRead);
    }
    else
    {
        SetEvent(CanWrite);
    }
}

DWORD __stdcall RunWriter(CONST LPVOID lpParams)
{
    while (1)
    {
        Sleep(2000 + rand() % 2000);
        StartWrite();
        count++;
        printf("WRITER %d: \t\t%d\n", (int)lpParams, count);
        StopWrite();
    }
}

DWORD __stdcall RunReader(CONST LPVOID lpParams)
{
    while (1)
    {
        Sleep(2000 + rand() % 2000);
        StartRead();
        printf("READER %d: %d\n", (int)lpParams, count);
        StopRead();
    }
}

int main(void)
{
    HANDLE ReadersThreads[ReadersCount];
    HANDLE WritersThreads[WritersCount];

    if ((Mutex = CreateMutex(NULL, 0, NULL)) == NULL)
    {
        perror("CreateMutex");
        exit(1);
    }

    if ((CanRead = CreateEvent(NULL, 0, 0, NULL)) == NULL)
    {
        perror("ReaderEvent");
        exit(1);
    }

    if ((CanWrite = CreateEvent(NULL, 0, 0, NULL)) == NULL)
    {
        perror("WriterEvent");
        exit(1);
    }

    for (int i = 0; i < ReadersCount; i++)
    {
        if ((ReadersThreads[i] = CreateThread(NULL, 0, RunReader, (LPVOID)i, 0, NULL)) == NULL)
        {
            perror("CreateThread");
            exit(1);
        }
    }

    for (int i = 0; i < WritersCount; i++)
    {
        if ((WritersThreads[i] = CreateThread(NULL, 0, RunWriter, (LPVOID)i, 0, NULL)) == NULL)
        {
            perror("CreateThread");
            exit(1);
        }
    }

    WaitForMultipleObjects(ReadersCount, ReadersThreads, 1, INFINITE);
    WaitForMultipleObjects(WritersCount, WritersThreads, 1, INFINITE);

    CloseHandle(Mutex);
    CloseHandle(CanRead);
    CloseHandle(CanWrite);

    return 0;
}
