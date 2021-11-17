#include "intrusive_list.h"
using namespace intrusive;

base_list_element& base_list_element::operator=(base_list_element&& other) noexcept {
  unlink();
  move_from(other);
  return *this;
}

base_list_element::base_list_element(base_list_element&& other) noexcept: base_list_element() {
  move_from(other);
}

void base_list_element::unlink() noexcept {
  if (in_list()) {
    link(prev, next);
    prev = next = nullptr;
  }
}

void base_list_element::insert_before(base_list_element* pos) {
  link(pos->prev, this);
  link(this, pos);
}

bool base_list_element::in_list() const noexcept {
  return prev != nullptr;
}

base_list_element::~base_list_element() {
  unlink();
}

void base_list_element::move_from(base_list_element& other) noexcept {
  if (&other == other.next) {
    next = prev = this;
    return;
  }

  if (other.prev) {
    link(other.prev, this);
  }
  if (other.next) {
    link(this, other.next);
  }
  other.prev = other.next = nullptr;
}

void base_list_element::link(base_list_element* l, base_list_element* r) {
  l->next = r;
  r->prev = l;
}
