#include "injector.h"

cap::Injector::Injector(std::string processName, std::string dllPath)
{
    this->processName = processName;
    this->dllPath = dllPath;
}

BOOL cap::Injector::Inject()
{
    if (!this->GetProcessHandle(this->hProcess)) {
        Error::SetLastError(1);
        return false;
    }

    if (!this->AllocateMemory(this->pBaseAddress)) {
        Error::SetLastError(2);
        return false;
    }

    if (!this->CreateRemoteThread(this->hThreadHandle)) {
        Error::SetLastError(3);
        return false;
    }

    if (this->WaitForThreadExit()) {
        Error::SetLastError(4);
        return false;
    }

    DWORD hDll;
    if (!GetExitCodeThread(this->hThreadHandle, &hDll)) {
        Error::SetLastError(5);
        return false;
    }

    if (hDll == 0x00000000) {
        Error::SetLastError(6);
        return false;
    }

    this->Close();
	return true;
}

BOOL cap::Injector::GetProcessHandle(HANDLE& hProcess)  // https://stackoverflow.com/a/865201
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            std::wstring cmp(this->processName.begin(), this->processName.end());
            if (!cmp.compare(entry.szExeFile))
            {
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            }
        }
    }

    CloseHandle(snapshot);

    return hProcess == INVALID_HANDLE_VALUE ? FALSE : TRUE;
}

BOOL cap::Injector::AllocateMemory(LPVOID& baseAddress)
{
    baseAddress = VirtualAllocEx(this->hProcess, NULL, this->dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!baseAddress) {
        return FALSE;
    }

    BOOL ret = WriteProcessMemory(this->hProcess, baseAddress, this->dllPath.c_str(), this->dllPath.length() + 1, NULL);

    return ret;
}

BOOL cap::Injector::CreateRemoteThread(HANDLE& threadHandle)
{
    void* LoadLibraryA = (void*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    threadHandle = ::CreateRemoteThread(this->hProcess, NULL, 0,(LPTHREAD_START_ROUTINE)LoadLibraryA, this->pBaseAddress, 0, NULL);

    return threadHandle == NULL ? FALSE : TRUE;
}

DWORD cap::Injector::WaitForThreadExit()
{
    return WaitForSingleObject(this->hThreadHandle, INFINITE);
}

VOID cap::Injector::Close()
{
    CloseHandle(this->hThreadHandle);
    VirtualFreeEx(this->hProcess, this->pBaseAddress, sizeof(dllPath), MEM_RELEASE);
    CloseHandle(this->hThreadHandle);
    CloseHandle(this->hProcess);
}

void cap::Error::SetLastError(int code)
{
    code = code;
}

int cap::Error::GetLastError()
{
    return code;
}
