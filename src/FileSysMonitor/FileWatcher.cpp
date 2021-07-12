#include "FileWatcher.h"
#include <stdexcept>

FileWacther::FileWacther(const wstring& dir){
    m_dir = dir;
    LPCTSTR lpDir = m_dir.data();
    m_Directory = CreateFileW(lpDir, FILE_LIST_DIRECTORY | GENERIC_READ,
    FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (m_Directory == INVALID_HANDLE_VALUE){
        throw RunTimeError("Error: INVALID_DIR_HANDLE_VALUE");
    }
}

FileWacther::~FileWacther(){
    CloseHandle(m_Directory);
}

void FileWacther::setTimeout(unsigned long millisecond){
    m_timeout = millisecond;
}

std::optional<VecStr> FileWacther::nextChanges(){
    OVERLAPPED olap;
    VecStr result;

    try{
        memset(&olap, 0, sizeof(olap));
        olap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!olap.hEvent)
            throw std::runtime_error(" CreateEvent problem ");

        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        DWORD bytesReturned = 0;
        DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME;
        BOOL bWatchSubtree = FALSE;

        auto readDirChngResult = ReadDirectoryChangesW( hDirectory,
            &buffer, sizeof(buffer), bWatchSubtree, filter, &bytesReturned, &olap, NULL);
        if (!readDirChngResult)
            throw std::runtime_error(" ReadDirectoryChangesW problem ");

        DWORD dwWaitStatus = WaitForSingleObject(olap.hEvent, m_timeout);
        if (dwWaitStatus == WAIT_OBJECT_0) {
            if (!GetOverlappedResult(hDirectory, &olap, &bytesReturned, TRUE))
                throw std::runtime_error(" GetOverlappedResult failed ");
            if (bytesReturned == 0)
                throw std::runtime_error(" Failed: bytesReturned is empty ");

            FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
            result = getChangesFromNotify(fni);
        }
        else if (dwWaitStatus == WAIT_FAILED)
            throw std::runtime_error(" WAIT_FAILED ");
        else if (dwWaitStatus == WAIT_TIMEOUT)
            throw std::runtime_error("No changes in the timeout period.");
        else
            throw std::runtime_error("ERROR: Unhandled dwWaitStatus.");
    }
    catch(std::exception e){
        m_lastErrorMessage = e.what();
    }
    catch(...){
        m_lastErrorMessage = "Undefined error";
    }

    CloseHandle(lap.hEvent);
    return result;
}


VecStr FileWacther::getChangesFromNotify(FILE_NOTIFY_INFORMATION* fni){
    VecStr result;

    do
    {
        if (fni->Action != 0)
        {
            wstring fileName(fni->FileName, fni->FileNameLength / sizeof(wchar_t));
            result.push_back(getChangeMessageByAction(fni->Action) + fileName);
        }

        if (fni->NextEntryOffset == 0)
            break;

        fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset);
    } while (true);

    return result;
}

wstring FileWacther::getChangeMessageByAction(DWORD action) {

    // TODO time??

    if (action == FILE_ACTION_ADDED)
        return L"File monitoring, add: ";
    else if (action == FILE_ACTION_REMOVED)
        return L"File monitoring, delete: ";
    else if (action == FILE_ACTION_MODIFIED)
        return L"File monitoring, modify: ";
    else if (action == FILE_ACTION_RENAMED_OLD_NAME)
        return L"File monitoring, rename(old): ";
    else if (action == FILE_ACTION_RENAMED_NEW_NAME) 
        return L"File monitoring, rename(new): ";

    return L"File monitoring, undefined change: ";
}