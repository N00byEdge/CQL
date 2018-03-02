#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Lookup, StringIntTuple) {
  CQL::Table<std::tuple<std::string, int>> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const a = db.lookup<0>("Alice");
  auto const b = db.lookup<0>("Bob");

  EXPECT_NE(a, nullptr);
  EXPECT_NE(b, nullptr);
  EXPECT_EQ(std::get<1>(*a), 5);
  EXPECT_EQ(std::get<1>(*b), 55);
}

TEST(Lookup, StringIntTupleFail) {
  CQL::Table<std::tuple<std::string, int>> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const fail = db.lookup<0>("Fail");

  EXPECT_EQ(fail, nullptr);
}
