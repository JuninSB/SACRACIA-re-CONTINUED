#include "pch.h"
#include <DbgHelp.h>
#include <strsafe.h>
#include <atomic>
#include <psapi.h>
#include <tlhelp32.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <locale>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Psapi.lib")

#include "cheat.h"
//#include <third_party/haxsdk/unity/hax_unity_uvm.h>

namespace Cheat
{
    extern void HookMonoRuntimeInvoke(); // game_hooks.cpp
    extern void HookWinProcs(); // win_hooks.cpp
    extern void HookGraphics(); // graphics_hooks.cpp
    extern void HookGameFuncs(); // game_hooks.cpp
    extern void HookPlayerLoop(); // game_hooks.cpp

    Context* G;
    static wchar_t g_DiagnosticsDirectory[MAX_PATH]{};
    static volatile LONG g_SteamAutoRelaunch = 0;
    static volatile LONG g_SteamInitFailureReported = 0;

    namespace
    {
        constexpr int BenchSections = 17;
        constexpr int BenchCounts = 12;
        constexpr int BenchManagedMethods = 256;
        constexpr int BenchManagedMethodRows = 24;
        struct BenchCounter { volatile LONG64 Calls = 0, Ticks = 0, MaxTicks = 0; };
        struct BenchManagedMethod
        {
            volatile LONG State = 0;
            void* Method = nullptr;
            char MethodName[24]{};
            char ClassName[96]{};
            volatile LONG64 Calls = 0, Ticks = 0, MaxTicks = 0;
        };
        static volatile LONG g_BenchEnabled = 0;
        static volatile LONG64 g_BenchFrames = 0, g_BenchFrameTicks = 0, g_BenchMaxFrameTicks = 0;
        static volatile LONG64 g_BenchIntervals = 0, g_BenchIntervalTicks = 0, g_BenchMaxIntervalTicks = 0;
        static volatile LONG64 g_BenchHitches50 = 0, g_BenchHitches100 = 0;
        static volatile LONG64 g_BenchHitches33 = 0;
        static volatile LONG64 g_BenchFrameBins[5] = {};
        static volatile LONG64 g_BenchGpuFrames = 0, g_BenchGpuUs = 0, g_BenchGpuMaxUs = 0;
        static BenchCounter g_BenchSections[BenchSections];
        static BenchManagedMethod g_BenchManagedMethods[BenchManagedMethods];
        static SRWLOCK g_BenchManagedMethodsLock = SRWLOCK_INIT;
        static volatile LONG g_BenchCounts[BenchCounts] = {};
        static volatile LONG g_BenchAnimatedScriptCount = 0;
        static volatile LONG64 g_BenchScanUs = 0;
        static volatile LONG g_BenchScanType = -1;
        static ULONGLONG g_BenchNextScan = 0;
        static int g_BenchScanIndex = 0;
        static volatile LONG g_BenchUnityThreadId = 0;
        static volatile LONG g_BenchInGame = 0, g_BenchIsClient = 0;
        static wchar_t g_BenchCsvPath[MAX_PATH]{};

        struct BenchManagedMethodSample
        {
            char MethodName[24]{};
            char ClassName[96]{};
            LONG64 Calls = 0;
            LONG64 Ticks = 0;
            LONG64 MaxTicks = 0;
        };

        static void AtomicMax(volatile LONG64* target, LONG64 value)
        {
            LONG64 old = ::InterlockedCompareExchange64(target, 0, 0);
            while (value > old)
            {
                const LONG64 seen = ::InterlockedCompareExchange64(target, value, old);
                if (seen == old) break;
                old = seen;
            }
        }

        static ULONGLONG FileTimeValue(const FILETIME& value)
        {
            ULARGE_INTEGER v{}; v.LowPart = value.dwLowDateTime; v.HighPart = value.dwHighDateTime; return v.QuadPart;
        }

        static DWORD CountProcessThreads(DWORD processId)
        {
            HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (snapshot == INVALID_HANDLE_VALUE) return 0;
            THREADENTRY32 entry{}; entry.dwSize = sizeof(entry);
            DWORD count = 0;
            if (::Thread32First(snapshot, &entry))
                do { if (entry.th32OwnerProcessID == processId) ++count; } while (::Thread32Next(snapshot, &entry));
            ::CloseHandle(snapshot);
            return count;
        }

