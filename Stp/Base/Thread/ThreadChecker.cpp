// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Thread/ThreadChecker.h"

#if ASSERT_IS_ON

namespace stp {

ThreadChecker::ThreadChecker() {
  EnsureThreadIdAssigned();
}

ThreadChecker::~ThreadChecker() {}

bool ThreadChecker::calledOnValidThread() const {
  EnsureThreadIdAssigned();
  AutoLock auto_lock(borrow(lock_));
  return valid_thread_ == NativeThread::currentHandle();
}

void ThreadChecker::DetachFromThread() {
  AutoLock auto_lock(borrow(lock_));
  valid_thread_ = InvalidNativeThreadHandle;
}

void ThreadChecker::EnsureThreadIdAssigned() const {
  AutoLock auto_lock(borrow(lock_));
  if (valid_thread_ == InvalidNativeThreadHandle)
    valid_thread_ = NativeThread::currentHandle();
}

} // namespace stp

#endif // ASSERT_IS_ON
