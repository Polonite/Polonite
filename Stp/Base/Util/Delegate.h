// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_DELEGATE_H_
#define STP_BASE_UTIL_DELEGATE_H_

#include "Base/Compiler/Cpu.h"
#include "Base/Debug/Assert.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace delegate_impl {

typedef void (*GenericFunction)();

#if COMPILER(MSVC)
class __single_inheritance GenericClass;
class GenericClass {};

// Define a generic class that uses virtual inheritance.
// It has a trival member function that returns the value of the 'this' pointer.
struct GenericVirtualClass : virtual public GenericClass {
  typedef GenericVirtualClass* (GenericVirtualClass::*ProbeFunction)();
  GenericVirtualClass* GetThis() { return this; }
};
#else
class GenericClass;
#endif

typedef void (GenericClass::*GenericMethod)();
typedef void (*GenericStaticMethod)(GenericClass*);

template<int N>
struct SimplifyMethod {
  template<class X, class XMethod, typename GMethod>
  static GenericClass* convert(XMethod method, X* that, GMethod& gmethod) {
    static_assert(N < 0, "unsupported member function pointer");
    return nullptr;
  }
};

#if COMPILER(GCC)

template<>
struct SimplifyMethod<sizeof(GenericMethod)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct GnuMemberFunction {
      union {
        // If even, it's a pointer to the function.
        GSMethod function;
        // If odd, it's the byte offset into the vtable.
        long vtable_offset;
      };
      long this_delta;
    };

    X* that = const_cast<X*>(thatc);
    GnuMemberFunction mf = bitCast<GnuMemberFunction>(method);
    #if CPU(X86_FAMILY)
    auto* object = reinterpret_cast<GenericClass*>(
        reinterpret_cast<char*>(that) + mf.this_delta);

    if (mf.vtable_offset & 1) {
      char* vtable_base = *reinterpret_cast<char**>(object);
      gsmethod = *reinterpret_cast<GSMethod*>(vtable_base + mf.vtable_offset - 1);
    } else {
      gsmethod = mf.function;
    }
    return object;
    #elif CPU(ARM_FAMILY)
    auto* object = reinterpret_cast<GenericClass*>(
        reinterpret_cast<char*>(that) + (mf.this_delta >> 1));

    if (mf.this_delta & 1) {
      char* vtable_base = *reinterpret_cast<char**>(object);
      gsmethod =  *reinterpret_cast<GSMethod*>(vtable_base + mf.vtable_offset);
    } else {
      gsmethod = mf.function;
    }
    return object;
    #else
    # error "cpu not supported"
    #endif // CPU(*)
  }
};

#elif COMPILER(MSVC)

// MSVC has different pointer sizes depending on type of class.
// - single inheritance
// - multiple inheritance
// - virtual inheritance
// - unknown inheritance

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    gsmethod = bitCast<GSMethod>(method);
    X* that = const_cast<X*>(thatc);
    return reinterpret_cast<GenericClass*>(that);
  }
};

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod) + sizeof(int)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct MSVCExtendedMemberFunction {
      GSMethod function;
      int this_delta;
    };

    MSVCExtendedMemberFunction mf = bitCast<MSVCExtendedMemberFunction>(method);

    gsmethod = mf.function;
    X* that = const_cast<X*>(thatc);
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(that) + mf.this_delta);
  }
};

template<>
struct SimplifyMethod<sizeof(GenericStaticMethod) + 2 * sizeof(int)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    struct MSVCVirtualMemberFunction {
      GSMethod function;
      int this_delta;
      int vtable_index;
    };

    MSVCVirtualMemberFunction mf = bitCast<MSVCVirtualMemberFunction>(method);
    int this_delta = mf.this_delta;
    gsmethod = mf.function;

    X* that = const_cast<X*>(thatc);
    static const int VtorDisplacement = sizeof(void*);
    if (mf.vtable_index != 0) {
      // Resolve VTable.
      const int* vtable = *reinterpret_cast<const int* const*>(
          reinterpret_cast<const char*>(that) + VtorDisplacement);

      // |vtable_index| tells us where in the table we should be looking.
      this_delta += VtorDisplacement + *reinterpret_cast<const int*>(
          reinterpret_cast<const char*>(vtable) + mf.vtable_index);
    }
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(that) + this_delta);
  }
};

