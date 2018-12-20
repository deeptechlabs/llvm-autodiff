// RUN: %clangxx_asan -fno-rtti -DBUILD_SO1 -fPIC -shared %s -o %dynamiclib1
// RUN: %clangxx_asan -fno-rtti -DBUILD_SO2 -fPIC -shared %s -o %dynamiclib2
// RUN: %clangxx_asan -fno-rtti %s %ld_flags_rpath_exe1 %ld_flags_rpath_exe2 -o %t
// RUN: %env_asan_opts=fast_unwind_on_malloc=0:detect_odr_violation=2 not %run %t 2>&1 | FileCheck %s

// FIXME: Somehow does not detect ODR on powerpc64be
// XFAIL: powerpc64

struct XYZ {
  virtual void foo();
};

#if defined(BUILD_SO1)

void XYZ::foo() {}

#elif defined(BUILD_SO2)

void XYZ::foo() {}

#else

int main() {}

#endif

// CHECK: AddressSanitizer: odr-violation
// CHECK-NEXT: 'vtable for XYZ'
// CHECK-NEXT: 'vtable for XYZ'
