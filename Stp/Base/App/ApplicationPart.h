// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_APP_APPLICATIONPART_H_
#define STP_BASE_APP_APPLICATIONPART_H_

#include "Base/String/StringSpan.h"

namespace stp {

class ApplicationPart;

struct ApplicationPartInfo {
  constexpr ApplicationPartInfo(StringSpan name) : name(name) {}

  StringSpan name;

  // Array of dependencies, null terminated.
  ApplicationPart* const* deps = nullptr;

  // Called before application starts, after all dependencies were initialized.
  void (*init)() = nullptr;
  // Called before application dies, and before any dependency is finalized.
  void (*fini)() = nullptr;
};

class ApplicationPart {
 public:
  StringSpan getName() const { return info_->name; }
  ApplicationPart* const* getDependencies() const { return info_->deps; }

  void init();
  void fini();

  enum Status {
    Unregistered = -1,
    Registering = 0,
    Registered = 1,
  };

  const ApplicationPartInfo* info_;
  Status status_;

  ApplicationPart* prev_;
  ApplicationPart* next_;
};

#define APPLICATION_PART_INITIALIZER(info) \
  { &info, ApplicationPart::Unregistered, nullptr, nullptr }

inline void ApplicationPart::init() {
  if (info_->init)
    info_->init();
}

inline void ApplicationPart::fini() {
  if (info_->fini)
    info_->fini();
}

} // namespace stp

#endif // STP_BASE_APP_APPLICATIONPART_H_
