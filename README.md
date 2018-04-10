# CQL

# Build status
## Master:
Windows: [![Build status](https://ci.appveyor.com/api/projects/status/83qokd6w77vsav6n/branch/master?svg=true)](https://ci.appveyor.com/project/N00byEdge/cql/branch/master)
Linux: [![Build Status](https://travis-ci.org/N00byEdge/CQL.svg?branch=master)](https://travis-ci.org/N00byEdge/CQL)

## Develop:
Windows: [![Build status](https://ci.appveyor.com/api/projects/status/83qokd6w77vsav6n/branch/develop?svg=true)](https://ci.appveyor.com/project/N00byEdge/cql/branch/develop)
Linux: [![Build Status](https://travis-ci.org/N00byEdge/CQL.svg?branch=develop)](https://travis-ci.org/N00byEdge/CQL)

# What is CQL?
CQL is a templated, header-only, database library for C++17 and above. It provides type safety, `const` correctness and automatic managment of your lookup tables, pretty much all using template "magic".

# Cool. What can I store in it?
Pretty much anything as long as you tell CQL how to store it. CQL needs a couple of things to understand your type.

# Okay, how do I use it?
The only thing you really need to do is to make your type decomposable:

```cpp
struct User {
  User(std::string &&name, const int age) : id{ []() {
    static int lastID = -1; return ++lastID;
  }() }, name{ std::forward(name) }, age{ age } { }

  User(const int id, std::string &&name, const int age) : id(id),
    name{ std::forward(name) }, age{ age } { }

  int id;
  std::string name;
  int age;
};
```

If we want to take the above type and make it decomposable, we have to specify how many "parts" it can be decomposable to, what types these parts have and how to get them. The C++ has ways of specifying this already, so we're reusing them:

```cpp
namespace std {
  // Getter
  template<size_t Ind>
  constexpr auto &get(User &u) noexcept {
    if constexpr(Ind == 0)
      return u.id;
    else if constexpr(Ind == 1)
      return u.name;
    else if constexpr(Ind == 2)
      return u.age;
  }

  // Const getter
  template<size_t Ind>
  constexpr auto &get(User const &u) noexcept {
    if constexpr(Ind == 0)
      return u.id;
    else if constexpr(Ind == 1)
      return u.name;
    else if constexpr(Ind == 2)
      return u.age;
  }
}

// How many parts this decomposes into
template <> struct std::tuple_size<User> : public std::integral_constant<size_t, 3> { };

// Type of each part, we peek at our get() for the answer.
template<size_t Ind> struct std::tuple_element<Ind, User> {
  using type = decltype(std::get<User, Ind>);
};
```

Cool. Your type is decomposable (bonus: this means this code can be used).

```cpp
for(auto &[id, username, age] : users)
  std::cout << "[" << id << "]: " << username << " is " << age << " years old.\n";
```

That's it. Now you can store your type in CQL and do fast queries on any of these parts.

# Lookups
First of all, all lookup tables use transparent comparators. Feel free to use lookup methods with anything comparable to your type parts.

To get your data out again, the simplest method is `std::shared_ptr<Entry const> Table::lookup<N>(T const &)`. This will do a lookup on the `N`th part of your type and grab one that is equal and hand you a shared pointer to it.

# Iteration
You can iterate over the container like any other:
```cpp
CQL::Table<MyType> table;

table.emplace("Hello", "world!");

for(MyType const &val : table) {
  std::cout << val << "\n";
}
```

Do note though, that this iteration has undefined order. If you want to iterate over a specific variable, you use `vbegin<N>()` and `vend<N>()` iterators:

```cpp
CQL::Table<MyType> table;

table.emplace("Hello", "world!");

for(auto it = table.vbegin<0>(); it != table.vend<0>(); ++ it) {
  std::cout << *it << "\n";
}
```

There are also `rvbegin<N>()` and `rvend<N>()` for your reverse iteration needs.

# Updating entries

Something that you might notice quite quickly is that when you're working with your objects inside the tables, you are in one way or another handed a `MyType const &`. This is to prevent accidental writing to the non-mutable members. Writing to these will not update the lookup tables, so please be `const` correct.

You can update values by asking the `Table` nicely. Just pass your `std::shared_ptr<Entry const>` into `Table::update(std::shared_ptr<Entry const>, T &&newVal)` with the value you want to set. There are also similar functions like `Table::swap(std::shared_ptr<Entry const>, T &&newVal)` (swaps new value with the current one). You can also pass these an iterator and change any part that you're **not currently iterating over**.

# Queries
Now you want to get down and dirty with some awesome queries without wasting any time looping through your entire table in linear time.

All query results are passed to a Functor using the `>>=` operator:
```cpp
CQL::Table<MyType> table;

std::vector<MyType> tableDump;

table.all() >>= [](MyType const &entry) {
  tableDump.emplace_back(entry);
}
```

You can apply the `range<N>(lower, upper)` query to get all the entries where `lower <= std::get<N>(entry) && std::get<N>(entry) <= upper`. For example:

```cpp
CQL::Table<MyType> table;

std::vector<MyType> smallAges;

table.range<2>(0, 10) >>= [](MyType const &entry) {
  smallAges.emplace_back(entry);
}
```

You probably don't want to store them into your container though, but you can work with your objects just like normal inside your functor.

Ranges can combine with set intersect or union:

```cpp
table.range<0>(0, 0) || table.range<1>(0, 0) >>= [...];
table.range<0>(0, 0) && table.range<1>(0, 0) >>= [...];
```

And you can combine any valid range expression with a predicate:

```cpp
(db.range<0>(0, 0) || db.range<1>(0, 0))
    && db.pred([](Point const &entry) {
  return entry.first != entry.second;
}) >>= [&](Point const &entry) {
  std::cout << "Point: " << entry << "\n";
};
```

The above code first does a union of the sets where a `Point` lies on the x and y axis respectively. Then the predicate throws out any `Point`s where `x == y`.

You can always look in the `Tests` directory to see some sample implementations of anything in this readme.
