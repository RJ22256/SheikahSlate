// Host OS interface: runtime/platform/android.c, runtime/platform/windows.c (WSword's PC build, from LAS Recomp's).
// Compatibility headers below also support isolated development harnesses on the host.
#pragma once
// WSword: the Android fork's runtime work - BOTW's core 0 discipline and the kernel's race fixes, the dispatch cache,
// the renderer's translation speed-ups - on the PC build too (Windows). A macOS / Linux build of LAS Recomp's desktop
// code keeps LAS's. (Android-only things - the moving guest window, ADPF, Turnip's placed heap - stay __ANDROID__.)
#if defined(__ANDROID__) || defined(_WIN32)
#define WS_FORK 1
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include "platform/win_compat.h"
#elif defined(__linux__) && !defined(__ANDROID__)  // (Bionic has strlcpy and arc4random)
#include "platform/linux_compat.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ---- guest address space
// Claim [lo, hi) for the guest at exactly that address, readable/writable (pages may be backed lazily).
bool plat_reserve_guest(uint64_t lo, uint64_t hi);
// Make sure [addr, addr+size) inside the guest window is backed (before the OS itself writes there, e.g.
// file reads); a no-op where the whole window is backed from the start.
void plat_commit(uint64_t addr, uint64_t size);
// In-place reboot: every page of [lo, hi) back to zero (and unbacked), the window still claimed.
bool plat_reset_guest(uint64_t lo, uint64_t hi);
// Pages of [addr, addr+size) in the guest window the CPU wrote since the last call over them (write tracking: Windows
// MEM_WRITE_WATCH), their addresses into pages[] (at most max), and the tracking reset for the range. Returns the
// count (max: maybe more), or -1 where there is no tracking (macOS, Linux). Page size: *page.
int plat_written_pages(uint64_t addr, uint64_t size, uint64_t *pages, int max, uint64_t *page);
// Android: write tracking made by protecting pages (none until enabled, for the guest range [lo, hi)), and the pages
// of [addr, addr+size) made writable before the OS itself writes there (a read into guest memory can't fault: EFAULT).
// No-ops elsewhere (Windows tracks writes itself).
void plat_write_tracking_enable(uint64_t lo, uint64_t hi);
void plat_write_prepare(uint64_t addr, uint64_t size);
enum { PLAT_PROT_NONE = 0, PLAT_PROT_READ = 1, PLAT_PROT_RW = 3 };
int plat_protect(uint64_t addr, uint64_t size, int prot);  // 0 on success
#define PLAT_PAGE 0x4000ull  // largest page size we deal with (macOS 16K; multiple of Windows' 4K)

// ---- time
uint64_t plat_now_ns(void);                // monotonic
void plat_sleep_until_ns(uint64_t t);      // precise wait for plat_now_ns() >= t
void plat_thread_realtime(uint64_t period_ns);  // periodic timing thread (vsync): don't coalesce its timers
int plat_thread_realtime_check(uint64_t period_ns);  // (the calling thread, after a late wait) 0 the policy holds, 1 it was
                                                     // gone and is set again, -1 can't tell; only macOS looks
int plat_cpu_count(void);  // logical cores this process may run on (its affinity mask); 0 if unknown
void plat_thread_guest(int prio);  // a guest thread's host scheduling class from its Switch priority (lower = more urgent)
uint64_t plat_thread_cpu_ns(void);  // the calling thread's CPU time so far (user + system; diagnostics)
// The calling thread is one of the frame's critical threads: on Windows, no power throttling (EcoQoS) for it and, on a
// hybrid CPU, a preference for the performance cores (selected CPU sets; LARECOMP_PCORE=0: off). Elsewhere a no-op.
enum { PLAT_PERF_GAME, PLAT_PERF_RENDER, PLAT_PERF_VSYNC };
#ifdef _WIN32
void plat_thread_perf_core(int role);
#else
static inline void plat_thread_perf_core(int role) { (void)role; }
#endif

// ---- process
void plat_init(void);                                // first thing in main()
bool plat_exe_path(char *buf, size_t size);
void plat_exec(char *const argv[]);  // replace the process (NULL = same command line); returns only on failure
#if defined(_WIN32) || defined(__linux__)
void plat_terminate(void);  // end the process now, skipping DLL teardown (exit status 0)
#endif
// The machine, for the log's first "[info]" line (runtime/main.c): nothing that identifies the player (no user name,
// no paths). Fields left empty / 0 when unknown.
typedef struct {
    char os[128];          // "Windows 11 24H2 (x64)"
    char cpu[128];
    char cores_detail[48]; // optional, after the core count: "8P + 16E"
    int cores, threads;    // physical cores, hardware threads
    uint64_t ram;          // bytes
    const char *package;   // "zip"; NULL: a build run from its folder
} PlatSysInfo;
void plat_system_info(PlatSysInfo *si);

