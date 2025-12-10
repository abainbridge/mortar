//#include <windows.h>
typedef __int64 LONGLONG;

typedef union _LARGE_INTEGER {
    struct {
        unsigned LowPart;
        long HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        unsigned LowPart;
        long HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER; 

__declspec(dllimport) int __stdcall QueryPerformanceCounter(LARGE_INTEGER *performanceCount);
__declspec(dllimport) int __stdcall QueryPerformanceFrequency(LARGE_INTEGER *frequency);


static double g_time_shift = 0.0;
static double g_tick_interval = -1.0;


double get_low_level_time() {
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart * g_tick_interval;
}

static void init_time() {
    LARGE_INTEGER count;
    QueryPerformanceFrequency(&count);
    g_tick_interval = 1.0 / (double)count.QuadPart;
    g_time_shift = get_low_level_time();
}

double get_time() {
    if (g_time_shift <= 0.0) {
        init_time();
    }

    double time_now = get_low_level_time();
    time_now -= g_time_shift;
    return time_now;
}
