#pragma once

#include "CQL/Serialize.hpp"
#include "CQL/Custom.hpp"

#include <unordered_set>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <memory>
#include <tuple>
#include <set>

namespace CQL {
  template<typename T, bool Instantiate>
  struct ConditionalVar { T val; };

  template<typename T>
  struct ConditionalVar<T, false> { };

  template<typename Entry>
  struct Table {
    Table() = default;

  private:
    struct AndOperation; struct OrOperation;
  public:
#define ExprOperators                                                  \
    template<typename Expr>                                            \
    auto operator&&(Expr &&other) && {                                 \
      return makeExpr<AndOperation>(std::move(*this),                  \
                                    std::forward<Expr>(other));        \
    }                                                                  \
                                                                       \
    template<typename Expr>                                            \
    auto operator&&(Expr &&other) & {                                  \
      return makeExpr<AndOperation>(*this, std::forward<Expr>(other)); \
    }                                                                  \
                                                                       \
    template<typename Expr>                                            \
    auto operator||(Expr &&other) && {                                 \
      return makeExpr<OrOperation>(std::move(*this),                   \
                                   std::forward<Expr>(other));         \
    }                                                                  \
                                                                       \
    template<typename Expr>                                            \
    auto operator||(Expr &&other) & {                                  \
      return makeExpr<OrOperation>(*this, std::forward<Expr>(other));  \
    }

#define ForEachOperator                                                \
    template<typename F>                                               \
    auto operator>>=(F functor) { return forEach(functor); }

    template<typename F>
    struct Predicate {
      Predicate(F &&f) : f{ f } { }
      bool operator()(Entry const &entry) { return f(entry); }

      ExprOperators

    private:
      F f;
    };

    template<typename RangeL, typename RangeR>
    struct RangeUnion {
      RangeUnion(RangeL &&rl, RangeR &&rr): rl{ std::forward<RangeL>(rl) }, rr{ std::forward<RangeR>(rr) } { }

      template<typename F>
      void forEach(F functor) {
        std::unordered_set<Entry const *> visited;
        rl.forEach([&](Entry const &entry) {
          functor(entry);
          visited.emplace(&entry);
        });

        rr.forEach([&](Entry const &entry) {
          if (visited.find(&entry) == visited.end()) {
            functor(entry);
          }
        });
      }

      bool operator()(Entry const &other) const {
        return rl(other) || rr(other);
      }

      ExprOperators
      ForEachOperator

    private:
      RangeL rl;
      RangeR rr;
    };

    template<typename Range, typename Pred>
    struct FilteredRangeExpr {
      FilteredRangeExpr(Range &&range, Pred &&predicate) : range{ range }, predicate{ predicate } { }

      bool operator()(Entry const &entry) {
        return range(entry) && predicate(entry);
      }

      template<typename F>
      void forEach(F functor) {
        range.forEach([&](Entry const &entry) {
          if (predicate(entry)) {
            functor(entry);
          }
        });
      }

      ExprOperators
      ForEachOperator

    private:
      Range range;
      Pred predicate;
    };

    template<size_t Ind, typename It>
    struct Iterator {
      Iterator &operator++() {
        ++setIt;
        return *this;
      }

      Iterator &operator--() {
        --setIt;
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

      template<size_t _Ind, typename _It>
      friend struct Range;

      explicit Iterator(It &&it) : setIt(std::forward<It>(it)) { }

      It setIt;
    };

    template<size_t Ind, typename It>
    struct Range {
      Range(It &&lo, It &&hi):
        lo{ std::forward<It>(lo) }, hi{ std::forward<It>(hi) } { }

      template<typename F>
      void forEach(F functor) const {
        for (auto it = lo; it != hi; ++it) {
          functor(*it);
        }
      }

      bool operator()(Entry const &other) const {
        if (lo == hi)
          return false;

        return std::get<Ind>(*lo) <= std::get<Ind>(other)
                                  && std::get<Ind>(other) <= std::get<Ind>(**std::prev(hi.setIt));
      }

      ExprOperators
      ForEachOperator

    private:
      Iterator<Ind, It> const lo, hi;
    };

#undef ForEachOperator
#undef ExprOperators

    template<typename ...Args>
    std::shared_ptr<Entry const> emplace(Args &&...args) {
      std::shared_ptr<Entry> ptr = std::make_shared<Entry>(std::forward<Args>(args)...);
      if (shouldInsert<0>(ptr)) {
        updateAll<0>(ptr);
        if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
          defaultLUT.val.emplace(ptr);
        }
        return ptr;
      }
      else {
        return nullptr;
      }
    }

    void erase(std::shared_ptr<Entry const> const &entry) {
      eraseAll<0>(entry);
      if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
        defaultLUT.val.erase(defaultLUT.val.find(entry.get()));
      }
    }

