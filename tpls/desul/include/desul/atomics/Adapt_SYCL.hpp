/*
Copyright (c) 2019, Lawrence Livermore National Security, LLC
and DESUL project contributors. See the COPYRIGHT file for details.
Source: https://github.com/desul/desul

SPDX-License-Identifier: (BSD-3-Clause)
*/

#ifndef DESUL_ATOMICS_ADAPT_SYCL_HPP_
#define DESUL_ATOMICS_ADAPT_SYCL_HPP_

#include <desul/atomics/Common.hpp>

// FIXME_SYCL SYCL2020 dictates that <sycl/sycl.hpp> is the header to include
// but icpx 2022.1.0 and earlier versions only provide <CL/sycl.hpp>
#if __has_include(<sycl/sycl.hpp>)
#include <sycl/sycl.hpp>
#else
#include <CL/sycl.hpp>
#endif

namespace desul {
namespace Impl {

#ifdef __clang__
namespace sycl_sync_and_atomics = ::sycl;
#else
namespace sycl_sync_and_atomics = ::sycl;
#endif

template <bool extended_namespace>
using sycl_memory_order = std::conditional_t<extended_namespace,
                                             sycl_sync_and_atomics::memory_order,
                                             sycl::memory_order>;
template <bool extended_namespace>
using sycl_memory_scope = std::conditional_t<extended_namespace,
                                             sycl_sync_and_atomics::memory_scope,
                                             sycl::memory_scope>;

//<editor-fold desc="SYCL memory order">
// The default memory order for sycl::atomic_ref
// can be seq_cst, acq_rel, or relaxed according to the
// "SYCL 2020 Specification (revision 6)", see
// https://registry.khronos.org/SYCL/specs/sycl-2020/html/sycl-2020.html#sec:atomic-references.
// Thus, we map MemoryOrderAcquire and MemoryOrderRelease to acq_rel.
template <class MemoryOrder, bool extended_namespace = true>
struct SYCLMemoryOrder;

template <bool extended_namespace>
struct SYCLMemoryOrder<MemoryOrderSeqCst, extended_namespace> {
  static constexpr sycl_memory_order<extended_namespace> value =
      sycl_memory_order<extended_namespace>::seq_cst;
};
template <bool extended_namespace>
struct SYCLMemoryOrder<MemoryOrderAcquire, extended_namespace> {
  static constexpr sycl_memory_order<extended_namespace> value =
      sycl_memory_order<extended_namespace>::acq_rel;
};
template <bool extended_namespace>
struct SYCLMemoryOrder<MemoryOrderRelease, extended_namespace> {
  static constexpr sycl_memory_order<extended_namespace> value =
      sycl_memory_order<extended_namespace>::acq_rel;
};
template <bool extended_namespace>
struct SYCLMemoryOrder<MemoryOrderAcqRel, extended_namespace> {
  static constexpr sycl_memory_order<extended_namespace> value =
      sycl_memory_order<extended_namespace>::acq_rel;
};
template <bool extended_namespace>
struct SYCLMemoryOrder<MemoryOrderRelaxed, extended_namespace> {
  static constexpr sycl_memory_order<extended_namespace> value =
      sycl_memory_order<extended_namespace>::relaxed;
};
//</editor-fold>

//<editor-fold desc="SYCL memory scope">
template <class MemoryScope, bool extended_namespace = true>
struct SYCLMemoryScope;

template <bool extended_namespace>
struct SYCLMemoryScope<MemoryScopeCore, extended_namespace> {
  static constexpr sycl_memory_scope<extended_namespace> value =
      sycl_memory_scope<extended_namespace>::work_group;
};

template <bool extended_namespace>
struct SYCLMemoryScope<MemoryScopeDevice, extended_namespace> {
  static constexpr sycl_memory_scope<extended_namespace> value =
      sycl_memory_scope<extended_namespace>::device;
};

template <bool extended_namespace>
struct SYCLMemoryScope<MemoryScopeSystem, extended_namespace> {
  static constexpr sycl_memory_scope<extended_namespace> value =
      sycl_memory_scope<extended_namespace>::system;
};
//</editor-fold>

// FIXME_SYCL generic_space isn't available yet for CUDA.
#ifdef __NVPTX__
template <class T, class MemoryOrder, class MemoryScope>
using sycl_atomic_ref = sycl::atomic_ref<T,
                                         SYCLMemoryOrder<MemoryOrder>::value,
                                         SYCLMemoryScope<MemoryScope>::value,
                                         sycl::access::address_space::global_space>;
#else
template <class T, class MemoryOrder, class MemoryScope>
using sycl_atomic_ref = sycl::atomic_ref<T,
                                         SYCLMemoryOrder<MemoryOrder>::value,
                                         SYCLMemoryScope<MemoryScope>::value,
                                         sycl::access::address_space::generic_space>;
#endif

// FIXME_SYCL Use SYCL_EXT_ONEAPI_DEVICE_GLOBAL when available instead
#ifdef DESUL_SYCL_DEVICE_GLOBAL_SUPPORTED
// FIXME_SYCL The compiler forces us to use device_image_scope. Drop this when possible.
template <class T>
using sycl_device_global = sycl::ext::oneapi::experimental::device_global<
    T,
    decltype(sycl::ext::oneapi::experimental::properties(
        sycl::ext::oneapi::experimental::device_image_scope))>;
#endif

}  // namespace Impl
}  // namespace desul

#endif
