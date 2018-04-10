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

TEST(LookupFail, StringIntTuple) {
  CQL::Table<std::tuple<std::string, int>> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const fail = db.lookup<0>("Fail");

  EXPECT_EQ(fail, nullptr);
}

TEST(Range, StringIntTuple) {
  CQL::Table<std::tuple<std::string, int>> db;

  db.emplace("Alice", 5);
  db.emplace("Bob", 55);
  db.emplace("Bert", 78);
  db.emplace("Calle", 33);

  std::vector<std::string> names;

  db.range<0>("B", "C") >>= [&](auto const &entry) {
    names.emplace_back(std::get<0>(entry));
  };

  std::sort(names.begin(), names.end());

  std::vector<std::string> const ans{ "Bert", "Bob" };

  EXPECT_EQ(names, ans);
}

TEST(RangeEmpty, StringIntTuple) {
  CQL::Table<std::tuple<std::string, int>> db;

  db.emplace("Alice", 5);
  db.emplace("Bob", 55);
  db.emplace("Bert", 78);
  db.emplace("Calle", 33);

  std::vector<std::string> names;

  db.range<0>("Q", "Z") >>= [&](auto const &entry) {
    names.emplace_back(std::get<0>(entry));
  };

  std::sort(names.begin(), names.end());

  std::vector<std::string> const ans{};

  EXPECT_EQ(names, ans);
}
