#pragma once

#include <utility>

struct Point : std::pair<int, int> {
  Point(int f, int s) : std::pair<int, int>{ f, s } { }
};

namespace std {
  template<std::size_t Ind>
  constexpr auto &get(Point &p) noexcept {
    if constexpr(Ind == 0)
      return p.first;
    else if constexpr(Ind == 1)
      return p.second;
  }

  template<std::size_t Ind>
  constexpr auto &get(Point const &p) noexcept {
    if constexpr(Ind == 0)
      return p.first;
    else if constexpr(Ind == 1)
      return p.second;
  }
}

template <> struct std::tuple_size<Point> : public std::integral_constant<std::size_t, 2> { };
template<std::size_t Ind> struct std::tuple_element<Ind, Point> {
  using type = decltype(std::get<Ind>(std::declval<Point>()));
};
