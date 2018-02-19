#pragma once

#include <tuple>
#include <memory>
#include <set>
#include <mutex>
#include <iostream>
#include <algorithm>

namespace CQL {
  template<typename Entry>
  struct Table {
    Table() = default;

    template<typename ...Args>
    std::shared_ptr<Entry const> addEntry(Args &&...args) {
      std::shared_ptr<Entry> ptr = std::make_shared<Entry>(std::forward<Args>(args)...);
      updateAll<0>(ptr);
      return ptr;
    }

    template<size_t N, typename T>
    std::shared_ptr<Entry const> lookup(T const &val) {
      std::shared_ptr<Entry> ret = nullptr;
      auto &l = lut<N>();
      {
        std::lock_guard<std::mutex> lock(l.mut);
        if (auto it = l.lut.find(val); it != l.lut.end())
          ret = *it;
      }

      return ret;
    }

  private:
    template<size_t N>
    struct Lookup {
      struct Compare {
        bool operator()(std::shared_ptr<Entry> const &lhs, std::shared_ptr<Entry> const &rhs) const {
          return std::get<N>(*lhs) < std::get<N>(*rhs);
        }

        template <typename T>
        bool operator()(T const &lhs, std::shared_ptr<Entry> const &rhs) const {
          return lhs < std::get<N>(*rhs);
        }

        template <typename T>
        bool operator()(std::shared_ptr<Entry> const &lhs, T const &rhs) const {
          return std::get<N>(*lhs) < rhs;
        }

        using is_transparent = int;
      };

      std::set<std::shared_ptr<Entry>, Compare> lut;
      std::mutex mut;
    };

    template<size_t N>
    Lookup<N> &lut() {
      static Lookup<N> lut;
      return lut;
    }

    template<size_t N>
    void updateAll(std::shared_ptr<Entry> &entry) {
      {
        auto &l = lut<N>();
        std::lock_guard<std::mutex>(l.mut);
        l.lut.emplace(entry);
      }
      updateAll<N + 1>(entry);
    }

    template <>
    void updateAll<std::tuple_size_v<Entry>>(std::shared_ptr<Entry> &entry) { }
  };

  template<typename T>
  void Serialize(std::ostream &os, T const &&val);

  template<typename T>
  void Serialize(std::ostream &os, int const &&val) {
    os.write(reinterpret_cast<const char *>(&val), sizeof(int));
  }

  template<>
  inline void Serialize(std::ostream &os, uint64_t const &&val) {
    os.write(reinterpret_cast<const char *>(&val), sizeof(uint64_t));
  }

  template<>
  inline void Serialize(std::ostream &os, std::string const &&val) {
    uint64_t len = val.size();
    Serialize(os, std::forward<uint64_t>(len));
    os.write(val.data(), len);
  }

  template<typename T>
  T Deserialize(std::istream &is);

  template<>
  inline int Deserialize(std::istream &is) {
    int temp;
    is.read(reinterpret_cast<char *>(&temp), sizeof(int));
    return temp;
  }

  template<>
  inline uint64_t Deserialize(std::istream &is) {
    uint64_t val;
    is.read(reinterpret_cast<char *>(&val), sizeof(uint64_t));
    return val;
  }

  template<>
  inline std::string Deserialize(std::istream &is) {
    auto const len = Deserialize<uint64_t>(is);
    std::string result;
    result.reserve(len);
    std::copy_n(std::istreambuf_iterator<char>(is), len, std::back_inserter(result));
    return result;
  }
}
