#pragma once

#include "CQL/Custom.hpp"

#include <string>
#include <tuple>

struct SimpleUser {
  SimpleUser(std::string &&name, const int age) : id{ []() {
    static int lastID = -1;
    return ++lastID;
  }() }, name{ std::move(name) }, age{ age } { }

  SimpleUser(const int id, std::string &&name, const int age) : id(id),
    name{ std::move(name) }, age{ age } { }

  int id;
  std::string name;
  int age;
};

namespace std {
  template<size_t Ind>
  constexpr auto &get(SimpleUser &su) noexcept {
    if constexpr(Ind == 0)
      return su.id;
    else if constexpr(Ind == 1)
      return su.name;
    else if constexpr(Ind == 2)
      return su.age;
  }

  template<size_t Ind>
  constexpr auto &get(SimpleUser const &su) noexcept {
    if constexpr(Ind == 0)
      return su.id;
    else if constexpr(Ind == 1)
      return su.name;
    else if constexpr(Ind == 2)
      return su.age;
  }
}

template <> struct std::tuple_size<SimpleUser> : public std::integral_constant<size_t, 3> { };
template<size_t Ind> struct std::tuple_element<Ind, SimpleUser> {
  using type = decltype(std::get<SimpleUser, Ind>);
};

namespace CQL::Custom {
  template<>
  struct Unique<SimpleUser, 0> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::EnforceUnique;
    }
  };
}
