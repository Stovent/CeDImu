#include "Function.hpp"

template <class F>
Function(F) -> Function<typename function_traits<decltype(&F::operator())>::type>;

consteval auto test_empty() {
  Function f = [] { return 42; };
  return f();
}

consteval auto test_arg() {
  Function f = [](int i) { return i; };
  return f(42);
}

consteval auto test_capture() {
  int i = 42;
  Function f = [&] { return i; };
  return f();
}

static_assert(42 == test_empty());
static_assert(42 == test_arg());
static_assert(42 == test_capture());
