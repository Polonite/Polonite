// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEM_REFCOUNTEDTHREADSAFE_H_
#define STP_BASE_MEM_REFCOUNTEDTHREADSAFE_H_

#include "Base/Mem/RefPtr.h"
#include "Base/Sync/AtomicRefCount.h"

namespace stp {

class BASE_EXPORT RefCountedThreadSafeBase {
 public:
  RefCountedThreadSafeBase()
      : ref_count_(1)
      #ifndef NDEBUG
      , in_dtor_(false)
      #endif
      {}

  ~RefCountedThreadSafeBase() {
    #ifndef NDEBUG
    ASSERT(in_dtor_, "RefCountedThreadSafeBase object deleted without calling DecRef()");
    #endif
  }

  bool HasOneRef() const {
    return AtomicRefCountIsOne(&const_cast<RefCountedThreadSafeBase*>(this)->ref_count_);
  }

  void IncRef() const {
    #ifndef NDEBUG
    ASSERT(!in_dtor_);
    #endif
    AtomicRefCountInc(&ref_count_);
  }

 protected:
  bool DecRefBase() const {
    #ifndef NDEBUG
    ASSERT(!in_dtor_);
    ASSERT(!AtomicRefCountIsZero(&ref_count_));
    #endif
    if (!AtomicRefCountDec(&ref_count_)) {
      #ifndef NDEBUG
      in_dtor_ = true;
      #endif
      return true;
    }
    return false;
  }

 private:
  mutable AtomicRefCount ref_count_;
  #ifndef NDEBUG
  mutable bool in_dtor_;
  #endif

  DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
};

template<typename T>
class RefCountedThreadSafe : public RefCountedThreadSafeBase {
 public:
  void DecRef() const {
    if (DecRefBase())
      delete static_cast<const T*>(this);
  }

 protected:
  RefCountedThreadSafe() = default;
};

} // namespace stp

#endif // STP_BASE_MEM_REFCOUNTEDTHREADSAFE_H_