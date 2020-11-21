#pragma once

#include <vector>
#include <cstdint>

// using byte_vector = std::vector<uint8_t>;
struct byte_vector : std::vector<uint8_t> {
    using strsize_t = uint16_t;
    using std::vector<uint8_t>::vector;
    auto operator<=>(const byte_vector&) const = default;
};

template<typename T>
inline auto operator<<(byte_vector&bv, T const& v) 
->std::enable_if_t<std::is_arithmetic_v<T> 
                   and not std::is_same_v<T,bool>, byte_vector& >
{
    static_assert(std::is_arithmetic_v<T>,"operator<<(byte_vector&,T) implemented for arithmetic types only");
    //ToDo C++20 if constexpr (std::endian::native == std::endian::little)
#if BYTE_ORDER == LITTLE_ENDIAN
    std::reverse_iterator<uint8_t*> first{(uint8_t *) (&v + 1)}, last{(uint8_t *) (&v)};
#else
    uint8_t *first=(uint8_t*)&v, *last=(uint8_t*)(&v+1);
#endif
    bv.insert(end(bv), first, last);
    return bv;
}

inline auto operator<<(byte_vector&vec, byte_vector const& bv) ->byte_vector&
{
    vec.insert(vec.end(), bv.begin(), bv.end());
    return vec;
}

template<typename T>
inline auto operator<<(byte_vector&vec, T const& br)
->decltype(br.begin(), br.end(), vec)  // SFINAE check if type has member functions
                                       // ToDo C++20 requires
{
    vec.insert(vec.end(), br.begin(), br.end());
    return vec;
}

#include <string_view>
inline auto operator<<(byte_vector&vec, std::string_view const& sv) ->byte_vector&
{
    vec<<byte_vector::strsize_t(sv.size());
    vec.insert(vec.end(), sv.begin(), sv.end());  // Alternative way: vec.reserve(vec.size()+sv.size()); for(auto&c:sv) vec<<c;
    return vec;
}

inline auto operator<<(byte_vector&vec, char const* str) ->byte_vector&
{
    //ToDo optimization wanted: first copy bytes till \0 then fill size.
    // while(*str!='\0')
    //     vec<<*str++;
    // return vec;
    return vec<<std::string_view(str);
}
template<size_t N>
inline auto operator<<(byte_vector&vec, char const str[N]) ->byte_vector&
{
    return vec<<std::string_view(str,N-1);
}

#include <optional>
template<typename T>
inline auto operator<<(byte_vector&vec, std::optional<T> const& arg) ->byte_vector&
{
    if(arg)
        vec << *arg;
    return vec;
    //return arg ? vec << arg->value : vec;
}

#include <tuple>
template<typename... Ts>
inline auto operator<<(byte_vector&vec, std::tuple<Ts...> const& theTuple) ->byte_vector&
{
    std::apply(
        [&](Ts const &... tupleArgs) {
            ((vec << tupleArgs), ...);
        }, theTuple
    );
    return vec;
}

#if 0
#include <iostream>
#include "byte_range_ascii.hpp"

template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T> &os, byte_vector const& bv){
    return os << byte_range_ascii(bv);
}
#endif