struct MSVCUnknownMemberFunction {
  GenericStaticMethod function;
  int this_delta;
  int vtor_displacement;
  int vtable_index;
};

template<>
struct SimplifyMethod<sizeof(MSVCUnknownMemberFunction)> {
  template<class X, class XMethod, typename GSMethod>
  static ALWAYS_INLINE GenericClass*
  convert(XMethod method, const X* thatc, GSMethod& gsmethod) {
    MSVCUnknownMemberFunction mf = bitCast<MSVCUnknownMemberFunction>(method);

    GenericClass* gobject = reinterpret_cast<GenericClass*>(thatc);
    int this_delta = mf.this_delta;
    gsmethod = bitCast<GSMethod>(mf.function);

    X* that = const_cast<X*>(thatc);
    // If index is zero, fallback to extended member function.
    if (mf.vtable_index != 0) {
      // Resolve VTable.
      const int* vtable = *reinterpret_cast<const int* const*>(
          reinterpret_cast<const char*>(that) + mf.vtor_displacement);

      // |vtable_index| tells us where in the table we should be looking.
      this_delta += mf.vtor_displacement + *reinterpret_cast<const int*>(
          reinterpret_cast<const char*>(vtable) + mf.vtable_index);
    }
    return reinterpret_cast<GenericClass*>(reinterpret_cast<char*>(gobject) + this_delta);
  }
};

#else
# error "compiler not supported"
#endif // COMPILER(*)

BASE_EXPORT void formatDelegate(TextWriter& out, const StringSpan& opts, void* ptr);

} // namespace delegate_impl

template<typename Signature>
class Delegate;

template<typename TResult, typename... TArgs>
class Delegate<TResult(TArgs...)> {
 public:
  Delegate()
      : gobject_(nullptr), gmethod_(nullptr) {}

  Delegate(nullptr_t)
      : gobject_(nullptr), gmethod_(nullptr) {}

  Delegate(const Delegate& other)
      : gobject_(other.gobject_),
        gmethod_(other.gmethod_) {}

  template<class TClass>
  Delegate(TResult (TClass::*method)(TArgs...), TClass* object) {
    gobject_ = delegate_impl::SimplifyMethod<sizeof(method)>::convert(method, object, gmethod_);
  }

  TResult operator()(TArgs... args) const {
    ASSERT(gmethod_ != nullptr);
    return (*gmethod_)(gobject_, forward<TArgs>(args)...);
  }

  Delegate& operator=(const Delegate& other) {
    gobject_ = other.gobject_;
    gmethod_ = other.gmethod_;
    return *this;
  }

  explicit operator bool() const { return gmethod_ != nullptr; }
  bool isNull() const { return gmethod_ == nullptr; }

  friend void format(TextWriter& out, const Delegate& x, const StringSpan& opts) {
    delegate_impl::formatDelegate(out, opts, reinterpret_cast<void*>(x.gmethod_));
  }

 private:
  delegate_impl::GenericClass* gobject_;
  TResult (*gmethod_)(delegate_impl::GenericClass*, TArgs...);
};

template<typename TClass, typename TThis, typename TResult, typename... TArgs>
inline Delegate<TResult(TArgs...)> makeDelegate(TResult(TClass::*method)(TArgs...), TThis* that) {
  TClass* thatx = that;
  return Delegate<TResult(TArgs...)>(method, thatx);
}

} // namespace stp

#endif // STP_BASE_UTIL_DELEGATE_H_
