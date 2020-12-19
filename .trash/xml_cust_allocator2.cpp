#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cassert>
#include <functional>

template<size_t TBytes, size_t Num>
struct Pool {
  enum : size_t { Bytes = TBytes };
  using slot = int64_t[(TBytes+sizeof(int64_t)-1)/sizeof(int64_t)];

  unsigned _i;
  short _free[Num];
  slot data[Num];

  Pool() : _i(Num) { int slot = -1; while (_i--) _free[_i] = ++slot; }

  inline slot* end()   { return &data[Num]; }
  inline slot* begin() { return &data[0]; }

  void* allocate() {
    if (_i < Num)
      return data[_free[_i++]];
    return NULL;
  }

  inline bool owns(void* ptr) {
    slot* p = reinterpret_cast<slot*>(ptr);
    return (&data[0] <= p && p < &data[Num]);
  }

  inline void deallocate(void* ptr) {
    slot* p = reinterpret_cast<slot*>(ptr);
    _free[--_i] = p - &data[0];
  }
};

#include<map>

struct Tracer {
  std::string name;
  std::map<int, int> log;
  std::map<int, int> log_peak;

  Tracer(std::string name_) : name(std::move(name_)) {}

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

//#define ENABLE_LOG

#ifdef ENABLE_LOG
Tracer mallocTrace("malloc");
#define IF_LOG_ENABLED(...) __VA_ARGS__
#else
#define IF_LOG_ENABLED(...) 0
#endif

#if 0
  X(32,    4096) \

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

struct Allocator {
#define X(SIZE, NUM) Pool<SIZE, NUM> p##SIZE;
  POOLS
#undef X

  void* allocate(size_t n) {
    void* ptr;
#define X(SIZE,_) if (n <= SIZE) { if ((ptr = p##SIZE.allocate())) return ptr; }
    POOLS
#undef X
#ifdef ENABLE_LOG
    mallocTrace.allocate(NULL, n);
#endif
    return malloc(n);
  }

  void deallocate(void* ptr) {
#define X(SIZE,_) if (p##SIZE.owns(ptr)) { p##SIZE.deallocate(ptr); return; }
    POOLS
#undef X
    free(ptr);
  }

  void* reallocate(void* ptr, size_t n) {
    size_t slot_size;
    if (0){}
#define X(SIZE,_) else if (p##SIZE.owns(ptr)) { slot_size = SIZE; }
    POOLS
#undef X
    else {
#ifdef ENABLE_LOG
      mallocTrace.allocate(NULL, n);
#endif
      return realloc(ptr, n);
    }

    if (n <= slot_size) {
      return ptr;
    }

    void* new_ptr = allocate(n);
    std::memcpy(new_ptr, ptr, std::min(slot_size, n));
    deallocate(ptr);
    return new_ptr;
  }
};

struct AllocationProxy {
  std::map<void*, size_t> allocations;

  std::function<void*(size_t)> real_allocate;
  std::function<void*(void*, size_t)> real_reallocate;
  std::function<void(void*)> real_deallocate;

  std::function<void(void*, size_t)> on_allocated;
  std::function<void(void*, size_t)> on_deallocated;

  AllocationProxy(
    std::function<void*(size_t)> real_allocate_,
    std::function<void*(void*, size_t)> real_reallocate_,
    std::function<void(void*)> real_deallocate_,

    std::function<void(void*, size_t)> on_allocated_,
    std::function<void(void*, size_t)> on_deallocated_
  )
  : real_allocate(real_allocate_)
  , real_reallocate(real_reallocate_)
  , real_deallocate(real_deallocate_)

  , on_allocated(on_allocated_)
  , on_deallocated(on_deallocated_)
  {}

  void deallocate(void* p) {
    real_deallocate(p);
    on_deallocated(p, allocations[p]);
    allocations.erase(p);
  }

  void* allocate(size_t n) {
    void* p = real_allocate(n);
    on_allocated(p, n);
    allocations[p] = n;
    return p;
  }

  void* reallocate(void* p, size_t n) {
    on_deallocated(p, allocations[p]);
    allocations.erase(p);
    p = real_reallocate(p, n);
    on_allocated(p, n);
    allocations[p] = n;
    return p;
  }
};

static Allocator allocator;

#ifdef ENABLE_LOG
static Tracer tracer("General allocs");
static AllocationProxy proxy(
    [](size_t n)          { return allocator.allocate(n); },
    [](void* p, size_t n) { return allocator.reallocate(p, n); },
    [](void* p)           { allocator.deallocate(p); },
    [](void* p, size_t n) { tracer.allocate(p, n); },
    [](void* p, size_t n) { tracer.deallocate(p, n); }
);
#define allocator proxy
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

#undef allocator

#if 0
int main() {
  Pool<3, 16> p;

  vector<void*> ptrs;

  for (int i = 0; i < 3; ++i) {
    void* ptr = p.allocate();
    ptrs.push_back(ptr);
    std::sprintf((char*) ptr, "%s", "halloduda");
  }

  assert(!p.allocate());

  for (auto ptr : ptrs)
    p.deallocate(ptr);

  for (int i = 0; i < 3; ++i) {
    void* ptr = p.allocate();
    ptrs.push_back(ptr);
    std::sprintf((char*) ptr, "%s", "halloduda");
  }
}

using namespace std;
static char alloc_pool[1024*1024*50];
static char* alloc_next;
#endif


#if 0
class XMLAllocator {
  XMLAllocator() {
    xmlMemSetup();
  }

  ~XMLAllocator() {

  }

  void free(void*) { }
  void*malloc(size_t n) {}
  void*realloc(void*,size_t n) {}
  char*strdup(const char*) {}
};
#endif

