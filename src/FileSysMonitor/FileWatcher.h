#pragma once

#include <optional>
#include <string>
#include <vector>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

using VecStr = std::vector<std::wstring>;

class FileWacther{
public:
    FileWacther(wstring dir);
    ~FileWacther();

    std::optional<VecStr> nextChanges();

    void setTimeout(unsigned long millisecond);

    wstring lastErrorMessage();

private:
    VecStr getChangesFromNotify(FILE_NOTIFY_INFORMATION* fni);    

    wstring getChangeMessageByAction(DWORD action);

private:
    unsigned long m_timeout = 1000; // 1 second default

    string m_lastErrorMessage;

    HANDLE m_Directory;

    wstring m_dir;
};