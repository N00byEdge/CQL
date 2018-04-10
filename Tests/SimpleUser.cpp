#include "SimpleUser.hpp"
#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Emplace, SimpleUser) {
  CQL::Table<SimpleUser> db;
  auto a = db.emplace("Alice", 5);

  EXPECT_NE(a, nullptr);
  EXPECT_EQ(a.use_count(), std::tuple_size<SimpleUser>::value + 2);
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

TEST(LookupFail, SimpleUser) {
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

TEST(OrderedIteration, SimpleUser) {
  CQL::Table<SimpleUser> db;

  db.emplace("Bob", 55);
  db.emplace("Alice", 5);
  db.emplace("Chris", 8);

  std::vector<int>         uids;
  std::vector<std::string> names;
  std::vector<int>         ages;

  for (auto it = db.vbegin<0>(); it != db.vend<0>(); ++it) {
    uids.emplace_back(std::get<0>(*it));
  }

  for (auto it = db.vbegin<1>(); it != db.vend<1>(); ++it) {
    names.emplace_back(std::get<1>(*it));
  }

  for (auto it = db.vbegin<2>(); it != db.vend<2>(); ++it) {
    ages.emplace_back(std::get<2>(*it));
  }

  std::vector<std::string> const nameAns{ "Alice", "Bob", "Chris" };
  std::vector<int>  const        ageAns { 5, 8, 55 };

  EXPECT_EQ(uids[0] + 1, uids[1]); EXPECT_EQ(uids[1] + 1, uids[2]);
  EXPECT_EQ(names, nameAns);
  EXPECT_EQ(ages, ageAns);
}

TEST(Uniqueness, SimpleUser) {
  // SimpleUser is set to enforce uniqueness on ID,
  // but will not care about the other variables.
  CQL::Table<SimpleUser> db;

  db.emplace(0, "Alice", 55);
  db.emplace(0, "Bob", 7);

  EXPECT_EQ(db.size(), 1);

  db.emplace(1, "Alice", 55);

  EXPECT_EQ(db.size(), 2);
}
