#pragma once

#include <algorithm>
#include <utility>
#include <memory>
#include <set>

namespace CQL {
  template<typename Entry>
  struct Table {
    Table() = default;

    template<typename ...Args>
    std::shared_ptr<Entry const> emplace(Args &&...args) {
      std::shared_ptr<Entry> ptr = std::make_shared<Entry>(std::forward<Args>(args)...);
      updateAll<0>(ptr);
      return ptr;
    }

    template<size_t N, typename T>
    std::shared_ptr<Entry const> lookup(T const &val) {
      std::shared_ptr<Entry> ret;
      if (auto it = std::get<N>(luts).find(val); it != std::get<N>(luts).end())
        ret = *it;

      return ret;
    }

    template<size_t N, typename T>
    void update(std::shared_ptr<Entry const> const &entry, T &&newVal) {
      std::shared_ptr<Entry> dbEntry = moveOutOfTable<N>(entry);
      std::get<N>(*dbEntry) = newVal;
      std::get<N>(luts).emplace(std::move(dbEntry));
    }

  private:
    template<size_t N>
    struct Compare {
      template<typename T>
      struct isEntry {
        constexpr static bool value = std::is_same<T, std::shared_ptr<Entry>>::value
                                   || std::is_same<T, std::shared_ptr<Entry const>>::value;
      };

      template<typename T1, typename T2>
      bool operator()(T1 const &lhs, T2 const &rhs) const {
        if constexpr(isEntry<T1>::value && isEntry<T2>::value) {
          return std::get<N>(*lhs) < std::get<N>(*rhs);
        }
        else if constexpr(isEntry<T1>::value) {
          return std::get<N>(*lhs) < rhs;
        }
        else if constexpr(isEntry<T2>::value) {
          return lhs < std::get<N>(*rhs);
        }
      }

      using is_transparent = void;
    };

    template<size_t ...Is>
    static decltype(auto) makeSets(std::index_sequence<Is...>) {
      return std::make_tuple(std::set<std::shared_ptr<Entry>, Compare<Is>>{}...);
    }

    using Sets = decltype(makeSets(std::make_index_sequence<std::tuple_size<Entry>::value>{}));

    Sets luts{};

    template<size_t N>
    void updateAll(std::shared_ptr<Entry> &entry) {
      if constexpr(N < std::tuple_size<Entry>::value) {
        std::get<N>(luts).emplace(entry);
        updateAll<N + 1>(entry);
      }
    }

    template<size_t N>
    auto findInLut(std::shared_ptr<Entry const> val) {
      auto low = std::get<N>(luts).lower_bound(val);
      auto hi = std::get<N>(luts).upper_bound(val);
      if (auto f = std::find(low, hi, val); f != hi)
        return f;
      return std::get<N>(luts).end();
    }

    template<size_t N>
    std::shared_ptr<Entry> moveOutOfTable(std::shared_ptr<Entry const> val) {
      auto it = findInLut<N>(val);
      auto dbEntry = std::move(*it);
      std::get<N>(luts).erase(it);
      return dbEntry;
    }
  };
}

