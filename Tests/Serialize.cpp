#include "CQL/Serialize.hpp"

#include "gtest/gtest.h"

#include <utility>
#include <vector>
#include <array>
#include <tuple>
#include <map>
#include <set>

static_assert( CQL::Detail::isContainer<std::vector<int>>);
static_assert( CQL::Detail::isContainer<std::set<int>>);
static_assert( CQL::Detail::isContainer<std::map<int, int>>);
static_assert( CQL::Detail::isContainer<std::array<int, 5>>);
static_assert(!CQL::Detail::isContainer<std::tuple<int, int, int>>);
static_assert(!CQL::Detail::isContainer<std::tuple<int, std::tuple<int, int>>>);
static_assert(!CQL::Detail::isContainer<std::pair<int, int>>);
static_assert(!CQL::Detail::isContainer<int>);

static_assert(!CQL::Detail::isTuploid<std::vector<int>>);
static_assert(!CQL::Detail::isTuploid<std::set<int>>);
static_assert(!CQL::Detail::isTuploid<std::map<int, int>>);
static_assert( CQL::Detail::isTuploid<std::array<int, 5>>);
static_assert( CQL::Detail::isTuploid<std::tuple<int, int, int>>);
static_assert( CQL::Detail::isTuploid<std::tuple<int, std::tuple<int, int>>>);
static_assert( CQL::Detail::isTuploid<std::pair<int, int>>);
static_assert(!CQL::Detail::isTuploid<int>);

static_assert( CQL::Detail::isContinuous<std::vector<int>>);
static_assert(!CQL::Detail::isContinuous<std::set<int>>);
static_assert(!CQL::Detail::isContinuous<std::map<int, int>>);
static_assert( CQL::Detail::isContinuous<std::array<int, 5>>);
static_assert(!CQL::Detail::isContinuous<std::tuple<int, int, int>>);
static_assert(!CQL::Detail::isContinuous<std::tuple<int, std::tuple<int, int>>>);
static_assert(!CQL::Detail::isContinuous<std::pair<int, int>>);
static_assert(!CQL::Detail::isContinuous<int>);

TEST(Serialize, int) {
  std::stringstream ss;

  CQL::serialize(ss, 5);

  EXPECT_EQ(CQL::deserialize<int>(ss), 5);
}

TEST(Serialize, intintinttuple) {
  std::stringstream ss;

  CQL::serialize(ss, std::make_tuple(5, 5, 5));

  auto const result = CQL::deserialize<std::tuple<int, int, int>>(ss);
  EXPECT_EQ(result, std::make_tuple(5, 5, 5));
}

TEST(Serialize, intset) {
  std::stringstream ss;

  std::set<int> s;
  for(int i = 0; i < 10; ++ i) {
    s.insert(i);
  }

  CQL::serialize(ss, s);

  auto const result = CQL::deserialize<std::set<int>>(ss);
  EXPECT_EQ(s, result);
}

TEST(Serialize, intmap) {
  std::stringstream ss;

  std::map<int, int> m;
  m[0] = 7;
  m[3] = m[5] = 8;

  CQL::serialize(ss, m);

  auto const result = CQL::deserialize<std::map<int, int>>(ss);
  EXPECT_EQ(m, result);
}

TEST(Serialize, string) {
  std::stringstream ss;
  std::string str("A quite long string");

  CQL::serialize(ss, str);

  auto const result = CQL::deserialize<std::string>(ss);
  EXPECT_EQ(str, result);
}

TEST(Serialize, stringstringmap) {
  std::stringstream ss;
  std::map<std::string, std::string> m;

  m["Testing"] = "is fun!";
  m["None of the"] = "test cases";
  m["shall"] = "fail!";

  CQL::serialize(ss, m);

  auto const result = CQL::deserialize<decltype(m)>(ss);
  EXPECT_EQ(m, result);
}

TEST(Serialize, intvector) {
  std::stringstream ss;
  std::vector<int> v{ 5, 2, 3, 6, 8, 1, 2, 5, 6 };

  CQL::serialize(ss, v);

  auto const result = CQL::deserialize<std::vector<int>>(ss);
  EXPECT_EQ(v, result);
}

TEST(Serialize, intvectorvector) {
  std::stringstream ss;
  std::vector<std::vector<int>> v {{5, 7, 1, 35, 6}, {1, 6, 16, 8, 15}, {612, 51, 6326, 747}};

  CQL::serialize(ss, v);

  auto const result = CQL::deserialize<decltype(v)>(ss);
  EXPECT_EQ(v, result);
}
