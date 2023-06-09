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

#ifndef KOKKOS_IMPL_STATICCRSGRAPH_FACTORY_HPP
#define KOKKOS_IMPL_STATICCRSGRAPH_FACTORY_HPP

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <Kokkos_Core.hpp>
#include <Kokkos_StaticCrsGraph.hpp>

namespace Kokkos {

template <class DataType, class Arg1Type, class Arg2Type, class Arg3Type,
          typename SizeType>
inline typename StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                               SizeType>::HostMirror
create_mirror_view(const StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                                        SizeType>& view,
                   std::enable_if_t<ViewTraits<DataType, Arg1Type, Arg2Type,
                                               Arg3Type>::is_hostspace>* = 0) {
  return view;
}

template <class DataType, class Arg1Type, class Arg2Type, class Arg3Type,
          typename SizeType>
inline typename StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                               SizeType>::HostMirror
create_mirror(const StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                                   SizeType>& view) {
  // Force copy:
  // using alloc = Impl::ViewAssignment<Impl::ViewDefault>; // unused
  using staticcrsgraph_type =
      StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type, SizeType>;

  typename staticcrsgraph_type::HostMirror tmp;
  typename staticcrsgraph_type::row_map_type::HostMirror tmp_row_map =
      create_mirror(view.row_map);
  typename staticcrsgraph_type::row_block_type::HostMirror
      tmp_row_block_offsets = create_mirror(view.row_block_offsets);

  // Allocation to match:
  tmp.row_map = tmp_row_map;  // Assignment of 'const' from 'non-const'
  tmp.entries = create_mirror(view.entries);
  tmp.row_block_offsets =
      tmp_row_block_offsets;  // Assignment of 'const' from 'non-const'

  // Deep copy:
  deep_copy(tmp_row_map, view.row_map);
  deep_copy(tmp.entries, view.entries);
  deep_copy(tmp_row_block_offsets, view.row_block_offsets);

  return tmp;
}

template <class DataType, class Arg1Type, class Arg2Type, class Arg3Type,
          typename SizeType>
inline typename StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                               SizeType>::HostMirror
create_mirror_view(const StaticCrsGraph<DataType, Arg1Type, Arg2Type, Arg3Type,
                                        SizeType>& view,
                   std::enable_if_t<!ViewTraits<DataType, Arg1Type, Arg2Type,
                                                Arg3Type>::is_hostspace>* = 0) {
  return create_mirror(view);
}
}  // namespace Kokkos

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace Kokkos {

template <class StaticCrsGraphType, class InputSizeType>
inline typename StaticCrsGraphType::staticcrsgraph_type create_staticcrsgraph(
    const std::string& label, const std::vector<InputSizeType>& input) {
  using output_type  = StaticCrsGraphType;
  using entries_type = typename output_type::entries_type;
  using work_type    = View<
      typename output_type::size_type[], typename output_type::array_layout,
      typename output_type::device_type, typename output_type::memory_traits>;

  output_type output;

  // Create the row map:

  const size_t length = input.size();

  {
    work_type row_work("tmp", length + 1);

    typename work_type::HostMirror row_work_host = create_mirror_view(row_work);

    size_t sum       = 0;
    row_work_host[0] = 0;
    for (size_t i = 0; i < length; ++i) {
      row_work_host[i + 1] = sum += input[i];
    }

    deep_copy(row_work, row_work_host);

    output.entries = entries_type(label, sum);
    output.row_map = row_work;
  }

  return output;
}

//----------------------------------------------------------------------------

template <class StaticCrsGraphType, class InputSizeType>
inline typename StaticCrsGraphType::staticcrsgraph_type create_staticcrsgraph(
    const std::string& label,
    const std::vector<std::vector<InputSizeType> >& input) {
  using output_type  = StaticCrsGraphType;
  using entries_type = typename output_type::entries_type;

  static_assert(entries_type::rank == 1, "Graph entries view must be rank one");

  using work_type = View<
      typename output_type::size_type[], typename output_type::array_layout,
      typename output_type::device_type, typename output_type::memory_traits>;

  output_type output;

  // Create the row map:

  const size_t length = input.size();

  {
    work_type row_work("tmp", length + 1);

    typename work_type::HostMirror row_work_host = create_mirror_view(row_work);

    size_t sum       = 0;
    row_work_host[0] = 0;
    for (size_t i = 0; i < length; ++i) {
      row_work_host[i + 1] = sum += input[i].size();
    }

    deep_copy(row_work, row_work_host);

    output.entries = entries_type(label, sum);
    output.row_map = row_work;
  }

  // Fill in the entries:
  {
    typename entries_type::HostMirror host_entries =
        create_mirror_view(output.entries);

    size_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
      for (size_t j = 0; j < input[i].size(); ++j, ++sum) {
        host_entries(sum) = input[i][j];
      }
    }

    deep_copy(output.entries, host_entries);
  }

  return output;
}

}  // namespace Kokkos

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif /* #ifndef KOKKOS_IMPL_CRSARRAY_FACTORY_HPP */
