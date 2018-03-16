#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <xmmintrin.h> // SSE
#include <immintrin.h> // AVX

#ifdef _WIN32

#include <intrin.h> // for __movsb, __movsd, __movsq
#include <windows.h>

static uint64_t GetTicks()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t.QuadPart;
}

static uint64_t GetFreq()
{
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return f.QuadPart;
}

#else

#include <sys/mman.h>
#include <time.h>
#include <pthread.h>

static uint64_t GetTicks()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static uint64_t GetFreq()
{
    return 1000000000ULL;
}

static void __movsb(void* dst, const void* src, size_t size)
{
    __asm__ __volatile__("rep movsb" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
}

static void __movsd(void* dst, const void* src, size_t size)
{
    __asm__ __volatile__("rep movsl" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
}

static void __movsq(void* dst, const void* src, size_t size)
{
    __asm__ __volatile__("rep movsq" : "+D"(dst), "+S"(src), "+c"(size) : : "memory");
}

#endif

static void CopyWithRepMovsb(uint8_t* dst, uint8_t* src, size_t size)
{
    __movsb(dst, src, size);
}

static void CopyWithRepMovsbUnaligned(uint8_t* dst, uint8_t* src, size_t size)
{
    __movsb(dst + 1, src + 1, size - 1);
}

// size must be multiple of 4 bytes
static void CopyWithRepMovsd(uint8_t* dst, uint8_t* src, size_t size)
{
    __movsd((unsigned long*)dst, (unsigned long*)src, size / sizeof(uint32_t));
}

// size must be multiple of 8 bytes
static void CopyWithRepMovsq(uint8_t* dst, uint8_t* src, size_t size)
{
    __movsq((uint64_t*)dst, (uint64_t*)src, size / sizeof(uint64_t));
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*8 = 128 bytes
static void CopyWithSSE(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 8 * sizeof(__m128i);
    while (size)
    {
        __m128 a = _mm_load_ps((float*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((float*)(src + 1*sizeof(__m128)));
        __m128 c = _mm_load_ps((float*)(src + 2*sizeof(__m128)));
        __m128 d = _mm_load_ps((float*)(src + 3*sizeof(__m128)));
        __m128 e = _mm_load_ps((float*)(src + 4*sizeof(__m128)));
        __m128 f = _mm_load_ps((float*)(src + 5*sizeof(__m128)));
        __m128 g = _mm_load_ps((float*)(src + 6*sizeof(__m128)));
        __m128 h = _mm_load_ps((float*)(src + 7*sizeof(__m128)));
        _mm_store_ps((float*)(dst + 0*sizeof(__m128)), a);
        _mm_store_ps((float*)(dst + 1*sizeof(__m128)), b);
        _mm_store_ps((float*)(dst + 2*sizeof(__m128)), c);
        _mm_store_ps((float*)(dst + 3*sizeof(__m128)), d);
        _mm_store_ps((float*)(dst + 4*sizeof(__m128)), e);
        _mm_store_ps((float*)(dst + 5*sizeof(__m128)), f);
        _mm_store_ps((float*)(dst + 6*sizeof(__m128)), g);
        _mm_store_ps((float*)(dst + 7*sizeof(__m128)), h);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
static void CopyWithSSESmall(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 2 * sizeof(__m128);
    while (size)
    {
        __m128 a = _mm_load_ps((float*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((float*)(src + 1*sizeof(__m128)));
        _mm_store_ps((float*)(dst + 0*sizeof(__m128)), a);
        _mm_store_ps((float*)(dst + 1*sizeof(__m128)), b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
static void CopyWithSSENoCache(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 2 * sizeof(__m128);
    while (size)
    {
        __m128 a = _mm_load_ps((float*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((float*)(src + 1*sizeof(__m128)));
        _mm_stream_ps((float*)(dst + 0*sizeof(__m128)), a);
        _mm_stream_ps((float*)(dst + 1*sizeof(__m128)), b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*16 = 512 bytes
static void CopyWithAVX(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 16 * sizeof(__m256i);
    while (size)
    {
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        __m256i c = _mm256_load_si256((__m256i*)src + 2);
        __m256i d = _mm256_load_si256((__m256i*)src + 3);
        __m256i e = _mm256_load_si256((__m256i*)src + 4);
        __m256i f = _mm256_load_si256((__m256i*)src + 5);
        __m256i g = _mm256_load_si256((__m256i*)src + 6);
        __m256i h = _mm256_load_si256((__m256i*)src + 7);
        __m256i i = _mm256_load_si256((__m256i*)src + 8);
        __m256i j = _mm256_load_si256((__m256i*)src + 9);
        __m256i k = _mm256_load_si256((__m256i*)src + 10);
        __m256i l = _mm256_load_si256((__m256i*)src + 11);
        __m256i m = _mm256_load_si256((__m256i*)src + 12);
        __m256i n = _mm256_load_si256((__m256i*)src + 13);
        __m256i o = _mm256_load_si256((__m256i*)src + 14);
        __m256i p = _mm256_load_si256((__m256i*)src + 15);
        _mm256_store_si256((__m256i*)dst + 0, a);
        _mm256_store_si256((__m256i*)dst + 1, b);
        _mm256_store_si256((__m256i*)dst + 2, c);
        _mm256_store_si256((__m256i*)dst + 3, d);
        _mm256_store_si256((__m256i*)dst + 4, e);
        _mm256_store_si256((__m256i*)dst + 5, f);
        _mm256_store_si256((__m256i*)dst + 6, g);
        _mm256_store_si256((__m256i*)dst + 7, h);
        _mm256_store_si256((__m256i*)dst + 8, i);
        _mm256_store_si256((__m256i*)dst + 9, j);
        _mm256_store_si256((__m256i*)dst + 10, k);
        _mm256_store_si256((__m256i*)dst + 11, l);
        _mm256_store_si256((__m256i*)dst + 12, m);
        _mm256_store_si256((__m256i*)dst + 13, n);
        _mm256_store_si256((__m256i*)dst + 14, o);
        _mm256_store_si256((__m256i*)dst + 15, p);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*2 = 64 bytes
static void CopyWithAVXSmall(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 2 * sizeof(__m256i);
    while (size)
    {
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        _mm256_store_si256((__m256i*)dst + 0, a);
        _mm256_store_si256((__m256i*)dst + 1, b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*2 = 64 bytes
static void CopyWithAVXNoCache(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t stride = 2 * sizeof(__m256i);
    while (size)
    {
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        _mm256_stream_si256((__m256i*)dst + 0, a);
        _mm256_stream_si256((__m256i*)dst + 1, b);

        size -= stride;
        src += stride;
        dst += stride;
    }
}

const size_t kThreadCount = 2;

struct ThreadWorkData
{
    uint8_t* src;
    uint8_t* dst;
    size_t size;

    volatile bool RunThread;
};

static ThreadWorkData ThreadData[kThreadCount];
static volatile long ThreadsReady;

#ifdef _WIN32

static DWORD WINAPI ThreadProc(LPVOID Arg)
{
    size_t ThreadIndex = (size_t)Arg;
    ThreadWorkData* MyData = &ThreadData[ThreadIndex];
    for (;;)
    {
        while (!MyData->RunThread)
        {
        }
        CopyWithRepMovsb(MyData->dst, MyData->src, MyData->size);
        _InterlockedIncrement(&ThreadsReady);
        MyData->RunThread = false;
    } 
    return 0;
}

static void SetupThreads()
{
    for (size_t i=0; i<kThreadCount; i++)
    {
        CreateThread(0, 0, ThreadProc, (LPVOID)i, 0, 0);
    }
}

#else

static void* ThreadProc(void* Arg)
{
    size_t ThreadIndex = (size_t)Arg;
    ThreadWorkData* MyData = &ThreadData[ThreadIndex];
    for (;;)
    {
        while (!MyData->RunThread)
        {
        }
        CopyWithSSENoCache(MyData->dst, MyData->src, MyData->size);
        __sync_add_and_fetch(&ThreadsReady, 1);
        MyData->RunThread = false;
    } 
    return 0;
}

static void SetupThreads()
{
    for (size_t i=0; i<kThreadCount; i++)
    {
        pthread_t thread;
        pthread_create(&thread, 0, ThreadProc, (void*)i);
    }
}

#endif

// dst and src must be 32-byte aligned
// size must be multiple of 32*2*kThreadCount = 64*kThreadCoutn bytes
static void CopyWithThreads(uint8_t* dst, uint8_t* src, size_t size)
{
    size_t size1 = size / kThreadCount;

    ThreadsReady = 0;
    for (size_t i=0; i<kThreadCount; i++)
    {
        ThreadData[i].dst = dst;
        ThreadData[i].src = src;
        ThreadData[i].size = size1;
        ThreadData[i].RunThread = true;

        dst += size1;
        src += size1;
    }

    while (ThreadsReady != kThreadCount)
    {
    }
}

#define BENCH(name)                                       \
{                                                         \
    uint64_t t1 = GetTicks();                             \
    for (size_t i=0; i<kCount; i++)                       \
    {                                                     \
        name(dst, src, kSize);                            \
        dummy += dst[kSize - 1];                          \
    }                                                     \
    if (memcmp(dst, src, kSize) != 0)                     \
    {                                                     \
        printf("ERROR: %s\n", #name);                     \
    }                                                     \
    uint64_t t2 = GetTicks();                             \
    double seconds = (double)(t2 - t1) / GetFreq();       \
    printf("%s = %.2f GiB/s\n", #name, kCount / seconds); \
}


int main()
{
    // 1GiB
    const size_t kSize = 1ULL << 30;
#ifdef _WIN32
    uint8_t* src = (uint8_t*)VirtualAlloc(0, kSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    uint8_t* dst = (uint8_t*)VirtualAlloc(0, kSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
#else
    uint8_t* src = (uint8_t*)mmap(0, kSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    uint8_t* dst = (uint8_t*)mmap(0, kSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif

    SetupThreads();

    for (size_t i=0; i<kSize; i++)
    {
        src[i] = (uint8_t)(i + i*i);
    }

    // dummy copy so OS really allocates memory
    memcpy(dst, src, kSize);

    // this is to prevent compiler to optimize out memory copies
    volatile uint8_t dummy = 0;

    const size_t kCount = 8;

    BENCH(memcpy);
    BENCH(CopyWithSSE);
    BENCH(CopyWithSSESmall);
    BENCH(CopyWithSSENoCache);
    BENCH(CopyWithAVX);
    BENCH(CopyWithAVXSmall);
    BENCH(CopyWithAVXNoCache);
    BENCH(CopyWithRepMovsb);
    BENCH(CopyWithRepMovsd);
    BENCH(CopyWithRepMovsq);
    BENCH(CopyWithRepMovsbUnaligned);
    BENCH(CopyWithThreads);
}