    auto begin() const {
      if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
        auto it = defaultLUT.val.begin();
        return Iterator<std::tuple_size<Entry>::value, decltype(it)>(std::move(it));
      }
      else {
        auto it = std::get<Custom::DefaultLookup<Entry>{}()>(luts).begin();
        return Iterator<Custom::DefaultLookup<Entry>{}(), decltype(it)>(std::move(it));
      }
    }

    auto end() const {
      if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
        auto it = defaultLUT.val.end();
        return Iterator<std::tuple_size<Entry>::value, decltype(it)>(std::move(it));
      }
      else {
        auto it = std::get<Custom::DefaultLookup<Entry>{}()>(luts).end();
        return Iterator<Custom::DefaultLookup<Entry>{}(), decltype(it)>(std::move(it));
      }
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
        return Iterator<Ind, T>(defaultLUT.val.erase(it.setIt));
      }
      else {
        if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
          defaultLUT.val.erase(*(it.setIt));
        }
        return eraseIt<std::tuple_size<Entry>::value, Ind>(it);
      }
    }

    size_t size() const {
      return defaultLookup().size();
    }

    bool empty() const {
      return defaultLookup().empty();
    }

    void clear() {
      clearAll<0>();
      if constexpr(Custom::DefaultLookup<Entry>{}() == std::tuple_size_v<Entry>) {
        defaultLUT.val.clear();
      }
    }

    std::shared_ptr<Entry> moveOut(std::shared_ptr<Entry const> const &entry) {
      erase(defaultLookup().find(entry));
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

    template<size_t N, typename T1, typename T2>
    auto range(T1 &&lb, T2 &&ub) {
      return makeRange<N>(std::get<N>(luts).lower_bound(lb),
                          std::get<N>(luts).upper_bound(ub));
    }

    auto all() {
      return makeRange<std::tuple_size<Entry>::value>(defaultLookup().begin(),
                                                      defaultLookup().end());
    }

    template<typename F>
    static auto pred(F &&predicate) {
      return Predicate<F>(std::forward<F>(predicate));
    }

  private:
    template<typename Operator, typename LE, typename RE>
    struct makeExprImpl {
      auto operator()(LE &&le, RE &&re) {
        if constexpr(std::is_same_v<Operator, AndOperation>) {
          return makeExpr<AndOperation>(std::forward<LE>(le), pred([r = std::forward<RE>(re)](auto val) { return r(val); }));
        }
        else {
          return RangeUnion<LE, RE>{ std::forward<LE>(le), std::forward<RE>(re) };
        }
      }
    };

    template<typename LE, typename T>
    struct makeExprImpl<AndOperation, LE, Predicate<T>> {
      auto operator()(LE &&expr, Predicate<T> &&pred) {
        return FilteredRangeExpr<LE, Predicate<T>>{ std::forward<LE>(expr), std::forward<Predicate<T>>(pred) };
      }
    };

    template<typename Operator, typename LE, typename RE>
    static auto makeExpr(LE &&le, RE &&re) {
      return makeExprImpl<Operator, LE, RE>{}(std::forward<LE>(le), std::forward<RE>(re));
    }

    template<size_t N, typename It>
    static auto makeRange(It &&itl, It &&itr) {
      return Range<N, It>{ std::forward<It>(itl), std::forward<It>(itr) };
    }

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

    ConditionalVar<std::set<std::shared_ptr<Entry>, Compare<std::tuple_size<Entry>::value>>,
                   std::tuple_size_v<Entry> == CQL::Custom::DefaultLookup<Entry>{}()> defaultLUT;

    auto constexpr &defaultLookup() const {
      if constexpr(Custom::DefaultLookup<Entry>{}() != std::tuple_size_v<Entry>) {
        static_assert(Custom::Unique<Entry, Custom::DefaultLookup<Entry>{}()>{}()
                                              != Custom::Uniqueness::NotUnique);
        return std::get<Custom::DefaultLookup<Entry>{}()>(luts);
      }
      else {
        return defaultLUT.val;
      }
    }

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

    template<size_t N>
    void clearAll() {
      if constexpr(N < std::tuple_size<Entry>::value) {
        std::get<N>(luts).clear();
        clearAll<N + 1>();
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
      if constexpr(Custom::Unique<Entry, N>{}() == Custom::Uniqueness::NotUnique) {
        auto[low, hi] = std::get<N>(luts).equal_range(val);
        if (auto f = std::find(low, hi, val); f != hi)
          return f;
      }
      else {
          return std::get<N>(luts).find(val);
      }
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

  template<typename T>
  struct Detail::Serialize<Table<T>> {
    void operator()(std::ostream &os, Table<T> const &table) const {
      uint64_t const s = table.size();
      serialize(os, s);
      for (auto &entry : table) {
        serialize(os, entry);
      }
    }
  };

  template<typename T>
  struct Detail::Deserialize<Table<T>> {
    Table<T> operator()(std::istream &is) const {
      Table<T> table;
      auto const s = static_cast<size_t>(deserialize<uint64_t>(is));

      for(size_t i = 0; i < s; ++ i) {
        table.emplace(deserialize<T>(is));
      }

      return table;
    }
  };
}

