#pragma once

#include <algorithm>
#include <cassert>
#include <type_traits>

namespace intrusive {
struct default_tag;

struct base_list_element {
  template <typename T, typename Tag>
  friend struct list;

  base_list_element() noexcept = default;

  base_list_element(base_list_element const&) = delete;
  base_list_element& operator=(base_list_element const&) = delete;

  base_list_element(base_list_element&& other) noexcept;

  base_list_element& operator=(base_list_element&& other) noexcept;

  bool in_list() const noexcept;

  ~base_list_element();

private:
  base_list_element* prev{nullptr};
  base_list_element* next{nullptr};

  void move_from(base_list_element& other) noexcept;

  static void link(base_list_element* l, base_list_element* r);
  
  void unlink() noexcept;

  void insert_before(base_list_element* pos);
};

template <typename Tag = default_tag>
struct list_element : base_list_element {};

template <typename T, typename Tag = default_tag>
struct list {
  static_assert(std::is_base_of_v<list_element<Tag>, T>,
                "You should derive from list_element");

  template <typename ValueType>
  struct base_iterator;

  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<T const>;

  using untagged = base_list_element;
  using tagged = list_element<Tag>;

  list() noexcept {
    root.next = root.prev = &root;
  }

  list(list const&) = delete;
  list& operator=(list const&) = delete;

  list(list&& other) noexcept : root(std::move(other.root)) {
    other.root.next = other.root.prev = &other.root;
  }

  list& operator=(list&& other) noexcept {
    clear();
    root = std::move(other.root);
    other.root.next = other.root.prev = &other.root;
    return *this;
  }

  ~list() {
    clear();
  }

  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  void push_back(T& elt) noexcept {
    insert(end(), elt);
  }

  void pop_back() noexcept {
    erase(std::prev(end()));
  }

  T& back() noexcept {
    return *std::prev(end());
  }

  T const& back() const noexcept {
    return *std::prev(end());
  }

  void push_front(T& elt) noexcept {
    insert(begin(), elt);
  }

  void pop_front() noexcept {
    erase(begin());
  }

  T& front() noexcept {
    return *begin();
  }

  T const& front() const noexcept {
    return *begin();
  }

  iterator begin() noexcept {
    return iterator(root.next);
  }

  const_iterator begin() const noexcept {
    return const_iterator(root.next);
  }

  iterator end() noexcept {
    return iterator(&root);
  }

  const_iterator end() const noexcept {
    return const_iterator(&root);
  }

  iterator as_iterator(T& elt) noexcept {
    return iterator(&static_cast<tagged&>(elt));
  }

  const_iterator as_iterator(T& elt) const noexcept {
    return const_iterator(&static_cast<tagged&>(elt));
  }

  bool empty() const noexcept {
    return root.prev == &root;
  }

  iterator insert(iterator pos, T& elt) noexcept {
    if (&static_cast<tagged&>(elt) == pos.ptr) {
      return pos;
    }
    static_cast<tagged&>(elt).unlink();
    static_cast<tagged&>(elt).insert_before(pos.ptr);

    return std::prev(pos);
  }

  iterator erase(const_iterator pos) noexcept {
    iterator res = iterator(pos.ptr->prev);
    pos.ptr->unlink();
    return res;
  }

  void splice(const_iterator pos, list&, const_iterator first, const_iterator last) noexcept {
    if (first == last) {
      return;
    }

    auto before_first_other = std::prev(first);
    auto last_other = std::prev(last);
    auto first_this = std::prev(pos);

    untagged::link(last_other.ptr, pos.ptr);
    untagged::link(first_this.ptr, first.ptr);
    untagged::link(before_first_other.ptr, last.ptr);
  }

private:
  untagged root;
};

template <typename T, typename Tag>
template <typename ValueType>
struct list<T, Tag>::base_iterator {
  friend list;

  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = ValueType;
  using pointer = ValueType*;
  using reference = ValueType&;

  base_iterator() = default;
  base_iterator(base_iterator const& other) : ptr(other.ptr) {}

  template <typename U, typename Y = std::enable_if_t<!std::is_const_v<U>>>
  base_iterator(base_iterator<U> const& other) : ptr(other.ptr) {}

  reference operator*() const {
    return *static_cast<pointer>(static_cast<tagged*>(ptr));
  }
  pointer operator->() const {
    return static_cast<pointer>(static_cast<tagged*>(ptr));
  }

  base_iterator& operator++() & {
    ptr = ptr->next;
    return *this;
  }
  base_iterator operator++(int) & {
    base_iterator tmp(*this);
    ++*this;
    return tmp;
  }

  base_iterator& operator--() & {
    ptr = ptr->prev;
    return *this;
  }
  base_iterator operator--(int) & {
    base_iterator tmp(*this);
    --*this;
    return tmp;
  }

  friend bool operator==(base_iterator const& a, base_iterator const& b) {
    return a.ptr == b.ptr;
  }

  friend bool operator!=(base_iterator const& a, base_iterator const& b) {
    return a.ptr != b.ptr;
  }

private:
  untagged* ptr{nullptr};
  explicit base_iterator(untagged const* ptr) : ptr(const_cast<untagged*>(ptr)) {}
};

} // namespace intrusive