        static unsigned int __stdcall BenchmarkWorker(void*)
        {
            HANDLE file = INVALID_HANDLE_VALUE;
            HANDLE methodFile = INVALID_HANDLE_VALUE;
            bool wasEnabled = false;
            ULONGLONG captureStart = 0, lastSample = 0;
            ULONGLONG previousProcessCpu = 0, previousThreadCpu = 0;
            DWORD previousPageFaults = 0;
            bool haveCpuBaseline = false, haveThreadBaseline = false;
            while (::WaitForSingleObject(::GetCurrentProcess(), 1000) == WAIT_TIMEOUT)
            {
                const bool enabled = ::InterlockedCompareExchange(&g_BenchEnabled, 0, 0) != 0;
                if (enabled && !wasEnabled)
                {
                    SYSTEMTIME time{}; ::GetLocalTime(&time);
                    ::StringCchPrintfW(g_BenchCsvPath, _countof(g_BenchCsvPath),
                        L"%ls\\Benchmark_%04u%02u%02u_%02u%02u%02u.csv", g_DiagnosticsDirectory,
                        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
                    file = ::CreateFileW(g_BenchCsvPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    wchar_t methodCsvPath[MAX_PATH]{};
                    ::StringCchPrintfW(methodCsvPath, _countof(methodCsvPath),
                        L"%ls\\Benchmark_Methods_%04u%02u%02u_%02u%02u%02u.csv", g_DiagnosticsDirectory,
                        time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
                    methodFile = ::CreateFileW(methodCsvPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (methodFile != INVALID_HANDLE_VALUE)
                    {
                        const char methodHeader[] = "elapsed_s,method,class,calls,total_ms,avg_ms,max_ms\r\n";
                        DWORD methodWritten = 0; ::WriteFile(methodFile, methodHeader, sizeof(methodHeader) - 1, &methodWritten, nullptr);
                    }
                    if (file != INVALID_HANDLE_VALUE)
                    {
                        const char header[] = "elapsed_s,fps,wall_frame_avg_ms,wall_frame_max_ms,hitches_over_33ms,hitches_over_50ms,hitches_over_100ms,frames_under_17ms,frames_17_to_33ms,frames_33_to_50ms,frames_50_to_100ms,frames_over_100ms,update_cpu_avg_ms,update_cpu_max_ms,postlate_avg_ms,postlate_calls,present_afterdraw_avg_ms,present_afterdraw_calls,grab_physics_avg_ms,grab_physics_calls,fixed_physics_avg_ms,fixed_physics_calls,fixed_scripts_avg_ms,fixed_scripts_calls,late_scripts_avg_ms,late_scripts_calls,overlay_cpu_avg_ms,overlay_calls,dxgi_present_cpu_avg_ms,dxgi_present_calls,enemy_fixed_avg_ms,enemy_fixed_calls,esp_total_avg_ms,esp_total_calls,esp_enemies_avg_ms,esp_enemies_calls,esp_valuables_avg_ms,esp_valuables_calls,esp_extraction_avg_ms,esp_extraction_calls,esp_truck_avg_ms,esp_truck_calls,esp_players_avg_ms,esp_players_calls,esp_cosmetics_avg_ms,esp_cosmetics_calls,gpu_frames,gpu_scene_avg_ms,gpu_scene_max_ms,process_cpu_machine_pct,unity_thread_cpu_pct,in_game,is_client,working_set_mb,private_mb,page_faults_delta,handle_count,thread_count,particle_systems,animators,legacy_animations,lod_groups,renderers,skinned_renderers,rigidbodies,colliders,lights,audio_sources,cameras,monobehaviours,animated_texture_like_scripts,component_scan_type_index,component_scan_us\r\n";
                        DWORD written = 0; ::WriteFile(file, header, sizeof(header) - 1, &written, nullptr);
                    }
                    wasEnabled = true;
                    captureStart = lastSample = ::GetTickCount64();
                    previousProcessCpu = previousThreadCpu = 0; previousPageFaults = 0;
                    haveCpuBaseline = haveThreadBaseline = false;
                }
                if (enabled && file != INVALID_HANDLE_VALUE)
                {
                    const LONG64 frames = ::InterlockedExchange64(&g_BenchFrames, 0);
                    const LONG64 frameTicks = ::InterlockedExchange64(&g_BenchFrameTicks, 0);
                    const LONG64 maxTicks = ::InterlockedExchange64(&g_BenchMaxFrameTicks, 0);
                    const LONG64 intervals = ::InterlockedExchange64(&g_BenchIntervals, 0);
                    const LONG64 intervalTicks = ::InterlockedExchange64(&g_BenchIntervalTicks, 0);
                    const LONG64 maxIntervalTicks = ::InterlockedExchange64(&g_BenchMaxIntervalTicks, 0);
                    const LONG64 h50 = ::InterlockedExchange64(&g_BenchHitches50, 0);
                    const LONG64 h100 = ::InterlockedExchange64(&g_BenchHitches100, 0);
                    const LONG64 h33 = ::InterlockedExchange64(&g_BenchHitches33, 0);
                    LONG64 bins[5]{}; for (int i = 0; i < 5; ++i) bins[i] = ::InterlockedExchange64(&g_BenchFrameBins[i], 0);
                    const LONG64 gpuFrames = ::InterlockedExchange64(&g_BenchGpuFrames, 0);
                    const LONG64 gpuUs = ::InterlockedExchange64(&g_BenchGpuUs, 0);
                    const LONG64 gpuMaxUs = ::InterlockedExchange64(&g_BenchGpuMaxUs, 0);
                    const ULONGLONG sampleNow = ::GetTickCount64();
                    const ULONGLONG sampleMs = sampleNow > lastSample ? sampleNow - lastSample : 1000;
                    lastSample = sampleNow;
                    const ULONGLONG elapsedSeconds = (sampleNow - captureStart) / 1000;
                    LARGE_INTEGER frequency{}; ::QueryPerformanceFrequency(&frequency);
                    const double updateAvgMs = frames && frequency.QuadPart ? (double)frameTicks * 1000.0 / frequency.QuadPart / frames : 0.0;
                    const double updateMaxMs = frequency.QuadPart ? (double)maxTicks * 1000.0 / frequency.QuadPart : 0.0;
                    const double wallAvgMs = intervals && frequency.QuadPart ? (double)intervalTicks * 1000.0 / frequency.QuadPart / intervals : 0.0;
                    const double wallMaxMs = frequency.QuadPart ? (double)maxIntervalTicks * 1000.0 / frequency.QuadPart : 0.0;
                    double sectionMs[BenchSections]{}; LONG64 sectionCalls[BenchSections]{};
                    for (int i = 0; i < BenchSections; ++i)
                    {
                        const LONG64 calls = ::InterlockedExchange64(&g_BenchSections[i].Calls, 0); sectionCalls[i] = calls;
                        const LONG64 ticks = ::InterlockedExchange64(&g_BenchSections[i].Ticks, 0);
                        ::InterlockedExchange64(&g_BenchSections[i].MaxTicks, 0);
                        if (frequency.QuadPart) sectionMs[i] = (double)ticks * 1000.0 / frequency.QuadPart / (calls ? calls : 1);
                    }
                    PROCESS_MEMORY_COUNTERS_EX memory{}; memory.cb = sizeof(memory);
                    ::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memory, sizeof(memory));
                    FILETIME creation{}, exit{}, kernel{}, user{};
                    double processCpuPct = 0.0, unityThreadPct = 0.0;
                    if (::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel, &user))
                    {
                        const ULONGLONG current = FileTimeValue(kernel) + FileTimeValue(user);
                        const LONG threadId = ::InterlockedCompareExchange(&g_BenchUnityThreadId, 0, 0);
                        HANDLE unityThread = threadId ? ::OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)threadId) : nullptr;
                        FILETIME threadCreate{}, threadExit{}, threadKernel{}, threadUser{};
                        ULONGLONG threadCurrent = 0;
                        const bool haveThread = unityThread && ::GetThreadTimes(unityThread, &threadCreate, &threadExit, &threadKernel, &threadUser);
                        if (haveThread) threadCurrent = FileTimeValue(threadKernel) + FileTimeValue(threadUser);
                        if (haveCpuBaseline && sampleMs)
                        {
                            processCpuPct = (double)(current - previousProcessCpu) / ((double)sampleMs * 100.0);
                            if (haveThread && haveThreadBaseline && threadCurrent >= previousThreadCpu)
                                unityThreadPct = (double)(threadCurrent - previousThreadCpu) / ((double)sampleMs * 100.0);
                        }
                        previousProcessCpu = current; haveCpuBaseline = true;
                        if (haveThread) { previousThreadCpu = threadCurrent; haveThreadBaseline = true; }
                        if (unityThread) ::CloseHandle(unityThread);
                    }
                    const DWORD pageFaultDelta = previousPageFaults && memory.PageFaultCount >= previousPageFaults ? memory.PageFaultCount - previousPageFaults : 0;
                    previousPageFaults = memory.PageFaultCount;
                    DWORD handleCount = 0; ::GetProcessHandleCount(::GetCurrentProcess(), &handleCount);
                    const DWORD threadCount = CountProcessThreads(::GetCurrentProcessId());
                    std::ostringstream line; line.imbue(std::locale::classic()); line << std::fixed;
                    line << elapsedSeconds << ',' << std::setprecision(2) << (double)frames * 1000.0 / sampleMs
                        << ',' << std::setprecision(3) << wallAvgMs << ',' << wallMaxMs << ',' << h33 << ',' << h50 << ',' << h100;
                    for (int i = 0; i < 5; ++i) line << ',' << bins[i];
                    line << ',' << updateAvgMs << ',' << updateMaxMs;
                    for (int i = 1; i < BenchSections; ++i)
                        line << ',' << std::setprecision(4) << sectionMs[i] << ',' << sectionCalls[i];
                    line << ',' << gpuFrames << ',' << std::setprecision(3) << (gpuFrames ? (double)gpuUs / gpuFrames / 1000.0 : 0.0)
                        << ',' << (double)gpuMaxUs / 1000.0;
                    const DWORD processorCount = ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
                    line << ',' << std::setprecision(2) << processCpuPct / (processorCount ? processorCount : 1)
                        << ',' << unityThreadPct << ',' << ::InterlockedCompareExchange(&g_BenchInGame, 0, 0)
                        << ',' << ::InterlockedCompareExchange(&g_BenchIsClient, 0, 0) << ',' << (double)memory.WorkingSetSize / (1024.0 * 1024.0)
                        << ',' << (double)memory.PrivateUsage / (1024.0 * 1024.0) << ',' << pageFaultDelta << ',' << handleCount << ',' << threadCount;
                    for (int i = 0; i < BenchCounts; ++i) line << ',' << ::InterlockedCompareExchange(&g_BenchCounts[i], 0, 0);
                    line << ',' << ::InterlockedCompareExchange(&g_BenchAnimatedScriptCount, 0, 0)
                        << ',' << ::InterlockedCompareExchange(&g_BenchScanType, -1, -1)
                        << ',' << ::InterlockedExchange64(&g_BenchScanUs, 0) << "\r\n";
                    const std::string row = line.str();
                    DWORD written = 0; ::WriteFile(file, row.data(), (DWORD)row.size(), &written, nullptr); ::FlushFileBuffers(file);

                    if (methodFile != INVALID_HANDLE_VALUE)
                    {
                        BenchManagedMethodSample samples[BenchManagedMethods]{};
                        int sampleCount = 0;
                        for (int i = 0; i < BenchManagedMethods; ++i)
                        {
                            BenchManagedMethod& method = g_BenchManagedMethods[i];
                            if (::InterlockedCompareExchange(&method.State, 0, 0) != 2)
                                continue;
                            const LONG64 calls = ::InterlockedExchange64(&method.Calls, 0);
                            const LONG64 ticks = ::InterlockedExchange64(&method.Ticks, 0);
                            const LONG64 maxTicks = ::InterlockedExchange64(&method.MaxTicks, 0);
                            if (!calls || !ticks)
                                continue;
                            BenchManagedMethodSample& sample = samples[sampleCount++];
                            strncpy_s(sample.MethodName, method.MethodName, _TRUNCATE);
                            strncpy_s(sample.ClassName, method.ClassName, _TRUNCATE);
                            sample.Calls = calls;
                            sample.Ticks = ticks;
                            sample.MaxTicks = maxTicks;
                        }
                        std::sort(samples, samples + sampleCount, [](const BenchManagedMethodSample& a, const BenchManagedMethodSample& b) { return a.Ticks > b.Ticks; });
                        const int rowsToWrite = (std::min)(sampleCount, BenchManagedMethodRows);
                        std::ostringstream methodRows; methodRows.imbue(std::locale::classic()); methodRows << std::fixed;
                        for (int i = 0; i < rowsToWrite; ++i)
                        {
                            const BenchManagedMethodSample& sample = samples[i];
                            const double totalMs = frequency.QuadPart ? (double)sample.Ticks * 1000.0 / frequency.QuadPart : 0.0;
                            const double maxMs = frequency.QuadPart ? (double)sample.MaxTicks * 1000.0 / frequency.QuadPart : 0.0;
                            methodRows << elapsedSeconds << ',' << sample.MethodName << ',' << sample.ClassName << ',' << sample.Calls
                                << ',' << std::setprecision(4) << totalMs << ',' << (totalMs / sample.Calls) << ',' << maxMs << "\r\n";
                        }
                        const std::string methodRowText = methodRows.str();
                        if (!methodRowText.empty())
                        {
                            DWORD methodWritten = 0;
                            ::WriteFile(methodFile, methodRowText.data(), (DWORD)methodRowText.size(), &methodWritten, nullptr);
                            ::FlushFileBuffers(methodFile);
                        }
                    }
                }
                if (!enabled && wasEnabled)
                {
                    if (file != INVALID_HANDLE_VALUE) { ::FlushFileBuffers(file); ::CloseHandle(file); file = INVALID_HANDLE_VALUE; }
                    if (methodFile != INVALID_HANDLE_VALUE) { ::FlushFileBuffers(methodFile); ::CloseHandle(methodFile); methodFile = INVALID_HANDLE_VALUE; }
                    wasEnabled = false;
                }
            }
            if (file != INVALID_HANDLE_VALUE) ::CloseHandle(file);
            if (methodFile != INVALID_HANDLE_VALUE) ::CloseHandle(methodFile);
            return 0;
        }
    }

