#ifndef ALLOCA_
#define ALLOCA_

#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cassert>
#include <functional>
#include <cstring>

struct StandardAllocator {
  inline void*  allocate(size_t n)            noexcept { return malloc(n);     }
  inline void   deallocate(void* p)           noexcept { free(p);              }
  inline void*  reallocate(void* p, size_t n) noexcept { return realloc(p, n); }
  inline size_t size(void*)                   noexcept { return SIZE_MAX;      }
  inline void   reset()                       noexcept { }
};

//template<size_t lhs> bool gt(size_t rhs)                { return lhs > rhs; }
//template<size_t lhs> bool le(size_t rhs)                { return lhs < rhs; }

template<size_t rhs>             struct le      { inline bool operator()(size_t lhs) { return lhs <= rhs; } };
template<size_t min, size_t max> struct between { inline bool operator()(size_t n)   { return min <= n && n <= max; } };

template<size_t TMinBytes, size_t TNum, class Predicate, class TNext>
struct Pool {
  using slot = int64_t[(TMinBytes+sizeof(int64_t)-1)/sizeof(int64_t)];
  enum : size_t { slot_size = sizeof(slot) };

  TNext next;
  int _i;
  short _free[TNum];
  slot data[TNum];

  //Pool() : _i(TNum) { int slot = -1; while (_i--) _free[_i] = ++slot; } << old, better?
  Pool() : _i(TNum) { reset(); }

  inline void reset() { for (_i = 0; _i < TNum; ++_i) _free[_i] = _i; } 

  inline slot* end()      { return &data[TNum]; }
  inline slot* begin()    { return &data[0]; }

  void* allocate(size_t n) {
    //if (n <= slot_size)
    if (Predicate()(n))
      if (_i > 0)
        return data[_free[--_i]];
    return next.allocate(n);
  }

  inline bool owns(void* ptr) {
    slot* p = reinterpret_cast<slot*>(ptr);
    return (&data[0] <= p && p < &data[TNum]);
  }

  inline size_t size(void* ptr) {
    if (owns(ptr))
      return slot_size;
    return next.size(ptr);
  }

  inline void deallocate(void* ptr) {
    if (owns(ptr))
      do_deallocate(ptr);
    else
      next.deallocate(ptr);
  }

  void* reallocate(void* ptr, size_t n) {
    if (owns(ptr)) {
      if (n <= slot_size)
        return ptr;

      void* new_ptr = next.allocate(n);
      std::memcpy(new_ptr, ptr, slot_size);
      do_deallocate(ptr);
      return new_ptr;
    }

    return next.reallocate(ptr, n);
  }

private:
  inline void do_deallocate(void* ptr) {
    slot* p = reinterpret_cast<slot*>(ptr);
    _free[_i++] = p - &data[0];
  }
};

#include<map>

#if 0
struct Tracer {
  std::string name;
  std::map<int, int> log;
  std::map<int, int> log_peak;

  Tracer(std::string name_) : name(std::move(name_)) {}

  void allocate(void*, size_t n) {
    log[fitInPow2(n)]++;
    log_peak[fitInPow2(n)] = std::max(log_peak[fitInPow2(n)], log[fitInPow2(n)]);
  }

  void print() {
    log_write("%s: intern allocated .............\n", name.c_str());
     for (const auto& i : log)
       if (i.second)
         log_write("%d: %d\n", i.first, i.second);
         //std::cout << i.first << i.second << std::endl;
    log_write("%s: intern peak ..................\n", name.c_str());
     for (const auto& i : log_peak)
       if (i.second)
         log_write("%d: %d\n", i.first, i.second);
         //std::cout << i.first << i.second << std::endl;
  }

  void reset() {
    log.clear();
    log_peak.clear();
  }

  static inline int fitInPow2(size_t n) {
    for (int i = 4; i < 32; ++i)
      if (n <= (1<<i))
        return 1<<i;
  }

  void allocate(void*, size_t n) {
    log[fitInPow2(n)]++;
    log_peak[fitInPow2(n)] = std::max(log_peak[fitInPow2(n)], log[fitInPow2(n)]);
  }

  void deallocate(void*, size_t n) {
    log[fitInPow2(n)]--;
  }
};
#endif

//#define ENABLE_LOG

#ifdef ENABLE_LOG
Tracer mallocTrace("malloc");
#define IF_LOG_ENABLED(...) __VA_ARGS__
#else
#define IF_LOG_ENABLED(...) 0
#endif

#define _POOLS \
  X(128,   8096) \
  X(256,   2048) \
  X(512,   128)  \
  X(2048,  128)  \
  X(4096,  128)  \
  X(32768, 16)

#define __POOLS \
  X(64,    4024) \
  X(128,   4096) \
  X(256,   2048) \
  X(512,   128)  \
  X(4096,  128)  \
  X(32768, 16)

#define POOLS \
  X(256,   8048) \
  X(4096,  4024)  \
  X(32768, 16)

template<class TAllocator>
struct AllocationTracer {
  TAllocator real;

  std::map<int, int> log;

  ~AllocationTracer() {
    print();
  }

  void print() {
    log_write("intern allocated .............\n");
     for (const auto& i : log)
       if (i.second > 5000)
         log_write("%08d: %d\n", i.first, i.second);
  }

  void deallocate(void* p) { real.deallocate(p); }

  void* allocate(size_t n) {
    //n = fitInPow2(n);
    log[n]++;
    return real.allocate(n);
  }

  void* reallocate(void* p, size_t n) {
    //n = fitInPow2(n);
    log[n]++;
    return real.reallocate(p, n);
  }

  static inline int fitInPow2(size_t n) {
    for (int i = 4; i < 32; ++i)
      if (n <= (1<<i))
        return 1<<i;
  }
};

#if 0
static
//AllocationTracer<
  Pool<128,   8096, le<128>,
  Pool<256,   2048, le<256>,
  Pool<512,   128,  le<512>,
  Pool<2048,  128,  le<2048>,
  Pool<4096,  128,  le<4096>,
  Pool<32768, 16,   between<8192, 32768>,
  StandardAllocator>>>>>> allocator;
#else
static
//AllocationTracer<
  Pool<128,   8096, le<128>, StandardAllocator> allocator;
#endif

static void* myMalloc(size_t n)           { return allocator.allocate(n); }
static void* myRealloc(void* p, size_t n) { return allocator.reallocate(p, n); }
static void  myFree(void* p)              { return allocator.deallocate(p); }
static char* myStrdup(const char*s) {
  size_t n = strlen(s);
  void*p = allocator.allocate(n);
  std::memcpy(p, s, n+1);
  return reinterpret_cast<char*>(p);
}

#endif
