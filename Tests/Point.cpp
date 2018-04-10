#include "Point.h"

#include "CQL.hpp"

#include "gtest/gtest.h"

TEST(Range, Point) {
  CQL::Table<Point> db;

  db.emplace(0, 0);
  db.emplace(0, 1);
  db.emplace(1, 0);
  db.emplace(1, 1);

  std::vector<Point> points;

  db.range<0>(0, 0) >>= [&](Point const &entry) {
    points.emplace_back(entry);
  };

  auto const ans = std::vector<Point>{ Point(0, 0), Point(0, 1) };
  
  EXPECT_EQ(points, ans);
}

TEST(RangeUnion, Point) {
  CQL::Table<Point> db;

  for(int x = -1; x <= 1; ++ x) {
    for(int y = -1; y <= 1; ++ y) {
      db.emplace(x, y);
    }
  }

  std::vector<Point> points;

  db.range<0>(0, 0) || db.range<1>(0, 0) >>= [&](Point const &entry) {
    points.emplace_back(entry);
  };

  std::sort(points.begin(), points.end());

  auto const ans = std::vector<Point>{ Point(-1, 0), Point(0, -1), Point(0, 0), Point(0, 1), Point(1, 0) };

  EXPECT_EQ(points, ans);
}

TEST(RangeIntersection, Point) {
  CQL::Table<Point> db;

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      db.emplace(x, y);
    }
  }

  std::vector<Point> points;

  db.range<0>(0, 0) && db.range<1>(0, 0) >>= [&](Point const &entry) {
    points.emplace_back(entry);
  };

  std::sort(points.begin(), points.end());

  auto const ans = std::vector<Point>{ Point(0, 0) };

  EXPECT_EQ(points, ans);
}

TEST(ComplexQuery, Point) {
  CQL::Table<Point> db;

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      db.emplace(x, y);
    }
  }

  std::vector<Point> points;

  (db.range<0>(0, 0) || db.range<1>(0, 0)) && db.pred([](Point const &entry) {
    return entry.first != entry.second;
  }) >>= [&](Point const &entry) {
    points.emplace_back(entry);
  };

  std::sort(points.begin(), points.end());

  auto const ans = std::vector<Point>{ Point(-1, 0), Point(0, -1), Point(0, 1), Point(1, 0) };

  EXPECT_EQ(points, ans);
}
