#include "SimpleUser.hpp"
#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Emplace, SimpleUser) {
  CQL::Table<SimpleUser> db;
  auto a = db.emplace("Alice", 5);

  EXPECT_NE(a, nullptr);
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

TEST(UpdateSame, SimpleUser) {
  CQL::Table<SimpleUser> db;

  auto const a = db.emplace("Alice", 55);

  auto const aid = std::get<0>(*a);
  auto const updateResult = db.update<0>(a, aid);
  EXPECT_EQ(updateResult, true);

  EXPECT_EQ(db.size(), 1);
  auto const find = db.lookup<0>(aid);
  EXPECT_NE(find, nullptr);
  auto const found = std::get<0>(*find);
  EXPECT_EQ(found, aid);
}

TEST(UpdateFail, SimpleUser) {
  CQL::Table<SimpleUser> db;

  auto const a = db.emplace("Alice", 55);
  auto const b = db.emplace("Bob", 5);
                 db.emplace("Chris", 8);

  auto const aid = std::get<0>(*a);
  auto const updateResult = db.update<0>(a, std::get<0>(*b));
  EXPECT_EQ(updateResult, false);

  EXPECT_EQ(db.size(), 3);
  auto const find = db.lookup<0>(aid);
  EXPECT_NE(find, nullptr);
  auto const found = std::get<0>(*find);
  EXPECT_EQ(found, aid);
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

TEST(Serialization, SimpleUser) {
  std::stringstream ss;
  std::vector<SimpleUser> v{{ "Alice", 55 }, { "Bob", 7 }, { "Chris", 22 }};

  CQL::serialize(ss, v);
  
  auto const resultv = CQL::deserialize<std::vector<SimpleUser>>(ss);

  EXPECT_EQ(resultv.size(), v.size());
  for (std::size_t i = 0; i < resultv.size() && i < v.size(); ++i) {
    EXPECT_TRUE(resultv[i].operator==(v[i]));
  }
}

TEST(TableSerialization, SimpleUser) {
  std::stringstream ss;
  CQL::Table<SimpleUser> db;

  std::vector<SimpleUser> v{{"Alice", 55}, {"Bob", 7}, {"Chris", 22}};

  std::sort(std::begin(v), std::end(v));

  for (auto &u : v)
    db.emplace(u);

  CQL::serialize(ss, db);

  std::vector<SimpleUser> resultv;
  auto const resultt = CQL::deserialize<CQL::Table<SimpleUser>>(ss);

  for(auto &usr : resultt) {
    resultv.emplace_back(usr);
  }

  EXPECT_EQ(resultv.size(), v.size());
  for (std::size_t i = 0; i < resultv.size() && i < v.size(); ++ i) {
    EXPECT_TRUE(resultv[i].operator==(v[i]));
  }
}

TEST(TableSerializationToVector, SimpleUser) {
  std::stringstream ss;
  CQL::Table<SimpleUser> db;

  std::vector<SimpleUser> v{ { "Alice", 55 },{ "Bob", 7 },{ "Chris", 22 } };

  std::sort(std::begin(v), std::end(v));

  for (auto &u : v)
    db.emplace(u);

  CQL::serialize(ss, db);

  auto result = CQL::deserialize<std::vector<SimpleUser>>(ss);
  std::sort(std::begin(result), std::end(result));

  EXPECT_EQ(result.size(), v.size());
  for (std::size_t i = 0; i < result.size() && i < v.size(); ++i) {
    EXPECT_TRUE(result[i].operator==(v[i]));
  }
}
