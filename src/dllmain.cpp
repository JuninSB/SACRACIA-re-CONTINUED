#include "pch.h"

#include <tlhelp32.h>
#include <strsafe.h>

namespace
{
    static void BootstrapLog(const wchar_t* message)
    {
        wchar_t tempPath[MAX_PATH]{}, directory[MAX_PATH]{}, filePath[MAX_PATH]{};
        if (!::GetTempPathW(_countof(tempPath), tempPath)) return;
        if (FAILED(::StringCchPrintfW(directory, _countof(directory), L"%lsRepoHaxDiagnostics", tempPath)) ||
            FAILED(::StringCchPrintfW(filePath, _countof(filePath), L"%ls\\steam-bootstrap.log", directory))) return;
        ::CreateDirectoryW(directory, nullptr);
        HANDLE file = ::CreateFileW(filePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        SYSTEMTIME now{}; ::GetLocalTime(&now);
        wchar_t line[1024]{};
        if (SUCCEEDED(::StringCchPrintfW(line, _countof(line), L"%04u-%02u-%02u %02u:%02u:%02u PID=%lu %ls\r\n",
            now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, ::GetCurrentProcessId(), message)))
        {
            char utf8[2048]{};
            const int bytes = ::WideCharToMultiByte(CP_UTF8, 0, line, -1, utf8, (int)sizeof(utf8), nullptr, nullptr);
            if (bytes > 1) { DWORD written = 0; ::WriteFile(file, utf8, (DWORD)bytes - 1, &written, nullptr); ::FlushFileBuffers(file); }
        }
        ::CloseHandle(file);
    }

    static bool IsSteamProcessRunning()
    {
        HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;
        PROCESSENTRY32W entry{}; entry.dwSize = sizeof(entry);
        bool found = false;
        if (::Process32FirstW(snapshot, &entry)) do
        {
            if (_wcsicmp(entry.szExeFile, L"steam.exe") == 0) { found = true; break; }
        } while (::Process32NextW(snapshot, &entry));
        ::CloseHandle(snapshot);
        return found;
    }

    static bool GetSteamExePath(wchar_t* path, size_t capacity)
    {
        HKEY key{};
        if (::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
        {
            DWORD type = 0, bytes = (DWORD)(capacity * sizeof(wchar_t));
            const LONG result = ::RegQueryValueExW(key, L"SteamPath", nullptr, &type, (BYTE*)path, &bytes);
            ::RegCloseKey(key);
            if (result == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ))
            {
                path[capacity - 1] = L'\0';
                for (wchar_t* c = path; *c; ++c) if (*c == L'/') *c = L'\\';
                if (SUCCEEDED(::StringCchCatW(path, capacity, L"\\steam.exe")) && ::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES)
                    return true;
            }
        }
        wchar_t programFiles[MAX_PATH]{};
        const DWORD length = ::GetEnvironmentVariableW(L"ProgramFiles(x86)", programFiles, _countof(programFiles));
        return length && length < _countof(programFiles) &&
            SUCCEEDED(::StringCchPrintfW(path, capacity, L"%ls\\Steam\\steam.exe", programFiles)) &&
            ::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
    }

    static void StartSteamAndRelaunchRepo()
    {
        wchar_t marker[8]{};
        if (::GetEnvironmentVariableW(L"REPOHAX_STEAM_RELAUNCHED", marker, _countof(marker)))
        {
            BootstrapLog(L"REPO restart reached HAX; automatic retry guard is active."); return;
        }
        if (IsSteamProcessRunning())
        {
            BootstrapLog(L"Steam.exe is already running; no automatic restart needed."); return;
        }
        wchar_t steamExe[MAX_PATH]{};
        if (!GetSteamExePath(steamExe, _countof(steamExe)))
        {
            BootstrapLog(L"ERROR: Steam was not running, but steam.exe could not be located from its registry path or Program Files (x86)."); return;
        }
        STARTUPINFOW steamStartup{}; steamStartup.cb = sizeof(steamStartup);
        PROCESS_INFORMATION steamProcess{};
        if (!::CreateProcessW(steamExe, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &steamStartup, &steamProcess))
        {
            wchar_t message[256]{};
            ::StringCchPrintfW(message, _countof(message), L"ERROR: starting Steam failed (Win32 error %lu).", ::GetLastError());
            BootstrapLog(message); return;
        }
        ::CloseHandle(steamProcess.hThread); ::CloseHandle(steamProcess.hProcess);
        BootstrapLog(L"Steam launch requested; waiting up to 90 seconds for steam.exe.");
        unsigned int stableSeconds = 0;
        for (unsigned int second = 0; second < 90 && stableSeconds < 5; ++second)
        {
            ::Sleep(1000);
            stableSeconds = IsSteamProcessRunning() ? stableSeconds + 1 : 0;
        }
        if (stableSeconds < 5)
        {
            BootstrapLog(L"ERROR: steam.exe did not become stable within 90 seconds; keeping current REPO process."); return;
        }
        ::Sleep(8000);
        wchar_t exePath[MAX_PATH]{}, commandLine[32768]{}, currentDirectory[MAX_PATH]{};
        const wchar_t* originalCommandLine = ::GetCommandLineW();
        if (!::GetModuleFileNameW(nullptr, exePath, _countof(exePath)) || !originalCommandLine ||
            FAILED(::StringCchCopyW(commandLine, _countof(commandLine), originalCommandLine)))
        {
            BootstrapLog(L"ERROR: reading the REPO path or command line failed; REPO was not restarted."); return;
        }
        ::GetCurrentDirectoryW(_countof(currentDirectory), currentDirectory);
        ::SetEnvironmentVariableW(L"REPOHAX_STEAM_RELAUNCHED", L"1");
        STARTUPINFOW repoStartup{}; repoStartup.cb = sizeof(repoStartup);
        PROCESS_INFORMATION repoProcess{};
        const BOOL started = ::CreateProcessW(exePath, commandLine, nullptr, nullptr, FALSE, 0, nullptr,
            currentDirectory[0] ? currentDirectory : nullptr, &repoStartup, &repoProcess);
        const DWORD error = started ? ERROR_SUCCESS : ::GetLastError();
        ::SetEnvironmentVariableW(L"REPOHAX_STEAM_RELAUNCHED", nullptr);
        if (!started)
        {
            wchar_t message[256]{};
            ::StringCchPrintfW(message, _countof(message), L"ERROR: Steam is running, but restarting REPO failed (Win32 error %lu).", error);
            BootstrapLog(message); return;
        }
        ::CloseHandle(repoProcess.hThread); ::CloseHandle(repoProcess.hProcess);
        BootstrapLog(L"Steam is running; REPO restart succeeded. Closing the original process.");
        ::ExitProcess(0);
    }
}

namespace Cheat
{
    extern void Initialize(HMODULE hDll);
}

static unsigned int WINAPI Start(void* hDll)
{
    if (Hax::Unity::IsUnityProcess())
        StartSteamAndRelaunchRepo();
    Cheat::Initialize((HMODULE)hDll);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        ::DisableThreadLibraryCalls(handle);
        
        HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, Start, (void*)handle, 0, nullptr);
        if (hThread == nullptr)
            return FALSE;

