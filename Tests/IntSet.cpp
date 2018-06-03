#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Emplace, IntSet) {
  CQL::Table<std::tuple<int>> db;
  auto const a = db.emplace(5);

  EXPECT_NE(a, nullptr);
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

TEST(LookupFail, IntSet) {
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

TEST(Empty, IntSet) {
  CQL::Table<std::tuple<int>> db;

  EXPECT_EQ(db.empty(), true);

  db.emplace(5);

  EXPECT_EQ(db.empty(), false);
}

TEST(Size, IntSet) {
  CQL::Table<std::tuple<int>> db;

  EXPECT_EQ(db.size(), 0);

  db.emplace(5);

  EXPECT_EQ(db.size(), 1);

  for(size_t i = 0; i < 4; ++ i)
    db.emplace(5);

  EXPECT_EQ(db.size(), 5);

  db.emplace(6);

  EXPECT_EQ(db.size(), 6);
}

TEST(Erase, IntSet) {
  CQL::Table<std::tuple<int>> db;
                 db.emplace(4);
  auto const a = db.emplace(5);
                 db.emplace(6);

  EXPECT_EQ(db.size(), 3);

  db.erase(a);

  EXPECT_EQ(db.size(), 2);

  EXPECT_EQ(db.lookup<0>(5), nullptr);
}

TEST(Clear, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(5);

  EXPECT_EQ(db.size(), 1);

  db.clear();

  EXPECT_EQ(db.size(), 0);
}

TEST(Iterate, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(4);
  db.emplace(5);
  db.emplace(6);

  std::vector<int> vals;

  for(auto &i : db) {
    vals.emplace_back(std::get<0>(i));
  }

  std::sort(vals.begin(), vals.end());

  auto const ans = std::vector<int>{ 4, 5, 6 };

  EXPECT_EQ(vals, ans);
}

TEST(BeginEndEq, IntSet) {
  CQL::Table<std::tuple<int>> db;

  EXPECT_EQ(db.begin(), db.end());
}

TEST(BeginEndNe, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(5);

  EXPECT_NE(db.begin(), db.end());
}

TEST(IteratorErase, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(4);
  db.emplace(5);
  db.emplace(6);

  for(auto it = db.begin(); it != db.end();) {
    if(std::get<0>(*it) == 5) {
      it = db.erase(it);
    }
    else {
      ++it;
    }
  }

  std::vector<int> vals;

  for (auto &i : db) {
    vals.emplace_back(std::get<0>(i));
  }

  std::sort(vals.begin(), vals.end());

  auto const ans = std::vector<int>{ 4, 6 };

  EXPECT_EQ(vals, ans);
}

TEST(OrderedIteration, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(4);
  db.emplace(5);
  db.emplace(6);

  std::vector<int> vals;

  for (auto it = db.vbegin<0>(); it != db.vend<0>(); ++ it) {
    vals.emplace_back(std::get<0>(*it));
  }

  auto const ans = std::vector<int>{ 4, 5, 6 };

  EXPECT_EQ(vals, ans);
}

TEST(OrderedIterationReversed, IntSet) {
  CQL::Table<std::tuple<int>> db;

  db.emplace(4);
  db.emplace(5);
  db.emplace(6);

  std::vector<int> vals;

  for (auto it = db.rvbegin<0>(); it != db.rvend<0>(); ++it) {
    vals.emplace_back(std::get<0>(*it));
  }

  auto const ans = std::vector<int>{ 6, 5, 4 };

  EXPECT_EQ(vals, ans);
}

TEST(Range, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for(int i = 0; i < 10; ++ i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(5, 7) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  auto const ans = std::vector<int>{ 5, 6, 7 };

  EXPECT_EQ(vals, ans);
}

TEST(RangeEmpty, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 10; ++i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(80, 90) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  auto const ans = std::vector<int>{};

  EXPECT_EQ(vals, ans);
}

TEST(All, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 4; ++i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.all() >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  std::sort(vals.begin(), vals.end());

  auto const ans = std::vector<int>{ 0, 1, 2, 3 };

  EXPECT_EQ(vals, ans);
}

TEST(Predicate, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 100; ++i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(10, 40) && db.pred([](auto const &val) {
    return std::get<0>(val) % 19 == 0;
  }) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };
  
  std::sort(vals.begin(), vals.end());

  auto const ans = std::vector<int>{ 19, 38 };

  EXPECT_EQ(vals, ans);
}

TEST(PredicateWithHalfRange, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 100; ++i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(10, 40) && db.range<0>(0, 600) && db.pred([](auto const &val) {
    return std::get<0>(val) % 10 == 0;
  }) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  auto const ans = std::vector<int>{ 10, 20, 30, 40 };

  EXPECT_EQ(vals, ans);
}

TEST(PredicateWithHalfLowerRange, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 100; ++i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(10, 40) && db.range<0>(-40, 20) && db.pred([](auto const &val) {
    return std::get<0>(val) % 10 == 0;
  }) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  auto const ans = std::vector<int>{ 10, 20 };

  EXPECT_EQ(vals, ans);
}

TEST(PredicateWithEmptyRange, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for(int i = 0; i < 100; ++ i) {
    db.emplace(i);
  }

  std::vector<int> vals;

  db.range<0>(10, 40) && db.range<0>(500, 600) && db.pred([](auto const &val) {
    return std::get<0>(val) % 10 == 0;
  }) >>= [&vals](auto const &val) {
    vals.emplace_back(std::get<0>(val));
  };

  auto const ans = std::vector<int>{};

  EXPECT_EQ(vals, ans);
}

TEST(Extract, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 100; ++i) {
    db.emplace(i);
  }

  EXPECT_EQ(db.size(), 100);

  auto val = db.lookup<0>(10);
  auto src = db.extract(val);

  EXPECT_EQ(db.size(), 99);

  EXPECT_EQ(std::get<0>(*src), 10);

  auto v2 = db.lookup<0>(10);

  EXPECT_EQ(v2, nullptr);
}

TEST(Swap, IntSet) {
  CQL::Table<std::tuple<int>> db;

  for (int i = 0; i < 100; ++i) {
    db.emplace(i);
  }

  EXPECT_EQ(db.size(), 100);

  auto val = db.lookup<0>(10);
  auto swapinval = 101;

  auto swapresult = db.swap<0>(val, swapinval);
  EXPECT_EQ(swapresult, true);

  auto oldval = db.lookup<0>(10);

  EXPECT_EQ(oldval, nullptr);

  auto newval = db.lookup<0>(101);

  EXPECT_NE(newval, nullptr);

  EXPECT_EQ(std::get<0>(*val), 101);
  EXPECT_EQ(std::get<0>(*newval), 101);
  EXPECT_EQ(swapinval, 10);
  
  EXPECT_EQ(db.size(), 100);
}
