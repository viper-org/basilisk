// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_UTIL_PROCESS_H
#define BASILISK_COMPILER_UTIL_PROCESS_H 1

#include <string>

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace util
{
    static inline void ExecuteProcess(std::string path)
    {
        std::string command = path;
        
#ifdef WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
		CreateProcess(path.c_str(), path.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#else
		execl(path.c_str(), path.c_str(), args..., nullptr);
#endif
    }
}

#endif // BASILISK_COMPILER_UTIL_PROCESS_H