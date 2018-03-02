#include "SimpleUser.hpp"
#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Emplace, SimpleUser) {
  CQL::Table<SimpleUser> db;
  auto a = db.emplace("Alice", 5);

  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a.use_count(), std::tuple_size<SimpleUser>::value + 1);
}


TEST(Lookup, SimpleUser) {
  CQL::Table<SimpleUser> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const a = db.lookup<1>("Alice");
  auto const b = db.lookup<1>("Bob");

  EXPECT_NE(a, nullptr);
  EXPECT_NE(b, nullptr);
  EXPECT_EQ(a->age, 5);
  EXPECT_EQ(b->age, 55);
}

TEST(Lookup, SimpleUserFail) {
  CQL::Table<SimpleUser> db;
  db.emplace("Alice", 5);
  db.emplace("Bob", 55);

  auto const fail = db.lookup<1>("Fail");

  EXPECT_EQ(fail, nullptr);
}

TEST(Update, SimpleUser) {
  CQL::Table<SimpleUser> db;
                 db.emplace("Alice", 55);
  auto const b = db.emplace("Bob", 5);
                 db.emplace("Chris", 8);

  db.update<1>(b, "Daniel");

  EXPECT_EQ(std::get<1>(*b), "Daniel");

  auto const fail = db.lookup<1>("Bob");

  EXPECT_EQ(fail, nullptr);
}

TEST(UpdateMove, SimpleUser) {
  CQL::Table<SimpleUser> db;
  db.emplace("Alice", 55);
  auto const b = db.emplace("Bob", 5);
  db.emplace("Chris", 8);

  std::string newname("Daniel");
  db.update<1>(b, std::move(newname));

  EXPECT_EQ(std::get<1>(*b), "Daniel");

  auto const fail = db.lookup<1>("Bob");

  EXPECT_EQ(fail, nullptr);
}
