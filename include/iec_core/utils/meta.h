#pragma once

#include <type_traits>

namespace engines
{
template <typename ValueExchangeType> class BPFEngine;
} // namespace engines

namespace handlers
{
template <typename ValueExchangeType> class NumberCrusher;
} // namespace handlers

namespace utils
{

template <typename T> struct ValueExchange;
template <typename T> struct ValueExchangeBlocking;

/// \brief Placeholder empty data type.
struct placeholder_t
{
};

template <typename T1, typename T2> //
struct rebind;

template <typename T2> //
struct rebind<ValueExchange<placeholder_t>, T2>
{
    static_assert(!std::is_same_v<placeholder_t, T2>, "Specified placeholder type, change it!");
    typedef ValueExchange<T2> type;
};

template <typename T2> //
struct rebind<ValueExchangeBlocking<placeholder_t>, T2>
{
    static_assert(!std::is_same_v<placeholder_t, T2>, "Specified placeholder type, change it!");
    typedef ValueExchangeBlocking<T2> type;
};

template <typename T2> //
struct rebind<engines::BPFEngine<placeholder_t>, T2>
{
    static_assert(!std::is_same_v<placeholder_t, T2>, "Specified placeholder type, change it!");
    typedef engines::BPFEngine<T2> type;
};

template <typename T2> //
struct rebind<handlers::NumberCrusher<placeholder_t>, T2>
{
    static_assert(!std::is_same_v<placeholder_t, T2>, "Specified placeholder type, change it!");
    typedef handlers::NumberCrusher<T2> type;
};

template <typename T1, typename T2> //
using rebind_t = typename rebind<T1, T2>::type;

} // namespace utils
