// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is an internal atomic implementation, use base/atomicops.h instead.

#ifndef STP_BASE_THREAD_ATOMICOPSINTERNALSATOMICWORDCOMPAT_H_
#define STP_BASE_THREAD_ATOMICOPSINTERNALSATOMICWORDCOMPAT_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Type/Basic.h"

// AtomicWord is a synonym for intptr_t, and Atomic32 is a synonym for int32_t,
// which in turn means int. On some LP32 platforms, intptr_t is an int, but
// on others, it's a long. When AtomicWord and Atomic32 are based on different
// fundamental types, their pointers are incompatible.
//
// This file defines function overloads to allow both AtomicWord and Atomic32
// data to be used with this interface.
//
// On LP64 platforms, AtomicWord and Atomic64 are both always long,
// so this problem doesn't occur.

#if !CPU(64BIT)

namespace stp {
namespace subtle {

inline AtomicWord NoBarrier_CompareAndSwap(volatile AtomicWord* ptr,
                                           AtomicWord old_value,
                                           AtomicWord new_value) {
  return NoBarrier_CompareAndSwap(
      reinterpret_cast<volatile Atomic32*>(ptr), old_value, new_value);
}

inline AtomicWord NoBarrier_AtomicExchange(volatile AtomicWord* ptr,
                                           AtomicWord new_value) {
  return NoBarrier_AtomicExchange(
      reinterpret_cast<volatile Atomic32*>(ptr), new_value);
}

inline AtomicWord NoBarrier_AtomicIncrement(volatile AtomicWord* ptr,
                                            AtomicWord increment) {
  return NoBarrier_AtomicIncrement(
      reinterpret_cast<volatile Atomic32*>(ptr), increment);
}

inline AtomicWord Barrier_AtomicIncrement(volatile AtomicWord* ptr,
                                          AtomicWord increment) {
  return Barrier_AtomicIncrement(
      reinterpret_cast<volatile Atomic32*>(ptr), increment);
}

inline AtomicWord Acquire_CompareAndSwap(volatile AtomicWord* ptr,
                                         AtomicWord old_value,
                                         AtomicWord new_value) {
  return subtle::Acquire_CompareAndSwap(
      reinterpret_cast<volatile Atomic32*>(ptr), old_value, new_value);
}

inline AtomicWord Release_CompareAndSwap(volatile AtomicWord* ptr,
                                         AtomicWord old_value,
                                         AtomicWord new_value) {
  return subtle::Release_CompareAndSwap(
      reinterpret_cast<volatile Atomic32*>(ptr), old_value, new_value);
}

inline void NoBarrier_Store(volatile AtomicWord *ptr, AtomicWord value) {
  NoBarrier_Store(
      reinterpret_cast<volatile Atomic32*>(ptr), value);
}

inline void Acquire_Store(volatile AtomicWord* ptr, AtomicWord value) {
  return subtle::Acquire_Store(
      reinterpret_cast<volatile Atomic32*>(ptr), value);
}

inline void Release_Store(volatile AtomicWord* ptr, AtomicWord value) {
  return subtle::Release_Store(
      reinterpret_cast<volatile Atomic32*>(ptr), value);
}

inline AtomicWord NoBarrier_Load(volatile const AtomicWord *ptr) {
  return NoBarrier_Load(
      reinterpret_cast<volatile const Atomic32*>(ptr));
}

inline AtomicWord Acquire_Load(volatile const AtomicWord* ptr) {
  return subtle::Acquire_Load(
      reinterpret_cast<volatile const Atomic32*>(ptr));
}

inline AtomicWord Release_Load(volatile const AtomicWord* ptr) {
  return subtle::Release_Load(
      reinterpret_cast<volatile const Atomic32*>(ptr));
}

} // namespace subtle
} // namespace stp

#endif // !CPU(64BIT)

#endif // STP_BASE_THREAD_ATOMICOPSINTERNALSATOMICWORDCOMPAT_H_