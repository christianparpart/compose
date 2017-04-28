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

template<typename Container>
class Compose {
 public:
  using iterator = typename Container::iterator;
  using value_type = typename Container::value_type;
  using Self = Compose<Container>;

  explicit Compose(Container& container) : container_(container) {}

  auto begin() { return container_.begin(); }
  auto end() { return container_.end(); }

  auto take(size_t limit) {
    return ComposeTake<Compose<Container>>(*this, limit);
  }

  template<typename Predicate>
  auto select(Predicate pred) {
    return ComposeSelect<Compose<Container>, Predicate>(*this, pred);
  }

  template<typename Map>
  auto map(Map m) {
    return ComposeMap<Container, Map, value_type, decltype(m(value_type()))>(container_, m);
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

template<typename Container, typename Map, typename InputType, typename OutputType>
class ComposeMap {
 public:
  //using input_type = typename std::iterator_traits<decltype(Container().begin())>::value_type;
  using input_type = InputType; // typename std::remove_reference<decltype(*Container().begin())>::type;
  using output_type = OutputType; // TODO typename std::result_of_t<Map>;
  using value_type = output_type;

  ComposeMap(Container& that, Map m)
      : container_(that), map_(m) {}

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

  size_t size() {
    size_t total = 0;
    for (const auto& a: *this)
      ++total;
    return total;
  }

  using Self = ComposeMap<Container, Map, InputType, OutputType>;

  template<typename Map2>
  auto map(Map2 m) {
    return ComposeMap<Self, Map2, value_type, decltype(m(value_type()))>(*this, m);
  }

  template<typename Predicate>
  auto select(Predicate pred) {
    return ComposeSelect<Self, Predicate>(*this, pred);
  }

  auto take(size_t limit) {
    return ComposeTake<Self>(*this, limit);
  }

 private:
  Container& container_;
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
class ComposeSelect {
 public:
  ComposeSelect(Container& container, Predicate predicate)
      : container_(container), predicate_(predicate) {}

  using value_type = typename Container::value_type;
  using iterator = SelectIterator<Container, Predicate>;
  iterator begin() { return iterator(this->container_, predicate_); }
  iterator end() { return iterator(this->container_); }

  size_t size() {
    size_t total = 0;
    for (const auto& a: *this)
      ++total;
    return total;
  }

  using Self = ComposeSelect<Container, Predicate>;

  ComposeTake<Self> take(size_t limit) {
    return ComposeTake<Self>(*this, limit);
  }

 private:
  Container& container_;
  Predicate predicate_;
};

template<typename Container>
class ComposeTake {
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
      : container_(container), limit_(limit) {}

  size_t size() {
    size_t total = 0;
    for (const auto& a: *this)
      ++total;
    return total;
  }

  iterator begin() { return iterator(this->container_, limit_); }
  iterator end() { return iterator(this->container_, 0); }

 private:
  Container& container_;
  size_t limit_;
};

template<typename T>
Compose<T> compose(T& container) {
  return Compose<T>(container);
}

