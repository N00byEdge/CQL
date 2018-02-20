#include "CQL.hpp"

#include "gtest/gtest.h"

struct SimpleUser {
  SimpleUser(std::string &&name, const int age) : id{ []() {
    static int lastID = -1;
    return ++lastID;
  }() }, name{ std::move(name) }, age{ age } { }

  int id;
  std::string name;
  int age;
};

namespace std {
  template<size_t Ind>
  constexpr auto &get(SimpleUser const &su) {
    if constexpr(Ind == 0) return su.id;
    else if constexpr(Ind == 1) return su.name;
    else if constexpr(Ind == 2) return su.age;
  }
}

template <> struct std::tuple_size<SimpleUser> : public std::integral_constant<size_t, 3> { };
template<size_t Ind> struct std::tuple_element<Ind, SimpleUser> {
  using type = decltype(std::get<Ind>(SimpleUser()));
};

TEST(Lookup, SimpleUser) {
  CQL::Table<SimpleUser> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const a = db.lookup<1>("Alice");
  auto const b = db.lookup<1>("Bob");

  EXPECT_NE(a.get(), nullptr);
  EXPECT_NE(b.get(), nullptr);
  EXPECT_EQ(a->age, 5);
  EXPECT_EQ(b->age, 55);
}

TEST(Lookup, SimpleUserFail) {
  CQL::Table<SimpleUser> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const fail = db.lookup<1>("Fail");

  EXPECT_EQ(fail.get(), nullptr);
}