    void SetBenchmarkCapture(bool enabled)
    {
        const LONG old = ::InterlockedExchange(&g_BenchEnabled, enabled ? 1 : 0);
        if (enabled && !old)
        {
            ::InterlockedExchange(&g_BenchUnityThreadId, 0);
            ::InterlockedExchange64(&g_BenchFrames, 0); ::InterlockedExchange64(&g_BenchFrameTicks, 0); ::InterlockedExchange64(&g_BenchMaxFrameTicks, 0);
            ::InterlockedExchange64(&g_BenchIntervals, 0); ::InterlockedExchange64(&g_BenchIntervalTicks, 0); ::InterlockedExchange64(&g_BenchMaxIntervalTicks, 0);
            ::InterlockedExchange64(&g_BenchHitches33, 0); ::InterlockedExchange64(&g_BenchHitches50, 0); ::InterlockedExchange64(&g_BenchHitches100, 0);
            for (int i = 0; i < 5; ++i) ::InterlockedExchange64(&g_BenchFrameBins[i], 0);
            ::InterlockedExchange64(&g_BenchGpuFrames, 0); ::InterlockedExchange64(&g_BenchGpuUs, 0); ::InterlockedExchange64(&g_BenchGpuMaxUs, 0);
            for (int i = 0; i < BenchSections; ++i) { ::InterlockedExchange64(&g_BenchSections[i].Calls, 0); ::InterlockedExchange64(&g_BenchSections[i].Ticks, 0); ::InterlockedExchange64(&g_BenchSections[i].MaxTicks, 0); }
            for (int i = 0; i < BenchManagedMethods; ++i)
            {
                ::InterlockedExchange64(&g_BenchManagedMethods[i].Calls, 0);
                ::InterlockedExchange64(&g_BenchManagedMethods[i].Ticks, 0);
                ::InterlockedExchange64(&g_BenchManagedMethods[i].MaxTicks, 0);
            }
            for (int i = 0; i < BenchCounts; ++i) ::InterlockedExchange(&g_BenchCounts[i], -1);
            ::InterlockedExchange(&g_BenchAnimatedScriptCount, -1);
            ::InterlockedExchange(&g_BenchScanType, -1);
            ::InterlockedExchange64(&g_BenchScanUs, 0);
            g_BenchNextScan = enabled ? (::GetTickCount64() + 5000) : 0;
            g_BenchScanIndex = 0;
        }
    }
    bool IsBenchmarkCaptureEnabled() { return ::InterlockedCompareExchange(&g_BenchEnabled, 0, 0) != 0; }
    void BenchmarkRecordFrame(unsigned long long ticks)
    {
        if (!IsBenchmarkCaptureEnabled()) return;
        ::InterlockedCompareExchange(&g_BenchUnityThreadId, (LONG)::GetCurrentThreadId(), 0);
        if (G) { ::InterlockedExchange(&g_BenchInGame, G->IsInGame ? 1 : 0); ::InterlockedExchange(&g_BenchIsClient, G->IsClient ? 1 : 0); }
        ::InterlockedIncrement64(&g_BenchFrames); ::InterlockedAdd64(&g_BenchFrameTicks, (LONG64)ticks); AtomicMax(&g_BenchMaxFrameTicks, (LONG64)ticks);
    }
    void BenchmarkRecordFrameInterval(unsigned long long ticks)
    {
        if (!IsBenchmarkCaptureEnabled()) return;
        ::InterlockedIncrement64(&g_BenchIntervals); ::InterlockedAdd64(&g_BenchIntervalTicks, (LONG64)ticks); AtomicMax(&g_BenchMaxIntervalTicks, (LONG64)ticks);
        LARGE_INTEGER f{}; ::QueryPerformanceFrequency(&f);
        int bin = 0;
        if (f.QuadPart && ticks * 1000ULL >= (unsigned long long)f.QuadPart * 100ULL) bin = 4;
        else if (f.QuadPart && ticks * 1000ULL >= (unsigned long long)f.QuadPart * 50ULL) bin = 3;
        else if (f.QuadPart && ticks * 1000ULL >= (unsigned long long)f.QuadPart * 33ULL) bin = 2;
        else if (f.QuadPart && ticks * 1000ULL >= (unsigned long long)f.QuadPart * 17ULL) bin = 1;
        ::InterlockedIncrement64(&g_BenchFrameBins[bin]);
        if (f.QuadPart && ticks * 1000ULL > (unsigned long long)f.QuadPart * 33ULL) ::InterlockedIncrement64(&g_BenchHitches33);
        if (f.QuadPart && ticks * 1000ULL > (unsigned long long)f.QuadPart * 50ULL) ::InterlockedIncrement64(&g_BenchHitches50);
        if (f.QuadPart && ticks * 1000ULL > (unsigned long long)f.QuadPart * 100ULL) ::InterlockedIncrement64(&g_BenchHitches100);
    }
    void BenchmarkRecordSection(int section, unsigned long long ticks)
    {
        if (!IsBenchmarkCaptureEnabled() || section < 0 || section >= BenchSections) return;
        ::InterlockedIncrement64(&g_BenchSections[section].Calls); ::InterlockedAdd64(&g_BenchSections[section].Ticks, (LONG64)ticks); AtomicMax(&g_BenchSections[section].MaxTicks, (LONG64)ticks);
    }
    bool BenchmarkManagedMethodKnown(void* method)
    {
        if (!method) return false;
        const size_t start = ((uintptr_t)method >> 4) % BenchManagedMethods;
        for (size_t probe = 0; probe < BenchManagedMethods; ++probe)
        {
            BenchManagedMethod& candidate = g_BenchManagedMethods[(start + probe) % BenchManagedMethods];
            const LONG state = ::InterlockedCompareExchange(&candidate.State, 0, 0);
            if (state == 0) return false;
            if (state == 2 && candidate.Method == method) return true;
        }
        return false;
    }
    void BenchmarkRecordManagedMethod(void* method, const char* methodName, const char* className, unsigned long long ticks)
    {
        if (!IsBenchmarkCaptureEnabled() || !method || !methodName || !ticks)
            return;

        const size_t start = ((uintptr_t)method >> 4) % BenchManagedMethods;
        BenchManagedMethod* target = nullptr;
        for (size_t probe = 0; probe < BenchManagedMethods; ++probe)
        {
            BenchManagedMethod& candidate = g_BenchManagedMethods[(start + probe) % BenchManagedMethods];
            const LONG state = ::InterlockedCompareExchange(&candidate.State, 0, 0);
            if (state == 0)
                break;
            if (state == 2 && candidate.Method == method)
            {
                target = &candidate;
                break;
            }
        }

        if (!target)
        {
            ::AcquireSRWLockExclusive(&g_BenchManagedMethodsLock);
            for (size_t probe = 0; probe < BenchManagedMethods; ++probe)
            {
                BenchManagedMethod& candidate = g_BenchManagedMethods[(start + probe) % BenchManagedMethods];
                if (candidate.State == 2 && candidate.Method == method)
                {
                    target = &candidate;
                    break;
                }
                if (candidate.State == 0)
                {
                    candidate.State = 1;
                    candidate.Method = method;
                    strncpy_s(candidate.MethodName, methodName, _TRUNCATE);
                    strncpy_s(candidate.ClassName, className ? className : "<unknown>", _TRUNCATE);
                    for (char* p = candidate.MethodName; *p; ++p) if (*p == ',' || *p == '\r' || *p == '\n') *p = '_';
                    for (char* p = candidate.ClassName; *p; ++p) if (*p == ',' || *p == '\r' || *p == '\n') *p = '_';
                    ::InterlockedExchange(&candidate.State, 2);
                    target = &candidate;
                    break;
                }
            }
            ::ReleaseSRWLockExclusive(&g_BenchManagedMethodsLock);
        }

        if (target)
        {
            ::InterlockedIncrement64(&target->Calls);
            ::InterlockedAdd64(&target->Ticks, (LONG64)ticks);
            AtomicMax(&target->MaxTicks, (LONG64)ticks);
        }
    }
    void BenchmarkRecordGpuTimeUs(unsigned long long microseconds)
    {
        if (!IsBenchmarkCaptureEnabled()) return;
        ::InterlockedIncrement64(&g_BenchGpuFrames); ::InterlockedAdd64(&g_BenchGpuUs, (LONG64)microseconds); AtomicMax(&g_BenchGpuMaxUs, (LONG64)microseconds);
    }
    void BenchmarkCaptureUnityCounts()
    {
        if (!IsBenchmarkCaptureEnabled()) return;
        const ULONGLONG now = ::GetTickCount64();
        if (now < g_BenchNextScan) return;
        g_BenchNextScan = now + 5000;
        static const char* assemblies[BenchCounts] = {
            "UnityEngine.ParticleSystemModule", "UnityEngine.AnimationModule", "UnityEngine.AnimationModule", "UnityEngine.CoreModule",
            "UnityEngine.CoreModule", "UnityEngine.CoreModule", "UnityEngine.PhysicsModule", "UnityEngine.PhysicsModule",
            "UnityEngine.CoreModule", "UnityEngine.AudioModule", "UnityEngine.CoreModule", "UnityEngine.CoreModule"
        };
        static const char* names[BenchCounts] = {
            "ParticleSystem", "Animator", "Animation", "LODGroup", "Renderer", "SkinnedMeshRenderer",
            "Rigidbody", "Collider", "Light", "AudioSource", "Camera", "MonoBehaviour"
        };
        LARGE_INTEGER start{}, end{}; ::QueryPerformanceCounter(&start);
        int i = g_BenchScanIndex;
        // Full-scene Renderer and MonoBehaviour enumeration caused 27-33 ms stalls
        // in large lobbies. Those columns stay -1 instead of perturbing the capture.
        while (i == 4 || i == 11)
            i = (i + 1) % BenchCounts;
        System::Type type = System::Type::GetType(assemblies[i], "UnityEngine", names[i], false);
        if (type.IsNull()) ::InterlockedExchange(&g_BenchCounts[i], -1);
        else
        {
            auto objects = Unity::Object::FindObjectsOfType(type);
            const int count = objects.IsNull() ? 0 : objects.Length();
            ::InterlockedExchange(&g_BenchCounts[i], count);
        }
        ::InterlockedExchange(&g_BenchScanType, i);
        g_BenchScanIndex = (i + 1) % BenchCounts;
        while (g_BenchScanIndex == 4 || g_BenchScanIndex == 11)
            g_BenchScanIndex = (g_BenchScanIndex + 1) % BenchCounts;
        ::QueryPerformanceCounter(&end); LARGE_INTEGER frequency{}; ::QueryPerformanceFrequency(&frequency);
        if (frequency.QuadPart) ::InterlockedExchange64(&g_BenchScanUs, (end.QuadPart - start.QuadPart) * 1000000 / frequency.QuadPart);
    }

