// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Containers/List.h"

namespace stp {

/** \n List
 * A List is a resizable array (std::vector equivalent).
 * https://en.wikipedia.org/wiki/List_(abstract_data_type)
 */

/** \fn void List::RemoveLast()
 * Removes the last item from the list.
 * The list must not be empty.
 */

/** \fn void List::RemoveAt(int at)
 * Removes an item at the specified index.
 */

/** \fn void List::RemoveRange(int at, int n)
 * Removes a range of items.
 * The given slice must be enclosed by [0..size).
 */

const char* ToNullTerminated(const List<char>& string) {
  ASSERT(!string.Contains('\0'));
  if (string.capacity() != 0) {
    auto* cstr = string.data();
    *(const_cast<char*>(cstr) + string.size()) = '\0';
    return cstr;
  }
  static char NullCharacter = '\0';
  return &NullCharacter;
}

} // namespace stp
