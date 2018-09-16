#include "lib.hpp"

#include <iostream>

struct Foo {};
std::ostream& operator<<(std::ostream& out, const Foo&) { return out << "Foo"; }

struct Bar {};
std::ostream& operator<<(std::ostream& out, const Bar&) { return out << "Bar"; }

int main() {
  Object o1 = Object::make_object<Foo>();
  Object o2 = Object::make_object<Bar>();

  std::cout << is_a<Foo>(o1) << std::endl;
  std::cout << is_a<Foo>(o2) << std::endl;

  std::cout << is_a<Bar>(o1) << std::endl;
  std::cout << is_a<Bar>(o2) << std::endl;

  std::cout << *as_a<Foo>(o1) << std::endl;
}