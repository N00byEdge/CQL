#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Emplace, IntSet) {
  CQL::Table<std::tuple<int>> db;
  auto const a = db.emplace(5);

  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a.use_count(), 2);
}

TEST(Lookup, IntSet) {
  CQL::Table<std::tuple<int>> db;
  auto const a = db.emplace(5);
  auto const b = db.emplace(555);

  EXPECT_NE(a, nullptr);
  EXPECT_NE(b, nullptr);
  EXPECT_EQ(std::get<0>(*a), 5);
  EXPECT_EQ(std::get<0>(*b), 555);
}

TEST(Lookup, IntSetFail) {
  CQL::Table<std::tuple<int>> db{};
  db.emplace(5);
  db.emplace(555);

  auto const fail = db.lookup<0>(55);

  EXPECT_EQ(fail, nullptr);
}

TEST(Update, IntSet) {
  CQL::Table<std::tuple<int>> db;
                 db.emplace(4);
  auto const a = db.emplace(5);
                 db.emplace(6);

  db.update<0>(a, 7);

  EXPECT_NE(a, nullptr);
  EXPECT_EQ(std::get<0>(*a), 7);

  auto const fail = db.lookup<0>(5);

  EXPECT_EQ(fail, nullptr);
}
