#pragma once

#include <cstddef>
#include <tuple>

namespace CQL::Custom {
  enum class Uniqueness {
    NotUnique,
    EnforceUnique,
    AssumeUnique,
  };

  template<typename T, std::size_t Idx>
  struct Unique {
    constexpr Uniqueness operator()() const {
      return Uniqueness::NotUnique;
    }
  };

  template<typename T>
  struct DefaultLookup {
    constexpr std::size_t operator()() const { return std::tuple_size_v<T>; }
  };
}
