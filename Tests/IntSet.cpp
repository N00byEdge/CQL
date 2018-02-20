#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Lookup, IntSet) {
  CQL::Table<std::tuple<int>> db;
  db.emplace(5);
  db.emplace(555);

  auto const a = db.lookup<0>(5);
  auto const b = db.lookup<0>(555);

  EXPECT_NE(a.get(), nullptr);
  EXPECT_NE(b.get(), nullptr);
  EXPECT_EQ(std::get<0>(*a), 5);
  EXPECT_EQ(std::get<0>(*b), 555);
}

TEST(Lookup, IntSetFail) {
  CQL::Table<std::tuple<int>> db;
  db.emplace(5);
  db.emplace(555);

  auto const fail = db.lookup<0>(55);

  EXPECT_EQ(fail.get(), nullptr);
}