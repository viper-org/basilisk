// Copyright 2025 solar-mist

#ifndef BASILISK_COMPILER_UTIL_PROCESS_H
#define BASILISK_COMPILER_UTIL_PROCESS_H 1

#include <filesystem>
#include <format>
#include <string>

#ifdef WIN32
#include <Windows.h>
#include <shlobj_core.h>
#endif

namespace util
{
    static inline std::string FindExecLinker()
    {
#ifdef WIN32
        /*PWSTR szPath;
        SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, CSIDL_PROGRAM_FILESX86, NULL, &szPath);
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, szPath, lstrlenW(szPath), NULL, 0, NULL, NULL);
        std::string converted = std::string(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, szPath, lstrlenW(szPath), converted.data(), converted.size(), NULL, NULL);
        CoTaskMemFree(szPath);
        
        std::filesystem::path path = converted;
        path /= "Microsoft Visual Studio";
        path /= "Installer";
        path /= "vswhere.exe";*/
        return "C:\\\"Program Files\"\\\"Microsoft Visual Studio\"\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat -arch=amd64 && link.exe";
#else
        return "gcc";
#endif
    }

    static inline std::string FindLibLinker()
    {
#ifdef WIN32
        return "C:\\\"Program Files\"\\\"Microsoft Visual Studio\"\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat -arch=amd64 && lib.exe";
#else
        return "ar";
#endif
    }

    static inline std::string EncodeExecLinkCommand(std::string linker, std::string outputFile, std::string inputFiles)
    {
#ifdef WIN32
		return std::format("{} {} kernel32.lib user32.lib /SUBSYSTEM:CONSOLE /OUT:{}.exe /ENTRY:_start", linker, inputFiles, outputFile);
#else
		return std::format("{} -o {} {}", linker, outputFile, inputFiles);
#endif
    }

    static inline std::string EncodeLibLinkCommand(std::string linker, std::string outputFile, std::string inputFiles)
    {
#ifdef WIN32
		return std::format("{} /OUT:{} {}", linker, outputFile, inputFiles);
#else
        return std::format("{} -rcs {} {}", linker, outputFile, inputFiles);
#endif
    }


    static inline std::string AppendLibrary(std::string& concat, std::filesystem::path library)
    {
#ifdef WIN32
        concat += " " + library.string();
        return concat;
#else
        concat += " -L " + library.parent_path().string();
		concat += " -l:" + library.filename().string();
        return concat;
#endif
    }
}

#endif // BASILISK_COMPILER_UTIL_PROCESS_H