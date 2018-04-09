#pragma once

#include "CQL/Custom.hpp"

#include <type_traits>
#include <algorithm>
#include <utility>
#include <memory>
#include <tuple>
#include <set>

namespace CQL {
  template<typename Entry>
  struct Table {
    Table() = default;

    template<size_t Ind, typename It>
    struct Iterator {
      Iterator &operator++() {
        ++setIt;
        return *this;
      }

      Entry const &operator*() const {
        return **setIt;
      }

      bool operator==(Iterator<Ind, It> const &other) const {
        return setIt == other.setIt;
      }

      bool operator!=(Iterator<Ind, It> const &other) const {
        return setIt != other.setIt;
      }

      std::shared_ptr<Entry const> ptr() const {
        return *setIt;
      }
    protected:
      friend struct Table;

      explicit Iterator(It &&it) : setIt(std::forward<It>(it)) { }

      It setIt;
    };

    template<typename ...Args>
    std::shared_ptr<Entry const> emplace(Args &&...args) {
      std::shared_ptr<Entry> ptr = std::make_shared<Entry>(std::forward<Args>(args)...);
      if (shouldInsert<0>(ptr)) {
        updateAll<0>(ptr);
        ptrLut.emplace(ptr);
        return ptr;
      }
      else {
        return nullptr;
      }
    }

    void erase(std::shared_ptr<Entry const> const &entry) {
      eraseAll<0>(entry);
      ptrLut.erase(ptrLut.find(entry.get()));
    }

    auto begin() const {
      auto it = ptrLut.begin();
      return Iterator<std::tuple_size<Entry>::value, decltype(it)>(std::move(it));
    }

    auto end() const {
      auto it = ptrLut.end();
      return Iterator<std::tuple_size<Entry>::value, decltype(it)>(std::move(it));
    }

    template<size_t Ind>
    auto vbegin() const {
      auto it = std::get<Ind>(luts).begin();
      return Iterator<Ind, decltype(it)>(std::move(it));
    }

    template<size_t Ind>
    auto rvbegin() const {
      auto it = std::get<Ind>(luts).rbegin();
      return Iterator<Ind, decltype(it)>(std::move(it));
    }

    template<size_t Ind>
    auto vend() const {
      auto it = std::get<Ind>(luts).end();
      return Iterator<Ind, decltype(it)>(std::move(it));
    }

    template<size_t Ind>
    auto rvend() const {
      auto it = std::get<Ind>(luts).rend();
      return Iterator<Ind, decltype(it)>(std::move(it));
    }

    template<size_t Ind, typename T>
    auto erase(Iterator<Ind, T> &it) {
      if constexpr(Ind == std::tuple_size<Entry>::value) {
        eraseAll<0>(*(it.setIt));
        return Iterator<Ind, T>(ptrLut.erase(it.setIt));
      }
      else {
        ptrLut.erase(*(it.setIt));
        return eraseIt<std::tuple_size<Entry>::value, Ind>(it);
      }
    }

    size_t size() const {
      return ptrLut.size();
    }

    bool empty() const {
      return ptrLut.empty();
    }

    std::shared_ptr<Entry> moveOut(std::shared_ptr<Entry const> const &entry) {
      erase(ptrLut.find(entry));
      return entry;
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
      if constexpr(Custom::Unique<Entry, N>{}() == Custom::Uniqueness::EnforceUnique) {
        if (std::get<N>(luts).find(newVal) != std::get<N>(luts).end())
          return;
      }
      std::get<N>(*dbEntry) = newVal;
      std::get<N>(luts).emplace(std::move(dbEntry));
    }

    template<size_t N, size_t OtherN, typename T, typename ItT>
    void update(Iterator<OtherN, ItT> const &entry, T &&newVal) {
      if constexpr(N == OtherN) {
        throw std::runtime_error {
          "You cannot update the value of an object when you are iterating over the value you want to change"
        };
      }
      else {
        update(*(entry.setIt));
      }
    }

    template<size_t N, typename T>
    void swap(std::shared_ptr<Entry const> const &entry, T &&newVal) {
      std::shared_ptr<Entry> dbEntry = moveOutOfTable<N>(entry);
      std::swap(std::get<N>(*dbEntry), newVal);
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
        if constexpr(N == std::tuple_size<Entry>::value) {
          if constexpr(isEntry<T1>::value && isEntry<T2>::value) {
            return lhs < rhs;
          }
          else if constexpr(isEntry<T1>::value && !isEntry<T2>::value) {
            return lhs.get() < rhs;
          }
          else if constexpr(!isEntry<T1>::value && isEntry<T2>::value) {
            return lhs < rhs.get();
          }
        }
        else {
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
      }

      using is_transparent = void;
    };

    template<size_t Idx>
    static decltype(auto) makeSet() {
      if constexpr(Custom::Unique<Entry, Idx>{}() == Custom::Uniqueness::NotUnique) {
        return std::multiset<std::shared_ptr<Entry>, Compare<Idx>>{};
      }
      else {
        return std::set<std::shared_ptr<Entry>, Compare<Idx>>{};
      }
    }

    template<size_t ...Is>
    static decltype(auto) makeSets(std::index_sequence<Is...>) {
      return std::make_tuple(makeSet<Is>()...);
    }

    using Sets = decltype(makeSets(std::make_index_sequence<std::tuple_size<Entry>::value>{}));

    Sets luts{};
    std::set<std::shared_ptr<Entry>, Compare<std::tuple_size<Entry>::value>> ptrLut;

    template<size_t N, typename T>
    void updateAll(T const &entry) {
      if constexpr(N < std::tuple_size<Entry>::value) {
        std::get<N>(luts).emplace(entry);
        updateAll<N + 1>(entry);
      }
    }

    template<size_t N, typename T>
    void eraseAll(T const &entry) {
      if constexpr(N < std::tuple_size<Entry>::value) {
        std::get<N>(luts).erase(findInLut<N>(entry));
        eraseAll<N + 1>(entry);
      }
    }

    template<size_t N, size_t Er, typename T>
    auto eraseIt(T const &it) {
      if constexpr(N < std::tuple_size<Entry>::value) {
        if constexpr(N == Er) {
          eraseIt<N + 1, Er>(it);
          return std::get<N>(luts).erase(it);
        }
        else {
          std::get<N>(luts).erase(findInLut<N>(*(it.setIt)));
          return eraseIt<N + 1, Er>(it);
        }
      }
    }

    template<size_t N, typename T>
    auto findInLut(T const &val) const {
      auto low = std::get<N>(luts).lower_bound(val);
      auto hi = std::get<N>(luts).upper_bound(val);
      if (auto f = std::find(low, hi, val); f != hi)
        return f;
      return std::get<N>(luts).end();
    }

    template<size_t N, typename T>
    std::shared_ptr<Entry> moveOutOfTable(T const &val) {
      auto it = findInLut<N>(val);
      auto dbEntry = std::move(*it);
      std::get<N>(luts).erase(it);
      return dbEntry;
    }
    
    template<size_t N, typename T>
    auto shouldInsert(T const &val) const {
      if constexpr(N < std::tuple_size<Entry>::value) {
        if constexpr(Custom::Unique<Entry, N>{}() == Custom::Uniqueness::EnforceUnique) {
          return (std::get<N>(luts).find(val) == std::get<N>(luts).end()) && shouldInsert<N + 1>(val);
        }
        else {
          return shouldInsert<N + 1>(val);
        }
      }
      else {
        return true;
      }
    }
  };
}

