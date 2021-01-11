#ifndef LIB_DEBUG_HPP
#define LIB_DEBUG_HPP

#include <cstdio>
#include <iostream>

namespace Debug {

static const char indent[] =
  "                                                       "
  "                                                       "
  "                                                       ";

inline static void innerPrint(std::ostream &stream) {}

template<typename Head, typename... Tail>
inline static void innerPrint(std::ostream &stream, const Head& head, const Tail& ... tail) {
  stream << head;
  innerPrint(stream, tail ...);
}

struct Logger {
  int call_level;

  Logger() : call_level(0) {}

  struct Function {
    Logger& logger;

    template<typename ... T>
    Function(Logger& logger_, const char* name, const T& ... args)
    : logger(logger_)
    {
      std::cout << &indent[sizeof(indent) - logger.call_level * 2 - 1] << name << '(';
      innerPrint(std::cout, args ...);
      std::cout << ")\n";
      logger.call_level++;
    }

    ~Function() {
      logger.call_level--;
    }
  };

  template<typename ... T>
  Function enter_function(const T& ... args) { return Function(*this, args ...); }

  template<typename ... T>
  void debug(const T& ... args) {
    std::cout << &indent[sizeof(indent) - call_level * 2 - 1];
    innerPrint(args ...);
    std::cout << '\n';
  }
};

} // namespace Debug

#endif
