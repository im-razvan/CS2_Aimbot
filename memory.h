// from cazz's GitHub, a bit modified
// works well for a demo

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string_view>

using namespace std;

class Memory
{
private:
    uintptr_t processId = 0;
    void* processHandle = nullptr;
    bool processFound = false;

public:
    Memory(const string_view processName) noexcept
    {
        ::PROCESSENTRY32 entry = { };
        entry.dwSize = sizeof(::PROCESSENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        processFound = false;
        while (::Process32Next(snapShot, &entry))
        {
            if (!processName.compare(entry.szExeFile))
            {
                processId = entry.th32ProcessID;

                // should hijack the handle instead
                processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                processFound = true;
                break;
            }
        }

        if (snapShot)
            ::CloseHandle(snapShot);
    }

    ~Memory()
    {
        if (processHandle)
            ::CloseHandle(processHandle);
    }

    bool IsValid() const noexcept
    {
        return processFound;
    }

    const uintptr_t GetModuleAddress(const string_view moduleName) const noexcept
    {
        ::MODULEENTRY32 entry = { };
        entry.dwSize = sizeof(::MODULEENTRY32);

        const auto snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);

        uintptr_t result = 0;

        while (::Module32Next(snapShot, &entry))
        {
            if (!moduleName.compare(entry.szModule))
            {
                result = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
                break;
            }
        }

        if (snapShot)
            ::CloseHandle(snapShot);

        return result;
    }

    template <typename T>
    constexpr const T Read(const uintptr_t& address) const noexcept
    {
        T value = { };
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
        return value;
    }
};