// Child processes (game code generator, game-file extraction). stdout + stderr go to log_path (NULL = /dev/null).
typedef intptr_t PlatProc;  // 0 = failed to start
PlatProc plat_spawn(const char *const argv[], const char *log_path, bool append_log);
int plat_proc_wait(PlatProc p, bool block);  // exit code (128+n: stopped by signal n), or -1 while still running (block = false)
#define PLAT_PROC_SIGNALED 256               // + N: ended by signal N instead of exiting (not on Windows)
#define PLAT_PROC_FAST_FAIL 512              // Windows: ended by a security check (0xC0000409, e.g. Control Flow Guard)
#ifdef _WIN32
// plat_spawn with Control Flow Guard off for the child (the generator: unicorn's longjmp fails CFG's check, LAS #337).
// *error: 0 = started that way; else why not (then it was started normally, or not at all if that failed too)
PlatProc plat_spawn_no_cfg(const char *const argv[], const char *log_path, bool append_log, unsigned long *error);
// dir is synced by OneDrive (under a OneDrive folder, or a cloud file); false on other systems
bool plat_onedrive_synced(const char *dir);
// Something the player should know that doesn't stop the game: a warning dialog from its own thread
void plat_notice(const char *text);
bool plat_module_name(uint64_t pc, char *out, size_t n);  // the file of the module (exe / library / driver) holding pc
#endif
void plat_proc_kill(PlatProc p);             // end it and what it started (then it's gone: no wait)
bool plat_remove_tree(const char *dir);      // rm -rf
#ifdef _WIN32
#define PLAT_EXE_SUFFIX ".exe"
#define PLAT_VENV_PYTHON "venv/Scripts/python.exe"  // relative to the venv's parent
#else
#define PLAT_EXE_SUFFIX ""
#define PLAT_VENV_PYTHON "venv/bin/python"
#endif

// ---- diagnostics
// Name of the host function containing pc (guest functions are f_<addr>), NULL if unknown.
const char *plat_symbol(uint64_t pc, uint64_t *offset);
void plat_print_backtrace(void);
void plat_fatal_notice(void);  // a fatal error is about to end the process: tell the user (Windows release: message box)
void plat_crash_handler_install(void);  // per thread where the OS needs it (alternate signal stack)
// Called by the crash handler (runtime/dispatch.c): true = fault handled (watchpoint), resume.
bool rt_fault_watch(uint64_t fault_addr, uint64_t host_pc);
void rt_fault_report(const char *what, uint64_t fault_addr, uint64_t host_pc, uint64_t host_lr) __attribute__((noreturn));

#if defined(__ANDROID__) || defined(_WIN32)
// ---- device state for the performance overlay (runtime/ui/perf_overlay.mm); -1 / 0 where unknown (WSword PC:
// platform/windows.c - the process's CPU and memory, the system's free memory, a laptop's battery)
typedef struct {
    int ncluster;                  // CPU clusters (cpufreq policies)
    uint32_t cluster_mhz[8];
    float cpu_pct;                 // this process's CPU time, as a share of all cores
    uint32_t rss_mb, avail_mb;     // this process's memory, the system's available memory
    float soc_temp_c;              // a thermal zone the app may read
    float batt_temp_c, batt_pct, watts;  // from the activity (BatteryManager)
    int thermal;                   // PowerManager thermal status: 0 none ... 6 shutdown
    float headroom;                // PowerManager thermal headroom (1.0 = throttling begins)
} PlatDeviceStats;
void plat_device_stats(PlatDeviceStats *out);
#endif
#ifdef WS_FORK
// ADPF (Android; no-ops on the PC): the calling thread's work for a frame (busy, ns) and its target (the frame time it must fit)
void plat_frame_work(uint64_t work_ns, uint64_t target_ns);
// (the presenting thread: cpu_ns is the frame's critical path - the longer of its own and the render thread's work -,
// game_ns its own part, for the overlay; the GPU's time too, API 35+)
void plat_frame_work_gpu(uint64_t cpu_ns, uint64_t gpu_ns, uint64_t target_ns, uint64_t game_ns);
// ADPF state for the performance overlay (platform/android.c)
typedef struct {
    int state;                 // -2 disabled via env, -1 not available, 0 not yet, 1 on
    int sessions;              // 1: one session for both threads (default), 2: one per thread (LARECOMP_ADPF_SPLIT=1)
    int threads;               // threads in the session
    int errors;                // failed reports
    int last_error;            // last error code
    int recreated;             // sessions created / updated
    float target_ms, work_ms;  // the last target and reported work (the frame's critical path)
    float game_ms, render_ms;  // each thread's work per frame, moving average
    int game_cpu, render_cpu;  // the core each last ran on (-1: unknown)
} PlatAdpfStats;
void plat_adpf_stats(PlatAdpfStats *out);
#ifdef __ANDROID__
// A new thread back on every core: threads inherit the creating thread's affinity, and the presenting thread is
// pinned to the fastest cores (platform/android.c)
void plat_thread_unpin(void);
#endif
#endif

#ifdef __cplusplus
}
#endif
