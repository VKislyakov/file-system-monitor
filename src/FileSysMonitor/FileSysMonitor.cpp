#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <string>

void WatchDirectory(LPCTSTR, BOOL watchSub = FALSE);
void RunWatchFile(LPCTSTR lpDir);
void RunWatchFile2(LPCTSTR lpDir, HANDLE hDirectory);
void myWatcher(LPCTSTR lpDir);

using namespace std;

int main()
{
    wstring dir;

    wcout << L"Dir: ";
    wcin >> dir;

    LPCTSTR str = dir.data();
    myWatcher(str);

}


void printChangeInfo(const wstring& fileName, DWORD action);

void myWatcher(LPCTSTR lpDir) {
    wcout << L"Run Watch File Changes in " << lpDir << endl;

    HANDLE hDirectory = CreateFileW(lpDir, FILE_LIST_DIRECTORY | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (hDirectory == INVALID_HANDLE_VALUE){
        wcout << L" INVALID_HANDLE_VALUE  " << endl;
        return;
    }

    OVERLAPPED olap;
    memset(&olap, 0, sizeof(olap));
    olap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!olap.hEvent) {
        wcout << L" CreateEvent problem " << endl;
        return;
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    DWORD bytesReturned = 0;
    DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME;
    BOOL bWatchSubtree = FALSE;

    while(true) {
        if (hDirectory == INVALID_HANDLE_VALUE) {
            wcout << L" INVALID_HANDLE_VALUE  " << endl;
            return;
        }

       auto readDirChngResult = ReadDirectoryChangesW( hDirectory,
            &buffer, sizeof(buffer), bWatchSubtree, filter, &bytesReturned, &olap, NULL);

       if (!readDirChngResult) {
           wcout << L" ReadDirectoryChangesW problem " << endl;
           return;
       }

       DWORD waitTimeout = 10000; // 10 second = 10000 millisecond
       DWORD dwWaitStatus = WaitForSingleObject(olap.hEvent, waitTimeout);
       switch (dwWaitStatus)
       {
           case WAIT_OBJECT_0:
           {
               if (!GetOverlappedResult(hDirectory, &olap, &bytesReturned, TRUE)) {
                   wcout << L" GetOverlappedResult failed " << endl;
                   return;
               }
               if (bytesReturned == 0) {
                    wcout << L" GetOverlappedResult failed " << endl;
                    return;
               }
                   
               FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
               do
               {
                   if (fni->Action != 0)
                   {
                       wstring fileName(fni->FileName, fni->FileNameLength / sizeof(wchar_t));
                       //wcout << L" Change file/dir: " << fileName << endl;
                       printChangeInfo(fileName, fni->Action);
                   }

                   if (fni->NextEntryOffset == 0)
                       break;

                   fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset);
               } while (true);

               break;
           }
           case WAIT_FAILED:
               wcout << L" WAIT_FAILED " << endl;
               return;
           case WAIT_TIMEOUT:
               printf("\nNo changes in the timeout period.\n");
               break;
           default:
               printf("\n ERROR: Unhandled dwWaitStatus.\n");
               return;
       }
    }
}

void printChangeInfo(const wstring& fileName, DWORD action) {
    if (action == FILE_ACTION_ADDED){
        wcout << L"File monitoring, add: " << fileName << endl;
    }
    else if (action == FILE_ACTION_REMOVED){
        wcout << L"File monitoring, delete: " << fileName << endl;
    }
    else if (action == FILE_ACTION_MODIFIED){
        wcout << L"File monitoring, modify: " << fileName << endl;
    }
    else if (action == FILE_ACTION_RENAMED_OLD_NAME){
        wcout << L"File monitoring, rename(old): " << fileName << endl;
    }
    else if (action == FILE_ACTION_RENAMED_NEW_NAME) {
        wcout << L"File monitoring, rename(new): " << fileName << endl;
    }
    else {
        wcout << L"File monitoring, undefined change: " << fileName << endl;
    }
}