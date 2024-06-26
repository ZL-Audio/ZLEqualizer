// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <type_traits>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace farbot
{

namespace detail
{
struct copy_tag;
struct move_tag;
struct constructible_tag;
struct assignable_tag;

template <typename T, typename CopyMoveTag, typename ConstructAssignTag, typename = void> struct is_rt_safe : std::false_type {};


template <typename T> struct is_rt_safe<T, copy_tag, assignable_tag,    typename std::enable_if<std::is_trivially_copy_assignable<T>::value>::type> : std::true_type  {};
template <typename T> struct is_rt_safe<T, copy_tag, constructible_tag, typename std::enable_if<std::is_trivially_copy_constructible<T>::value>::type> : std::true_type  {};
template <typename T> struct is_rt_safe<T, move_tag, assignable_tag,    typename std::enable_if<std::is_trivially_move_assignable<T>::value>::type> : std::true_type  {};
template <typename T> struct is_rt_safe<T, move_tag, constructible_tag, typename std::enable_if<std::is_trivially_move_constructible<T>::value>::type> : std::true_type  {};

// specialisations
template <typename T, typename Tag> struct is_rt_safe<std::vector<T>, move_tag, Tag> : is_rt_safe<T, move_tag, Tag> {};
template <typename T, typename Tag> struct is_rt_safe<std::set<T>, move_tag, Tag> : is_rt_safe<T, move_tag, Tag> {};
template <typename T, typename U, typename Tag> struct is_rt_safe<std::map<T, U>, move_tag, Tag> : std::integral_constant<bool, is_rt_safe<T, move_tag, Tag>::value && is_rt_safe<U, move_tag, Tag>::value> {};
template <typename T, typename U, typename Tag> struct is_rt_safe<std::unordered_map<T, U>, move_tag, Tag> : std::integral_constant<bool, is_rt_safe<T, move_tag, Tag>::value && is_rt_safe<U, move_tag, Tag>::value> {};
template <typename T, typename Tag> struct is_rt_safe<std::unordered_set<T>, move_tag, Tag> : is_rt_safe<T, move_tag, Tag> {};
}

template <typename T> struct is_realtime_copy_assignable    : detail::is_rt_safe<T, detail::copy_tag, detail::assignable_tag> {};
template <typename T> struct is_realtime_copy_constructable : detail::is_rt_safe<T, detail::copy_tag, detail::constructible_tag> {};
template <typename T> struct is_realtime_move_assignable    : detail::is_rt_safe<T, detail::move_tag, detail::assignable_tag> {};
template <typename T> struct is_realtime_move_constructable : detail::is_rt_safe<T, detail::move_tag, detail::constructible_tag> {};

}