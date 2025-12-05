// SPDX-License-Identifier: (LicenseRef-KooijmanInc-Commercial OR GPL-3.0-only)
// Copyright (c) 2025 Kooijman Incorporate Holding B.V.

#ifndef JSON_H
#define JSON_H

#include <type_traits>
#include <QJsonObject>
#include <QJsonArray>


namespace gx::orm {

template <typename T, typename = void> struct has_member_toJson : std::false_type {};
template <typename T> struct has_member_toJson<T, std::void_t<decltype(std::declval<const T&>().toJson())>> : std::true_type {};
template <typename T, typename = void> struct has_member_fromJson : std::false_type {};
template <typename T> struct has_member_fromJson<T, std::void_t<decltype(T::fromJson(std::declval<QJsonObject>()))>> : std::true_type {};

template <typename T, typename = void> struct has_adl_toJson : std::false_type {};
template <typename T> struct has_adl_toJson<T, std::void_t<decltype(toJson(std::declval<const T&>()))>> : std::true_type {};
template <typename T, typename = void> struct has_adl_fromJson : std::false_type {};
template <typename T> struct has_adl_fromJson<T, std::void_t<decltype(fromJson<T>(std::declval<QJsonObject>()))>> : std::true_type {};

template <typename T>
inline QJsonObject toJsonObject(const T& v) {
    if constexpr (has_member_toJson<T>::value) return v.toJson();
    else if constexpr (has_adl_toJson<T>::value) return toJson(v);
    else { static_assert(sizeof(T) == 0, "Provide toJson() member or ADL toJson(T)"); }
}

template <typename T>
inline T fromJsonObject(const QJsonObject& j) {
    if constexpr (has_member_fromJson<T>::value) return T::fromJson(j);
    else if constexpr (has_adl_fromJson<T>::value) return fromJson<T>(j);
    else { static_assert(sizeof(T) == 0, "Provide T::fromJson(obj) or ADL fromJson<T>(obj)"); }
}

template <typename T>
inline QJsonArray toJsonArray(const QList<T>& list) {
    QJsonArray a;
    for (const auto& v : list) a.append(toJsonObject(v)); return a;
}

template <typename T>
inline QList<T> fromJsonArray(const QJsonArray& a) {
    QList<T> out;
    out.reserve(a.size());
    for (const auto& v : a) out.push_back(fromJsonObject<T>(v.toObject()));
    return out;
}

}

#endif // JSON_H