        ::CloseHandle(hThread);
    }

    return TRUE;
}

#define SYS_PATH "\\\\.\\GLOBALROOT\\SystemRoot\\System32\\dwmapi."
#pragma comment(linker, "/export:DwmpDxGetWindowSharedSurface=" SYS_PATH "DwmpDxGetWindowSharedSurface,@100")
#pragma comment(linker, "/export:DwmpDxUpdateWindowSharedSurface=" SYS_PATH "DwmpDxUpdateWindowSharedSurface,@101")
#pragma comment(linker, "/export:DwmEnableComposition=" SYS_PATH "DwmEnableComposition,@102")
#pragma comment(linker, "/export:DllCanUnloadNow=" SYS_PATH "DllCanUnloadNow,PRIVATE")
#pragma comment(linker, "/export:DllGetClassObject=" SYS_PATH "DllGetClassObject,PRIVATE")
#pragma comment(linker, "/export:DwmAttachMilContent=" SYS_PATH "DwmAttachMilContent,@116")
#pragma comment(linker, "/export:DwmDefWindowProc=" SYS_PATH "DwmDefWindowProc,@117")
#pragma comment(linker, "/export:DwmDetachMilContent=" SYS_PATH "DwmDetachMilContent,@118")
#pragma comment(linker, "/export:DwmEnableBlurBehindWindow=" SYS_PATH "DwmEnableBlurBehindWindow,@119")
#pragma comment(linker, "/export:DwmEnableMMCSS=" SYS_PATH "DwmEnableMMCSS,@120")
#pragma comment(linker, "/export:DwmExtendFrameIntoClientArea=" SYS_PATH "DwmExtendFrameIntoClientArea,@121")
#pragma comment(linker, "/export:DwmFlush=" SYS_PATH "DwmFlush,@122")
#pragma comment(linker, "/export:DwmGetColorizationColor=" SYS_PATH "DwmGetColorizationColor,@123")
#pragma comment(linker, "/export:DwmGetCompositionTimingInfo=" SYS_PATH "DwmGetCompositionTimingInfo,@125")
#pragma comment(linker, "/export:DwmGetGraphicsStreamClient=" SYS_PATH "DwmGetGraphicsStreamClient,@126")
#pragma comment(linker, "/export:DwmpGetColorizationParameters=" SYS_PATH "DwmpGetColorizationParameters,@127")
#pragma comment(linker, "/export:DwmpDxgiIsThreadDesktopComposited=" SYS_PATH "DwmpDxgiIsThreadDesktopComposited,@128")
#pragma comment(linker, "/export:DwmGetGraphicsStreamTransformHint=" SYS_PATH "DwmGetGraphicsStreamTransformHint,@129")
#pragma comment(linker, "/export:DwmGetTransportAttributes=" SYS_PATH "DwmGetTransportAttributes,@130")
#pragma comment(linker, "/export:DwmpSetColorizationParameters=" SYS_PATH "DwmpSetColorizationParameters,@131")
#pragma comment(linker, "/export:DwmGetUnmetTabRequirements=" SYS_PATH "DwmGetUnmetTabRequirements,@133")
#pragma comment(linker, "/export:DwmGetWindowAttribute=" SYS_PATH "DwmGetWindowAttribute,@134")
#pragma comment(linker, "/export:DwmpRenderFlick=" SYS_PATH "DwmpRenderFlick,@135")
#pragma comment(linker, "/export:DwmpAllocateSecurityDescriptor=" SYS_PATH "DwmpAllocateSecurityDescriptor,@136")
#pragma comment(linker, "/export:DwmpFreeSecurityDescriptor=" SYS_PATH "DwmpFreeSecurityDescriptor,@137")
#pragma comment(linker, "/export:DwmpEnableDDASupport=" SYS_PATH "DwmpEnableDDASupport,@143")
#pragma comment(linker, "/export:DwmInvalidateIconicBitmaps=" SYS_PATH "DwmInvalidateIconicBitmaps,@149")
#pragma comment(linker, "/export:DwmTetherTextContact=" SYS_PATH "DwmTetherTextContact,@156")
#pragma comment(linker, "/export:DwmpUpdateProxyWindowForCapture=" SYS_PATH "DwmpUpdateProxyWindowForCapture,@183")
#pragma comment(linker, "/export:DwmIsCompositionEnabled=" SYS_PATH "DwmIsCompositionEnabled,@184")
#pragma comment(linker, "/export:DwmModifyPreviousDxFrameDuration=" SYS_PATH "DwmModifyPreviousDxFrameDuration,@185")
#pragma comment(linker, "/export:DwmQueryThumbnailSourceSize=" SYS_PATH "DwmQueryThumbnailSourceSize,@186")
#pragma comment(linker, "/export:DwmRegisterThumbnail=" SYS_PATH "DwmRegisterThumbnail,@187")
#pragma comment(linker, "/export:DwmRenderGesture=" SYS_PATH "DwmRenderGesture,@188")
#pragma comment(linker, "/export:DwmSetDxFrameDuration=" SYS_PATH "DwmSetDxFrameDuration,@189")
#pragma comment(linker, "/export:DwmSetIconicLivePreviewBitmap=" SYS_PATH "DwmSetIconicLivePreviewBitmap,@190")
#pragma comment(linker, "/export:DwmSetIconicThumbnail=" SYS_PATH "DwmSetIconicThumbnail,@191")
#pragma comment(linker, "/export:DwmSetPresentParameters=" SYS_PATH "DwmSetPresentParameters,@192")
#pragma comment(linker, "/export:DwmSetWindowAttribute=" SYS_PATH "DwmSetWindowAttribute,@193")
#pragma comment(linker, "/export:DwmShowContact=" SYS_PATH "DwmShowContact,@194")
#pragma comment(linker, "/export:DwmTetherContact=" SYS_PATH "DwmTetherContact,@195")
#pragma comment(linker, "/export:DwmTransitionOwnedWindow=" SYS_PATH "DwmTransitionOwnedWindow,@196")
#pragma comment(linker, "/export:DwmUnregisterThumbnail=" SYS_PATH "DwmUnregisterThumbnail,@197")
#pragma comment(linker, "/export:DwmUpdateThumbnailProperties=" SYS_PATH "DwmUpdateThumbnailProperties,@198")
