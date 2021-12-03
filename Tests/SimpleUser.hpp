#pragma once

#include "CQL/Serialize.hpp"
#include "CQL/Custom.hpp"

#include <string>
#include <tuple>

struct SimpleUser {
  SimpleUser(std::string &&name, const int age) : id{ []() {
    static int lastID = -1;
    return ++lastID;
  }() }, name{ name }, age{ age } { }

  SimpleUser(const int id, std::string &&name, const int age) : id(id),
    name{ name }, age{ age } { }

  int id;
  std::string name;
  int age;

  bool operator<(SimpleUser const &other) const {
    if (auto cmp = id - other.id; cmp) return cmp < 0;
    if (auto cmp = name.compare(other.name); cmp) return cmp < 0;
    return age - other.age < 0;
  }

  bool operator==(SimpleUser const &other) const {
    return id == other.id && name == other.name && age == other.age;
  }

  void serialize(std::ostream &os) const {
    CQL::serialize(os, id);
    CQL::serialize(os, name);
    CQL::serialize(os, age);
  }

  static SimpleUser deserialize(std::istream &is) {
    auto id  = CQL::deserialize<int>        (is);
    auto s   = CQL::deserialize<std::string>(is);
    auto age = CQL::deserialize<int>        (is);
    return SimpleUser(id, std::move(s), age);
  }
};

namespace std {
  template<std::size_t Ind>
  constexpr auto &get(SimpleUser &su) noexcept {
    if constexpr(Ind == 0)
      return su.id;
    else if constexpr(Ind == 1)
      return su.name;
    else if constexpr(Ind == 2)
      return su.age;
  }

  template<std::size_t Ind>
  constexpr auto &get(SimpleUser const &su) noexcept {
    if constexpr(Ind == 0)
      return su.id;
    else if constexpr(Ind == 1)
      return su.name;
    else if constexpr(Ind == 2)
      return su.age;
  }
}

template <> struct std::tuple_size<SimpleUser> : public std::integral_constant<std::size_t, 3> { };
template<std::size_t Ind> struct std::tuple_element<Ind, SimpleUser> {
  using type = decltype(std::get<Ind>(std::declval<SimpleUser>()));
};

namespace CQL::Custom {
  template<>
  struct Unique<SimpleUser, 0> {
    constexpr Uniqueness operator()() const {
      return Uniqueness::EnforceUnique;
    }
  };

  template<>
  struct DefaultLookup<SimpleUser> {
    constexpr std::size_t operator()() const { return 0; }
  };
}
