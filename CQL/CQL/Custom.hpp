#pragma once

#include <cstddef>

namespace CQL::Custom {
  enum class Uniqueness {
    NotUnique,
    EnforceUnique,
    AssumeUnique,
  };

  template<typename T, size_t Idx>
  struct Unique {
    constexpr Uniqueness operator()() const {
      return Uniqueness::NotUnique;
    }
  };
}
