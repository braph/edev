// ============================================================================
//               c_str() / reinterpret_cast<>() - free code
// ============================================================================

// ============================================================================
// Option 0: Use a converter class as function parameter ======================
// ============================================================================

struct TString {
  const char *s;
  template<class T>
  inline TString(const T& s_)             noexcept : s(s_.c_str()) {}
  inline TString(const char* s_)          noexcept : s(s_)         {}
  inline TString(const signed char* s_)   noexcept : s(reinterpret_cast<const char*>(s_)) {}
  inline TString(const unsigned char* s_) noexcept : s(reinterpret_cast<const char*>(s_)) {}
  inline operator const char*()           noexcept { return s; }
};

void f(TString a, TString b) {
  return strstr(a, b);
}

/*

Pros:
  - No bloat
  - Single function declaration
  - Easy to type and looks fine
Cons:
  - Implicit conversions inside the function body may not work:
       std::string x = a;            // can't convert
       void x(std::string){}  x(a);  // can't convert
       std::string x{a};             // works...

*/


// Next two options share some code:
template<class T>
const char* c_str(const T& s)             { return s.c_str(); }
const char* c_str(const char* s)          { return s; }
const char* c_str(const unsigned char* s) { return reinterpret_cast<const char*>(s); }


// ============================================================================
// Option 1: Single template function =========================================
// ============================================================================

template<class A, class B>
void f(const A& a, const B& b) {
  return strstr(c_str(a), c_str(b));
}

/*
  Pros:
    - Single function declaration
    - No problems with implicit conversions
  Cons:
    - Code bloat for each instantiation of the function
    - Looks a bit strange...
*/


// ============================================================================
// Option 2: Write a templated wrapper function ===============================
// ============================================================================

void f(const char* a, const char* b) {
  return strstr(a, b);
}

template<class A, class B>
void f(const A& a, const B& b) {
  return f(c_str(a), c_str(b));
}

/*
  Pros:
    - No bloat
    - No problems with implicit conversions
  Cons:
    - More to type, looks ugly
*/

// ============================================================================
// Conclusion
// ============================================================================

/*
  Option 0 is the best solution unless the function is working with functions
  that don't operate directly on `const char*`.

  Option 1 is ideal for small functions that may get inlined anyway.

  Option 2 should be used for large functions.
*/