    static void SnapshotDiagnosticTail(const wchar_t* source, const wchar_t* destination, DWORD maxBytes)
    {
        HANDLE input = ::CreateFileW(source, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (input == INVALID_HANDLE_VALUE)
            return;

        LARGE_INTEGER size{};
        if (!::GetFileSizeEx(input, &size) || size.QuadPart <= 0)
        {
            ::CloseHandle(input);
            return;
        }
        const LONGLONG start = size.QuadPart > maxBytes ? size.QuadPart - maxBytes : 0;
        LARGE_INTEGER offset{};
        offset.QuadPart = start;
        if (!::SetFilePointerEx(input, offset, nullptr, FILE_BEGIN))
        {
            ::CloseHandle(input);
            return;
        }

        HANDLE output = ::CreateFileW(destination, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (output != INVALID_HANDLE_VALUE)
        {
            char buffer[32768];
            DWORD read = 0;
            while (::ReadFile(input, buffer, sizeof(buffer), &read, nullptr) && read)
            {
                DWORD written = 0;
                if (!::WriteFile(output, buffer, read, &written, nullptr) || written != read)
                    break;
            }
            ::FlushFileBuffers(output);
            ::CloseHandle(output);
        }
        ::CloseHandle(input);
    }

    static bool IsSteamProcessPresentForDiagnostics()
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

    static bool PlayerLogShowsSteamInitFailure(const wchar_t* path)
    {
        if (!path || !path[0]) return false;
        HANDLE file = ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER size{};
        if (!::GetFileSizeEx(file, &size) || size.QuadPart <= 0)
        {
            ::CloseHandle(file); return false;
        }
        const DWORD bytesToRead = (DWORD)((size.QuadPart < 128 * 1024) ? size.QuadPart : 128 * 1024);
        LARGE_INTEGER offset{}; offset.QuadPart = size.QuadPart - bytesToRead;
        if (!::SetFilePointerEx(file, offset, nullptr, FILE_BEGIN))
        {
            ::CloseHandle(file); return false;
        }
        std::string tail(bytesToRead, '\0');
        DWORD bytesRead = 0;
        const BOOL readOk = ::ReadFile(file, tail.data(), bytesToRead, &bytesRead, nullptr);
        ::CloseHandle(file);
        if (!readOk) return false;
        tail.resize(bytesRead);
        return tail.find("Steamworks failed to initialize") != std::string::npos ||
            tail.find("Steam not initialized. Quitting game.") != std::string::npos;
    }

    static unsigned int __stdcall DiagnosticTailWorker(void*)
    {
        wchar_t userProfile[MAX_PATH]{}, appData[MAX_PATH]{};
        DWORD userLength = ::GetEnvironmentVariableW(L"USERPROFILE", userProfile, _countof(userProfile));
        DWORD appLength = ::GetEnvironmentVariableW(L"APPDATA", appData, _countof(appData));
        wchar_t playerLog[MAX_PATH]{}, playerSnapshot[MAX_PATH]{};
        if (userLength && userLength < _countof(userProfile))
        {
            ::StringCchPrintfW(playerLog, _countof(playerLog), L"%ls\\AppData\\LocalLow\\semiwork\\Repo\\Player.log", userProfile);
            ::StringCchPrintfW(playerSnapshot, _countof(playerSnapshot), L"%ls\\Player.log.live", g_DiagnosticsDirectory);
        }

        wchar_t profilesRoot[MAX_PATH]{}, searchPath[MAX_PATH]{}, activeProfileLog[MAX_PATH]{}, activeSnapshot[MAX_PATH]{};
        if (appLength && appLength < _countof(appData))
        {
            ::StringCchPrintfW(profilesRoot, _countof(profilesRoot), L"%ls\\Thunderstore Mod Manager\\DataFolder\\REPO\\profiles", appData);
            ::StringCchPrintfW(searchPath, _countof(searchPath), L"%ls\\*", profilesRoot);
            ::StringCchPrintfW(activeSnapshot, _countof(activeSnapshot), L"%ls\\BepInEx_active_LogOutput.live", g_DiagnosticsDirectory);
        }

        while (::WaitForSingleObject(::GetCurrentProcess(), 2000) == WAIT_TIMEOUT)
        {
            if (playerLog[0])
            {
                SnapshotDiagnosticTail(playerLog, playerSnapshot, 1024 * 1024);
                if (::InterlockedCompareExchange(&g_SteamAutoRelaunch, 0, 0) &&
                    !::InterlockedCompareExchange(&g_SteamInitFailureReported, 0, 0) &&
                    PlayerLogShowsSteamInitFailure(playerLog) &&
                    !::InterlockedExchange(&g_SteamInitFailureReported, 1))
                {
                    const bool steamRunning = IsSteamProcessPresentForDiagnostics();
                    Hax::LogError(G->Logger,
                        L"Steamworks initialization failed after HAX automatically started Steam and relaunched REPO. steam.exe process currently detected: %ls. See Player.log.live for the exact Steam API failure and steam-bootstrap.log for the launch/restart steps.",
                        steamRunning ? L"YES" : L"NO");
                }
            }

            if (!searchPath[0])
                continue;
            WIN32_FIND_DATAW profile{};
            HANDLE search = ::FindFirstFileW(searchPath, &profile);
            if (search == INVALID_HANDLE_VALUE)
                continue;

            FILETIME newest{};
            wchar_t newestLog[MAX_PATH]{};
            do
            {
                if (!(profile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || profile.cFileName[0] == L'.')
                    continue;
                wchar_t candidate[MAX_PATH]{};
                ::StringCchPrintfW(candidate, _countof(candidate), L"%ls\\%ls\\BepInEx\\LogOutput.log", profilesRoot, profile.cFileName);
                WIN32_FILE_ATTRIBUTE_DATA attributes{};
                if (::GetFileAttributesExW(candidate, GetFileExInfoStandard, &attributes) &&
                    (!newestLog[0] || ::CompareFileTime(&attributes.ftLastWriteTime, &newest) > 0))
                {
                    newest = attributes.ftLastWriteTime;
                    ::StringCchCopyW(newestLog, _countof(newestLog), candidate);
                }
            } while (::FindNextFileW(search, &profile));
            ::FindClose(search);
            if (newestLog[0])
                SnapshotDiagnosticTail(newestLog, activeSnapshot, 1024 * 1024);
        }
        return 0;
    }

    static void CopyDiagnosticFile(const wchar_t* source, const wchar_t* destination)
    {
        if (!source || !destination || !source[0] || !destination[0])
            return;
        if (::GetFileAttributesW(source) == INVALID_FILE_ATTRIBUTES)
            return;
        ::CopyFileW(source, destination, FALSE);
    }

    static void SnapshotExternalLogs()
    {
        if (!g_DiagnosticsDirectory[0])
            return;

        wchar_t localLow[MAX_PATH]{};
        DWORD length = ::GetEnvironmentVariableW(L"USERPROFILE", localLow, _countof(localLow));
        if (length && length < _countof(localLow))
        {
            wchar_t source[MAX_PATH]{}, destination[MAX_PATH]{};
            ::StringCchPrintfW(source, _countof(source), L"%ls\\AppData\\LocalLow\\semiwork\\Repo\\Player.log", localLow);
            ::StringCchPrintfW(destination, _countof(destination), L"%ls\\Player.log", g_DiagnosticsDirectory);
            CopyDiagnosticFile(source, destination);
            ::StringCchPrintfW(source, _countof(source), L"%ls\\AppData\\LocalLow\\semiwork\\Repo\\Player-prev.log", localLow);
            ::StringCchPrintfW(destination, _countof(destination), L"%ls\\Player-prev.log", g_DiagnosticsDirectory);
            CopyDiagnosticFile(source, destination);
        }

        wchar_t appData[MAX_PATH]{};
        length = ::GetEnvironmentVariableW(L"APPDATA", appData, _countof(appData));
        if (!length || length >= _countof(appData))
            return;

        wchar_t profilesRoot[MAX_PATH]{};
        ::StringCchPrintfW(profilesRoot, _countof(profilesRoot), L"%ls\\Thunderstore Mod Manager\\DataFolder\\REPO\\profiles", appData);
        wchar_t profilesSearch[MAX_PATH]{};
        ::StringCchPrintfW(profilesSearch, _countof(profilesSearch), L"%ls\\*", profilesRoot);
        WIN32_FIND_DATAW profile{};
        HANDLE search = ::FindFirstFileW(profilesSearch, &profile);
        if (search == INVALID_HANDLE_VALUE)
            return;

        do
        {
            if (!(profile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || profile.cFileName[0] == L'.')
                continue;

            wchar_t source[MAX_PATH]{}, destination[MAX_PATH]{};
            ::StringCchPrintfW(source, _countof(source), L"%ls\\%ls\\BepInEx\\LogOutput.log", profilesRoot, profile.cFileName);
            ::StringCchPrintfW(destination, _countof(destination), L"%ls\\BepInEx_%ls_LogOutput.log", g_DiagnosticsDirectory, profile.cFileName);
            CopyDiagnosticFile(source, destination);
            ::StringCchPrintfW(source, _countof(source), L"%ls\\%ls\\BepInEx\\config\\JustSomeGuyDev.DisableClientsideTimeout.cfg", profilesRoot, profile.cFileName);
            ::StringCchPrintfW(destination, _countof(destination), L"%ls\\BepInEx_%ls_DisableClientsideTimeout.cfg", g_DiagnosticsDirectory, profile.cFileName);
            CopyDiagnosticFile(source, destination);
        } while (::FindNextFileW(search, &profile));
        ::FindClose(search);
    }

    void SnapshotDiagnostics(const wchar_t* reason)
    {
        if (G)
            Hax::Log(G->Logger, L"Diagnostic snapshot requested: %ls", reason ? reason : L"unspecified");
        SnapshotExternalLogs();
    }

    static LONG WINAPI RepoHaxCrashHandler(EXCEPTION_POINTERS* exception)
    {
        wchar_t tempPath[MAX_PATH]{};
        if (::GetTempPathW(_countof(tempPath), tempPath) > 0)
        {
            wchar_t dumpPath[MAX_PATH]{};
            wchar_t crashLogPath[MAX_PATH]{};
            if (!g_DiagnosticsDirectory[0])
            {
                ::StringCchPrintfW(g_DiagnosticsDirectory, _countof(g_DiagnosticsDirectory), L"%lsRepoHaxDiagnostics", tempPath);
                ::CreateDirectoryW(g_DiagnosticsDirectory, nullptr);
            }
            SYSTEMTIME time{};
            ::GetLocalTime(&time);
            ::StringCchPrintfW(dumpPath, _countof(dumpPath), L"%ls\\Crash_%04u%02u%02u_%02u%02u%02u_%lu.dmp",
                g_DiagnosticsDirectory, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, ::GetCurrentThreadId());
            ::StringCchPrintfW(crashLogPath, _countof(crashLogPath), L"%ls\\Crash_%04u%02u%02u_%02u%02u%02u_%lu.txt",
                g_DiagnosticsDirectory, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, ::GetCurrentThreadId());
            HANDLE crashLog = ::CreateFileW(crashLogPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (crashLog != INVALID_HANDLE_VALUE)
            {
                wchar_t line[512]{};
                const DWORD code = exception && exception->ExceptionRecord ? exception->ExceptionRecord->ExceptionCode : 0;
                const void* address = exception && exception->ExceptionRecord ? exception->ExceptionRecord->ExceptionAddress : nullptr;
                const int length = _snwprintf_s(line, _countof(line), _TRUNCATE,
                    L"Unhandled native exception 0x%08X at %p\r\nProcess: %lu Thread: %lu\r\nTime: %04u-%02u-%02u %02u:%02u:%02u\r\n",
                    code, address, ::GetCurrentProcessId(), ::GetCurrentThreadId(), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
                if (length > 0) { DWORD written = 0; ::WriteFile(crashLog, line, (DWORD)(length * sizeof(wchar_t)), &written, nullptr); }
                ::CloseHandle(crashLog);
            }
            SnapshotExternalLogs();
            HANDLE dump = ::CreateFileW(dumpPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (dump != INVALID_HANDLE_VALUE)
            {
                MINIDUMP_EXCEPTION_INFORMATION info{};
                info.ThreadId = ::GetCurrentThreadId();
                info.ExceptionPointers = exception;
                info.ClientPointers = FALSE;
                const BOOL dumpWritten = ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), dump,
                    (MINIDUMP_TYPE)(MiniDumpWithThreadInfo | MiniDumpWithIndirectlyReferencedMemory | MiniDumpWithUnloadedModules | MiniDumpWithPrivateReadWriteMemory),
                    exception ? &info : nullptr, nullptr, nullptr);
                if (!dumpWritten)
                {
                    wchar_t errorLine[128]{};
                    const int errorLength = _snwprintf_s(errorLine, _countof(errorLine), _TRUNCATE,
                        L"MiniDumpWriteDump failed with Win32 error %lu\r\n", ::GetLastError());
                    HANDLE report = ::CreateFileW(crashLogPath, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                    if (report != INVALID_HANDLE_VALUE)
                    {
                        DWORD written = 0;
                        ::WriteFile(report, errorLine, (DWORD)(errorLength * sizeof(wchar_t)), &written, nullptr);
                        ::CloseHandle(report);
                    }
                }
                ::CloseHandle(dump);
            }
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    void RestoreSettings()
    {
        Hax::Log(G->Logger, L"Restore Settings requested; inGame=%d client=%d shop=%d", G->IsInGame, G->IsClient, G->IsInShop);
        G->UseConsole = false; G->DarkenBackground = false; G->HideHostOnlyOptions = false; G->EspMatchFrameRate = false; G->EspUpdateRate = 10; G->EspFrameInterval = 1; G->Language = Lang_Eng; G->VkOpenClose = VK_OEM_3;
        G->Godmode = G->InfStamina = G->InfJumps = G->NoTumble = G->EasyGrab = G->UnlimGrabRange = false;
        G->GrabThrough = G->GrabAssist = false; G->AntiMusicBoxSpin = G->AntiDuck = false; G->GrabAssistRadius = 64;
        G->PerfDisableShadows = G->PerfDisableAntiAliasing = G->PerfDisablePixelLights = G->PerfDisableLodBias = true;
        G->PerfDisableFog = false;
        G->PerfDisableSoftParticles = G->PerfDisableSoftVegetation = G->PerfDisableReflectionProbes = G->PerfDisablePostProcessing = true;
        G->ShopInventory = false;
        G->FpsCounter = false; G->FpsCorner = 1; G->FpsFontSize = 12;
        G->NoOvercharge = G->AutoUseUpgrs = G->HealToMax = false;
        G->Accel.Walking = G->Accel.Sprinting = G->Accel.Crouching = 1;
        for (auto& upgrade : G->UpgradesData) upgrade.AutoUse = false;
        G->NoGrabMaxTime = G->EnemiesEsp = G->PlayersEsp = G->PlayersChams = false;
        G->ChatPrefered = ChatPref::None;
        G->BetterVision = G->ThirdPerson = G->MaxHeadBattery = false; G->Fov = 60;
        G->Flashlight.Intensity = 10; G->Flashlight.Angle = 60; G->Flashlight.InCrouch = false;
        G->ValuablesEsp = G->ValuablesChams = G->Unbreakable = G->ExtrPointsEsp = false; G->ValuablesEspRange = 20; G->HideStoredValuables = true;
        G->InfBattery = G->UseLaser = false; G->TruckEsp = false;
        G->UnlockAllCosmetic = G->ResetAllCosmetic = G->CosmeticBoxesEsp = false;
        G->EnemyToSpawn = nullptr; G->PlayerToTumble = G->PlayerToRevive = G->PlayerToKill = nullptr;
        G->UpgradeToToggle = nullptr; G->AidToToggle = nullptr; G->ItemToSpawn = nullptr;
        G->ValuableToSpawn = nullptr; G->SpawnMoneyValue = 0;
        G->TeleportToTruck = G->ForceNextLevel = G->SetToZero = G->SetToMax = G->ActivateNextPoint = false;
        G->ApplyIdentityName = G->RandomizeAvatar = false;
        G->LoadIdentityName = false;
        G->SelectedManageEnemy = null; G->AggroTargetPlayer = null; G->EnemyAction = 0;
        G->Loc = g_LocDict[G->Language];
        Hax::IniSave(G->Config);
        Hax::Log(G->Logger, L"Restore Settings completed and configuration saved");
    }

    static void SetupConfig(Hax::IniFile& ini);
    static void WaitForUnityToLoad();

    void Initialize(HMODULE hCheatDll)
    {
        if (!Hax::Unity::IsUnityProcess())
            return;

        G = new Context();
        G->Handle = hCheatDll;

        SetupConfig(G->Config);
        wchar_t tempPath[MAX_PATH]{};
        wchar_t logPath[MAX_PATH]{};
        if (::GetTempPathW(_countof(tempPath), tempPath) > 0)
        {
            SYSTEMTIME time{};
            ::GetLocalTime(&time);
            ::StringCchPrintfW(g_DiagnosticsDirectory, _countof(g_DiagnosticsDirectory),
                L"%lsRepoHaxDiagnostics\\Session_%04u%02u%02u_%02u%02u%02u_%lu",
                tempPath, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, ::GetCurrentProcessId());
            ::CreateDirectoryW(g_DiagnosticsDirectory, nullptr);
            ::StringCchPrintfW(logPath, _countof(logPath), L"%ls\\RepoHax.log", g_DiagnosticsDirectory);
            Hax::InitLogFile(G->Logger, logPath, G->UseConsole);
        }
        else
            Hax::InitLogFile(G->Logger, L"haxsdk_logs.txt", G->UseConsole);
        ::SetUnhandledExceptionFilter(RepoHaxCrashHandler);
        Hax::Log(G->Logger, L"RepoHax diagnostics started; version 0.4.5.4 [PATCH]");
        wchar_t steamRelaunched[8]{};
        if (::GetEnvironmentVariableW(L"REPOHAX_STEAM_RELAUNCHED", steamRelaunched, _countof(steamRelaunched)))
        {
            ::InterlockedExchange(&g_SteamAutoRelaunch, 1);
            Hax::Log(G->Logger, L"Steam bootstrap relaunch guard detected: REPO was restarted after Steam startup; a second automatic restart is disabled.");
        }
        wchar_t modulePath[MAX_PATH]{};
        ::GetModuleFileNameW(hCheatDll, modulePath, _countof(modulePath));
        Hax::Log(G->Logger, L"PID=%lu TID=%lu module=%ls", ::GetCurrentProcessId(), ::GetCurrentThreadId(), modulePath);
        Hax::Log(G->Logger, L"Diagnostics directory: %ls", g_DiagnosticsDirectory);
        SnapshotExternalLogs();

        uintptr_t diagnosticThread = _beginthreadex(nullptr, 0, DiagnosticTailWorker, nullptr, 0, nullptr);
        if (diagnosticThread)
        {
            ::CloseHandle((HANDLE)diagnosticThread);
            Hax::Log(G->Logger, L"Live diagnostic tail snapshots enabled (2 second interval; 1 MiB per file)");
        }
        else
            Hax::LogWarning(G->Logger, L"Unable to start live diagnostic tail worker; startup/crash snapshots remain enabled");

        uintptr_t benchmarkThread = _beginthreadex(nullptr, 0, BenchmarkWorker, nullptr, 0, nullptr);
        if (benchmarkThread) { ::CloseHandle((HANDLE)benchmarkThread); Hax::Log(G->Logger, L"Benchmark CSV worker ready"); }
        else Hax::LogWarning(G->Logger, L"Unable to start benchmark CSV worker");

        WaitForUnityToLoad();
        Hax::Unity::Initialize(&G->Logger);

        HookWinProcs();

        HMODULE hDirectX11 = 0;
        do 
        { 
            Sleep(200); 
            hDirectX11 = ::GetModuleHandleW(L"d3d11.dll"); 
        } while (hDirectX11 == 0);

        HookGraphics();
        HookGameFuncs();

        while (!G->PlayerLoop.ReadyToHook) 
            Sleep(200);
        HookPlayerLoop();
    }

    template <int Default, int Min, int Max>
    static void IniFileRead_IntClamped(void* data, const char* str)
    {
        int value = Default;
        if (sscanf_s(str, "%d", &value) != 1)
            value = Default;

        *(int*)data = Hax::Clamp(value, Min, Max);
    }

    static void SetupConfig(Hax::IniFile& ini)
    {
        Hax::IniAddEntry(ini, "Settings",   "UseConsole",          &G->UseConsole,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Settings",   "VkOpenClose",         &G->VkOpenClose,         Hax::IniFileWrite_Int,  IniFileRead_IntClamped<VK_OEM_3, 1, 255>);
        Hax::IniAddEntry(ini, "Settings",   "Language",            &G->Language,            Hax::IniFileWrite_Int,  IniFileRead_IntClamped<0, 0, Lang_Count - 1>);
        Hax::IniAddEntry(ini, "Settings",   "DarkenBackground",    &G->DarkenBackground,    Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Settings",   "HideHostOnlyOptions", &G->HideHostOnlyOptions, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Settings",   "EspMatchFrameRate",  &G->EspMatchFrameRate, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Settings",   "EspUpdateRate",       &G->EspUpdateRate,       Hax::IniFileWrite_Int, IniFileRead_IntClamped<10, 10, 60>);
        Hax::IniAddEntry(ini, "Settings",   "EspFrameInterval",    &G->EspFrameInterval, Hax::IniFileWrite_Int, IniFileRead_IntClamped<1, 1, 10>);

        Hax::IniAddEntry(ini, "Stats",      "Godmode",             &G->Godmode,             Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "InfiniteStamina",     &G->InfStamina,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "WalkingSpeed",        &G->Accel.Walking,       Hax::IniFileWrite_Int,  IniFileRead_IntClamped<1, 1, 5>);
        Hax::IniAddEntry(ini, "Stats",      "SprintingSpeed",      &G->Accel.Sprinting,     Hax::IniFileWrite_Int,  IniFileRead_IntClamped<1, 1, 5>);
        Hax::IniAddEntry(ini, "Stats",      "CrouchingSpeed",      &G->Accel.Crouching,     Hax::IniFileWrite_Int,  IniFileRead_IntClamped<1, 1, 5>);
        Hax::IniAddEntry(ini, "Stats",      "InfiniteJumps",       &G->InfJumps,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "NoTumble",            &G->NoTumble,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "EasyGrab",            &G->EasyGrab,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "UnlimitedGrabRange",  &G->UnlimGrabRange,      Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Grab",       "GrabThrough",         &G->GrabThrough,         Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Grab",       "GrabAssist",          &G->GrabAssist,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Grab",       "AntiMusicBoxSpin",    &G->AntiMusicBoxSpin,    Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Grab",       "AntiDuck",            &G->AntiDuck,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Grab",       "GrabAssistRadius",    &G->GrabAssistRadius,    Hax::IniFileWrite_Int,  IniFileRead_IntClamped<64, 25, 600>);
        Hax::IniAddEntry(ini, "Performance", "DisableShadows",      &G->PerfDisableShadows,  Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableAntiAliasing", &G->PerfDisableAntiAliasing, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisablePixelLights",  &G->PerfDisablePixelLights, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableLodBias",      &G->PerfDisableLodBias,  Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableFog",          &G->PerfDisableFog,      Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableSoftParticles", &G->PerfDisableSoftParticles, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableSoftVegetation", &G->PerfDisableSoftVegetation, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisableReflectionProbes", &G->PerfDisableReflectionProbes, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "DisablePostProcessing", &G->PerfDisablePostProcessing, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "FpsCounter",         &G->FpsCounter,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Performance", "FpsCorner",          &G->FpsCorner,           Hax::IniFileWrite_Int,  IniFileRead_IntClamped<1, 0, 3>);
        Hax::IniAddEntry(ini, "Performance", "FpsFontSize",        &G->FpsFontSize,         Hax::IniFileWrite_Int,  IniFileRead_IntClamped<12, 8, 20>);
        Hax::IniAddEntry(ini, "Stats",      "NoOverCharge",        &G->NoOvercharge,        Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Stats",      "AutoUseUpgrades",     &G->AutoUseUpgrs,        Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        static constexpr const char* s_Names[(int)(UpgradeType::N)] = 
        {
            "CrouchRest", "ExtraJump", "Range", "SprintSpeed", "Stamina", "Strength",
            "Launch", "Wings", "Health", "PlayersCount", "Climb", "HeadBattery"
        };

        for (int i = 0; i < (int)UpgradeType::N; ++i)
            Hax::IniAddEntry(ini, "UpgradeAutoUse", s_Names[i], &G->UpgradesData[i].AutoUse, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniAddEntry(ini, "Entities",   "NoGrabMaxTime",       &G->NoGrabMaxTime,       Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Entities",   "EnemiesEsp",          &G->EnemiesEsp,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Entities",   "PlayersEsp",          &G->PlayersEsp,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Entities",   "PlayersChams",        &G->PlayersChams,        Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniAddEntry(ini, "Vision",     "BetterVision",        &G->BetterVision,        Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Vision",     "ThirdPerson",         &G->ThirdPerson,         Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Vision",     "Fov",                 &G->Fov,                 Hax::IniFileWrite_Int,  IniFileRead_IntClamped<60, 60, 140>);
        Hax::IniAddEntry(ini, "Vision",     "FlashlightIntensity", &G->Flashlight.Intensity,Hax::IniFileWrite_Int,  IniFileRead_IntClamped<10, 10, 20>);
        Hax::IniAddEntry(ini, "Vision",     "FlashAngle",          &G->Flashlight.Angle,    Hax::IniFileWrite_Int,  IniFileRead_IntClamped<60, 60, 120>);
        Hax::IniAddEntry(ini, "Vision",     "FlashInCrouch",       &G->Flashlight.InCrouch, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Vision",     "MaxHeadBattery",      &G->MaxHeadBattery,      Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniAddEntry(ini, "Valuables",  "ValuablesEsp",        &G->ValuablesEsp,        Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Valuables",  "ValuablesEspRange",   &G->ValuablesEspRange,   Hax::IniFileWrite_Int,  IniFileRead_IntClamped<20, 5, 500>);
        Hax::IniAddEntry(ini, "Valuables",  "HideStoredValuables", &G->HideStoredValuables, Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Valuables",  "ValuablesChams",      &G->ValuablesChams,      Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Valuables",  "Unbreakable",         &G->Unbreakable,         Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Valuables",  "ExtrPointsEsp",       &G->ExtrPointsEsp,       Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniAddEntry(ini, "Items",      "InfBattery",          &G->InfBattery,          Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Items",      "UseLaser",            &G->UseLaser,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);
        Hax::IniAddEntry(ini, "Items",      "ShopInventory",       &G->ShopInventory,       Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniAddEntry(ini, "Misc",       "TruckEsp",            &G->TruckEsp,            Hax::IniFileWrite_Bool, Hax::IniFileRead_Bool);

        Hax::IniLoad(G->Config);
        G->Loc = g_LocDict[G->Language];
    }

    static void WaitForUnityToLoad()
    {
        Hax::Log(G->Logger, L"Waiting for unity virtual machine...");
        while (!Hax::Unity::GetUvmHandle())
            ::Sleep(200);

        HANDLE hEvent = ::CreateEvent(0, TRUE, FALSE, nullptr);
        HAX_PANIC(hEvent != nullptr, &G->Logger, L"%zu", HAX_LINE);

        G->UnityLoadedEvent = hEvent;
        HookMonoRuntimeInvoke();

        Hax::Log(G->Logger, L"Waiting for unity...");
        ::WaitForSingleObject(hEvent, INFINITE);
        ::CloseHandle(hEvent);
    }
}

