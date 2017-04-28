// This file is part of the "x0" project
//   <http://github.com/christianparpart/x0>
//   (c) 2017 Christian Parpart <christian@parpart.family>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#pragma once

#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>

template<typename Container, typename Map, typename InputType, typename OutputType> class ComposeMap;
template<typename Container, typename Predicate> class ComposeSelect;
template<typename Container> class ComposeTake;

template<typename Container, typename Derived>
class Compose {
 public:
  using iterator = typename Container::iterator;
  using value_type = typename Container::value_type;

  explicit Compose(Container& container) : container_(container) {}

  auto begin() { return static_cast<Derived&>(*this).begin(); }
  auto end() { return static_cast<Derived&>(*this).end(); }

  auto take(size_t limit) {
    return ComposeTake<Derived>(static_cast<Derived&>(*this), limit);
  }

  template<typename Predicate>
  auto select(Predicate pred) {
    return ComposeSelect<Derived, Predicate>(static_cast<Derived&>(*this), pred);
  }

  template<typename Map>
  auto map(Map m) {
    return ComposeMap<Derived, Map, value_type, decltype(m(value_type()))>(static_cast<Derived&>(*this), m);
  }

  template<typename Func>
  auto fold(value_type init, Func func) -> value_type {
    for (const auto& a: *this)
      init = func(init, a);

    return init;
  }

  template<typename Func>
  void each_with_index(Func func) {
    size_t i = 0;
    for (const auto& a: *this) {
      func(i, a);
      ++i;
    }
  }

  template<typename Func>
  void each(Func func) {
    for (const auto& a: *this) {
      func(a);
    }
  }

  size_t size() {
    size_t total = 0;
    for (const auto& a: *this)
      ++total;
    return total;
  }

 protected:
  Container& container_;
};

template<typename Container>
class ComposeForward : public Compose<Container, ComposeForward<Container>> {
 public:
  ComposeForward(Container& that)
      : Compose<Container, ComposeForward<Container>>(that) {}

  using iterator = typename Container::iterator;
  using value_type = typename Container::value_type;

  auto begin() { return this->container_.begin(); }
  auto end() { return this->container_.end(); }
  auto size() { return this->container_.size(); }
};

template<typename Container, typename Map, typename InputType, typename OutputType>
class ComposeMap : public Compose<Container, ComposeMap<Container, Map, InputType, OutputType>> {
 public:
  //using input_type = typename std::iterator_traits<decltype(Container().begin())>::value_type;
  using input_type = InputType; // TODO typename std::remove_reference<decltype(*Container().begin())>::type;
  using output_type = OutputType; // TODO typename std::result_of_t<Map>;
  using value_type = output_type;

  ComposeMap(Container& that, Map m)
      : Compose<Container, ComposeMap<Container, Map, InputType, OutputType>>(that),
        map_(m) {}

  class iterator { // {{{
   public:
    iterator(Container& that, Map map)
        : cur_(that.begin()), end_(that.end()), map_(map) {}
    explicit iterator(Container& that)
        : cur_(that.end()), end_(that.end()), map_() {}

    auto operator*() { return map_(*cur_); }
    auto operator->() { return cur_.operator->(); }

    iterator& operator++() {
      if (cur_ != end_) {
        ++cur_;
      }
      return *this;
    }

    bool operator==(const iterator& other) { return cur_ == other.cur_; }
    bool operator!=(const iterator& other) { return cur_ != other.cur_; }

   private:
    typename Container::iterator cur_;
    typename Container::iterator end_;
    std::function<output_type(input_type)> map_;
  }; // }}}

  auto begin() { return iterator(this->container_, map_); }
  auto end() { return iterator(this->container_); }

 private:
  Map map_;
};

template<typename Container, typename Predicate>
class SelectIterator {
 public:
  SelectIterator(Container& that, Predicate pred)
      : cur_(that.begin()), end_(that.end()), predicate_(pred) {
    while (cur_ != end_ && !predicate_(*cur_)) {
      ++cur_;
    }
  }

  explicit SelectIterator(Container& that)
      : cur_(that.end()), end_(that.end()), predicate_() {}

  auto operator*() { return *this->cur_; }
  auto operator->() { return cur_.operator->(); }

  SelectIterator& operator++() {
    for (;;) {
      if (cur_ == end_) {
        break;
      }

      ++cur_;

      if (cur_ == end_) {
        break;
      }

      if (predicate_(*cur_)) {
        return *this;
      }
    }
    return *this;
  }

  bool operator==(const SelectIterator<Container, Predicate>& other)
      { return cur_ == other.cur_; }

  bool operator!=(const SelectIterator<Container, Predicate>& other)
      { return cur_ != other.cur_; }

 private:
  typename Container::iterator cur_;
  typename Container::iterator end_;
  std::function<bool(decltype(*cur_))> predicate_;
};

template<typename Container, typename Predicate>
class ComposeSelect : public Compose<Container, ComposeSelect<Container, Predicate>> {
 public:
  ComposeSelect(Container& container, Predicate predicate)
      : Compose<Container, ComposeSelect<Container, Predicate>>(container),
        predicate_(predicate) {}

  using value_type = typename Container::value_type;
  using iterator = SelectIterator<Container, Predicate>;

  auto begin() { return iterator(this->container_, predicate_); }
  auto end() { return iterator(this->container_); }

 private:
  Predicate predicate_;
};

template<typename Container>
class ComposeTake : public Compose<Container, ComposeTake<Container>> {
 public:
  using value_type = typename Container::value_type;

  class iterator { // {{{
   public:
    iterator(Container& that, size_t limit)
        : cur_(that.begin()), end_(that.end()), limit_(limit) {}

    iterator& operator++() {
      if (limit_ != 0 && cur_ != end_) {
        ++cur_;
        --limit_;
      }
      return *this;
    }

    auto operator*() { return *cur_; }
    auto operator->() { return cur_.operator->(); }

    bool operator==(const iterator& other) { return limit_ == other.limit_; }
    bool operator!=(const iterator& other) { return limit_ != other.limit_; }

   private:
    size_t limit_;
    typename Container::iterator cur_;
    typename Container::iterator end_;
  }; // }}}

  ComposeTake(Container& container, size_t limit)
      : Compose<Container, ComposeTake<Container>>(container),
        limit_(limit) {}

  auto begin() { return iterator(this->container_, limit_); }
  auto end() { return iterator(this->container_, 0); }

 private:
  size_t limit_;
};

template<typename T>
ComposeForward<T> compose(T& container) {
  return ComposeForward<T>(container);
}
