//////////////////////////////////////////////////////////////////////////////////////
// This file is distributed under the University of Illinois/NCSA Open Source License.
// See LICENSE file in top directory for details.
//
// Copyright (c) 2016 Jeongnim Kim and QMCPACK developers.
//
// File developed by: 
//
// File created by: Jeongnim Kim, jeongnim.kim@intel.com, Intel Corp.
//////////////////////////////////////////////////////////////////////////////////////
// -*- C++ -*-
/** @file allocator.hpp
 */
#ifndef QMCPLUSPLUS_ALLOCATOR_H
#define QMCPLUSPLUS_ALLOCATOR_H

#include <config.h>
#include <vector>
#include <cstdlib>
#include "simd/Mallocator.hpp"

namespace qmcplusplus
{
  template<class T>
#if defined(__bgq__)
    using aligned_allocator=std::allocator<T>;
#else
    using aligned_allocator=Mallocator<T, QMC_CLINE>;
#endif
  template<class T>
    using aligned_vector = std::vector<T,aligned_allocator<T> >;

}

/** return size in T's of allocated aligned memory
 */
template<typename T, size_t ALIGN = QMC_CLINE> inline size_t getAlignedSize(size_t n)
{
  constexpr size_t ND=ALIGN/sizeof(T);
  return ((n+ND-1)/ND)*ND;
}

template<typename T, size_t ALIGN = QMC_CLINE> inline size_t getAlignment()
{
  return ALIGN/sizeof(T);
}

#endif
