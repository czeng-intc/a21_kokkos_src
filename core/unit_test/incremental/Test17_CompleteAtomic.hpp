//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER

#include <Kokkos_Core.hpp>
#include <array>
#include <random>
#include <gtest/gtest.h>

/// @Kokkos_Feature_Level_Required:17
// Incremental test for atomic views.
// In this test we sort N integers into num_buckets number of buckets based on
// their rermainder, i.e., a histogram based on remainder. Since the number of
// integers is greater than the number of buckets, we use atomic views for the
// sorted histogram.

namespace Test {

using value_type      = int;
const int N           = 1000;
const int num_buckets = 10;

template <class ExecSpace>
struct TestAtomicView {
  // 1D  View of int
  using View = typename Kokkos::View<value_type *, ExecSpace>;

  // 1D atomic view
  using atomic_view =
      typename Kokkos::View<value_type *, ExecSpace,
                            Kokkos::MemoryTraits<Kokkos::Atomic> >;

  void atomicView() {
    // Use default_random_engine object to introduce randomness.
    std::default_random_engine generator;
    // Initialize uniform_int_distribution class.
    std::uniform_int_distribution<int> distribution(0, N);

    // Device and Host views of N number of integers
    View d_data("deviceData_1D", N);
    auto h_data = create_mirror_view(d_data);

    // Atomic Device and Host views of histogram
    atomic_view d_hist("histogram", num_buckets);
    auto h_hist = create_mirror_view(d_hist);

    // An array to store correct results for verification
    std::array<int, num_buckets> correct_results;

    // Initialize host side histogram arrays
    for (int i = 0; i < num_buckets; ++i) {
      h_hist(i)          = 0;
      correct_results[i] = 0;
    }

    // Fill host data with integers from the distribution object.
    for (int i = 0; i < N; ++i) h_data(i) = distribution(generator);

    // Copy data from host to device
    Kokkos::deep_copy(d_data, h_data);
    Kokkos::deep_copy(d_hist, h_hist);

    // Update histogram
    Kokkos::parallel_for(
        Kokkos::RangePolicy<ExecSpace>(0, N),
        KOKKOS_LAMBDA(const int i) { d_hist(d_data(i) % num_buckets)++; });

    // Perform the same computation on host for correctness test.
    for (int i = 0; i < N; ++i) correct_results[h_data(i) % num_buckets]++;

    // Copy the histogram back to host
    Kokkos::deep_copy(h_hist, d_hist);

    // Validate results
    for (int i = 0; i < num_buckets; ++i)
      ASSERT_EQ(correct_results[i], h_hist(i));
  }
};

// atomic view tests
TEST(TEST_CATEGORY, incr_17_atomicView) {
  TestAtomicView<TEST_EXECSPACE> test;
  test.atomicView();
}

}  // namespace Test
