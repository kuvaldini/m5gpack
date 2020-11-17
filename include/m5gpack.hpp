#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include <string_view>
#include <map>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "byte_vector.hpp"
#include "byte_range.hpp"
#include "byte_range_ascii.hpp"
#include "byte_range_hex.hpp"

/*
Formats
Overview
format name 	first byte (in binary) 	first byte (in hex)
positive fixint 	0xxxxxxx 	0x00 - 0x7f
fixmap 	1000xxxx 	0x80 - 0x8f
fixarray 	1001xxxx 	0x90 - 0x9f
fixstr 	101xxxxx 	0xa0 - 0xbf
nil 	11000000 	0xc0
(never used) 	11000001 	0xc1
false 	11000010 	0xc2
true 	11000011 	0xc3
bin 8 	11000100 	0xc4
bin 16 	11000101 	0xc5
bin 32 	11000110 	0xc6
ext 8 	11000111 	0xc7
ext 16 	11001000 	0xc8
ext 32 	11001001 	0xc9
float 32 	11001010 	0xca
float 64 	11001011 	0xcb
uint 8 	11001100 	0xcc
uint 16 	11001101 	0xcd
uint 32 	11001110 	0xce
uint 64 	11001111 	0xcf
int 8 	11010000 	0xd0
int 16 	11010001 	0xd1
int 32 	11010010 	0xd2
int 64 	11010011 	0xd3
fixext 1 	11010100 	0xd4
fixext 2 	11010101 	0xd5
fixext 4 	11010110 	0xd6
fixext 8 	11010111 	0xd7
fixext 16 	11011000 	0xd8
str 8 	11011001 	0xd9
str 16 	11011010 	0xda
str 32 	11011011 	0xdb
array 16 	11011100 	0xdc
array 32 	11011101 	0xdd
map 16 	11011110 	0xde
map 32 	11011111 	0xdf
negative fixint 	111xxxxx 	0xe0 - 0xff
*/

// namespace m5gpack {
   
   // helper constant to fail static_assert and report type
   template<class> inline constexpr bool always_false_v = false;
      
   // From https://en.cppreference.com/w/cpp/utility/variant/visit
   // helper type for the visitor #4
   template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
   // explicit deduction guide (not needed as of C++20)
   template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
      
   // template<class T=void>
   struct m5g_value 
   {
      using string = std::string;
      template<class...Ts> using vector = std::vector<Ts...>;
      template<class...Ts> using variant = std::variant<Ts...>;
      template<class...Ts> using stdmap = std::map<Ts...>;
      using ostream = std::ostream;
      
      struct nil_t {};
      using bin_t = byte_vector;
      using array_t = std::vector<m5g_value>;
      using map_t  = std::map<m5g_value,m5g_value>;
      struct ext_t{}; //todo
      using time_t =  std::chrono::nanoseconds;

      // struct member_t {
      //    m5g_value<T> key;
      //    m5g_value<T> value;
      // };
      
      // enum kind_t {
      //    nil=0, boolean, sigint, unsint, float32, float64, str, bin, arr, map, ext, time
      // } kind = nil;
      
      using var_t = std::variant<nil_t,bool,int64_t,uint64_t,double,string,bin_t,array_t,map_t>;//,ext_t,time_t>;
      
      var_t var;  // the only variable member
      
      template<class...Ts>
      constexpr m5g_value(Ts&&...vs) : var(std::forward<Ts>(vs)...) {}
      
      // template<class> constexpr m5g_value(m5g_value&&val) : var(std::forward<m5g_value>(val).var) {}
      m5g_value(m5g_value&&val) { *this = std::move(val); } //: var{std::move(val.var)} { val.var = {}; }
      m5g_value(m5g_value &val) = default;
      m5g_value(m5g_value const&val) = default;
      m5g_value& operator=(m5g_value&&val) { this->var=std::move(val.var); val.var={}; return *this; }
      m5g_value& operator=(m5g_value const&val) = default;
      
      
      // // template<class>
      // m5g_value& operator=(m5g_value const&val) {
      //    this->var = val.var;
      //    return *this;
      // }
      // m5g_value& operator=(m5g_value&val) {
      //    this->var = val.var;
      //    return *this;
      // }
      
      // template<class...Ts>
      // constexpr decltype(auto) operator=(Ts&&...vs) { var = {vs}; return *this; }
      
      // friend decltype(auto) operator<<(ostream&os, bin_t const& b) {
      //    return os << byte_range_ascii(b);
      // }
      
      /// Compare for sorting in map
      constexpr bool operator<(m5g_value const& other) const{
         if(this->var.index() == other.var.index()){
            // bin_t,array_t,map_t,ext_t should never be a key in map
            return std::visit<bool>(overloaded {
               [&](auto&&arg) { return arg < std::get<decltype(arg)>(other.var); },
               [](nil_t) { return true; },
               [](bin_t const&) { return true; },
               [](array_t const&) { return true; },
               [](map_t const&) { return true; },
               [](ext_t const&) { return true; },
            }, this->var);
         }else{
            return this->var.index() < other.var.index();
         }
      }
      
      friend auto operator<<(ostream&os, m5g_value const& val) ->ostream& {
         std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            using std::is_same_v;
            /* if constexpr (std::is_same_v<T, int>)
               os << "int with value " << arg;
            else if constexpr (std::is_same_v<T, long>)
               os << "long with value " << arg;
            else if constexpr (std::is_same_v<T, double>)
               os << "double with value " << arg;
            else if constexpr (std::is_same_v<T, std::string>)
               os << "std::string with value " << std::quoted(arg);
            else*/ if constexpr (is_same_v<T, nil_t>)
               os << "nil";
            else if constexpr (is_same_v<T, bool>)
               os << std::boolalpha << arg;
            else if constexpr (is_same_v<T, bin_t>)
               os << byte_range_hex(arg);
            else if constexpr (is_same_v<T, array_t>){
               os << "[";
               auto comma = "";
               for(auto&x:arg)
                  os << comma << x, comma = ", ";
               os << "]";
            }else if constexpr (is_same_v<T, map_t>){
               os << "{";
               auto comma = "";
               for(auto&[k,w]:arg)
                  os << comma << k << " : " << w, comma = ", ";
               os << "}";
            }else if constexpr (is_same_v<T, time_t>){
               os << arg.count() << "ns";
            }else if constexpr (is_same_v<T, string>){
               os << std::quoted(arg);
            }else
               os << arg;
               //static_assert(always_false_v<T>, "non-exhaustive visitor!");
         }, val.var);
         // std::visit(overloaded {
         //    [&](auto arg) { os << arg; },
         //    [&](nil_t arg) { os << "nil"; },
         //    [&](bool arg) { os << std::boolalpha << arg; },
         //    //[&](double arg) { os << std::fixed << arg; },
         //    //[&](const std::string& arg) { os << std::quoted(arg); },
         //    [&](bin_t arg) { os << byte_range(arg); },
         //    // [&](array_t a) { for(auto&v:a) os << v << ","; },
         //    // [&](map_t m) { for(auto&pair:m) os << pair.first << " : " << pair.second; },
         // }, v.var);
         return os;
      }
   };
   
   // std::ostream& operator<<(std::ostream&os, m5g_value const& v) {
   //    std::visit([&](auto&& arg) {
   //       using T = std::decay_t<decltype(arg)>;
   //       using std::is_same_v;
   //       /* if constexpr (std::is_same_v<T, int>)
   //          os << "int with value " << arg;
   //       else if constexpr (std::is_same_v<T, long>)
   //          os << "long with value " << arg;
   //       else if constexpr (std::is_same_v<T, double>)
   //          os << "double with value " << arg;
   //       else if constexpr (std::is_same_v<T, std::string>)
   //          os << "std::string with value " << std::quoted(arg);
   //       else*/ if constexpr (is_same_v<T, m5g_value::nil_t>)
   //          os << "nil";
   //       else if constexpr (is_same_v<T, bool>)
   //          os << std::boolalpha << arg;
   //       else if constexpr (is_same_v<T, m5g_value::bin_t>)
   //          os << byte_range_ascii(arg);
   //       else if constexpr (is_same_v<T, m5g_value::array_t>)
   //          for(auto&x:arg)
   //             os << x << ", ";
   //       else
   //          os << arg;
   //          //static_assert(always_false_v<T>, "non-exhaustive visitor!");
   //    }, v.var);
   //    return os;
   // }
      
      
   // struct m5value;
   // struct nil_t {};
   // using string_t = std::string;
   // using bin_t = byte_vector;
   // using array_t = std::vector<m5value>;
   // using map_t  = std::map<m5value,m5value>;
   // struct ext_t{}; //todo
   // using time_t =  std::chrono::nanoseconds;
   
   // // struct m5value : std::variant<nil_t,bool,int64_t,uint64_t,double,string_t,bin_t,array_t> {};
   // using m5value = std::variant<nil_t,bool,int64_t,uint64_t,double,string_t,bin_t,array_t>;
   
   // auto operator<<(std::ostream&os, m5value const& v) ->std::ostream& {
   //    std::visit([&](auto&& arg) {
   //       using T = std::decay_t<decltype(arg)>;
   //       using std::is_same_v;
   //       /* if constexpr (std::is_same_v<T, int>)
   //          os << "int with value " << arg;
   //       else if constexpr (std::is_same_v<T, long>)
   //          os << "long with value " << arg;
   //       else if constexpr (std::is_same_v<T, double>)
   //          os << "double with value " << arg;
   //       else if constexpr (std::is_same_v<T, std::string>)
   //          os << "std::string with value " << std::quoted(arg);
   //       else*/ if constexpr (is_same_v<T, nil_t>)
   //          os << "nil";
   //       else if constexpr (is_same_v<T, bool>)
   //          os << std::boolalpha << arg;
   //       else if constexpr (is_same_v<T, bin_t>)
   //          os << byte_range_ascii(arg);
   //       else if constexpr (is_same_v<T, array_t>)
   //          for(auto&x:arg)
   //             os << x << ", ";
   //       else if constexpr (std::is_arithmetic_v<T> or is_same_v<T,string_t>)
   //          os << arg;
   //       else
   //          static_assert(always_false_v<T>, "non-exhaustive visitor!");
   //    }, v);
   //    // std::visit(overloaded {
   //    //    [&](auto arg) { os << arg; },
   //    //    [&](nil_t arg) { os << "nil"; },
   //    //    [&](bool arg) { os << std::boolalpha << arg; },
   //    //    //[&](double arg) { os << std::fixed << arg; },
   //    //    //[&](const std::string& arg) { os << std::quoted(arg); },
   //    //    [&](bin_t arg) { os << byte_range(arg); },
   //    //    // [&](array_t a) { for(auto&v:a) os << v << ","; },
   //    //    // [&](map_t m) { for(auto&pair:m) os << pair.first << " : " << pair.second; },
   //    // }, v.var);
   //    return os;
   // }
   
   struct m5g 
   {
      enum format : uint8_t {
         /// Nil format stores nil in 1 byte.
         nil = 0xc0,
         fals = 0xc2,
         tru = 0xc3,
         
         /// positive fixnum stores 7-bit positive integer 0XXXXXXX
         pos_fixnum_id = 0b00000000,
         pos_fixnum_data_mask = 0b01111111,
         pos_fixnum_id_mask = (uint8_t)~pos_fixnum_data_mask,
         
         /// negative fixnum stores 5-bit negative integer 111YYYYY
         neg_fixnum_id = (uint8_t)0b11100000,
         neg_fixnum_data_mask = (uint8_t)0b00011111,
         neg_fixnum_id_mask = (uint8_t)~neg_fixnum_data_mask,
         
         /// uint 8 stores a 8-bit unsigned integer
         uint8  = 0xcc,
         uint16 = 0xcd,
         uint32 = 0xce,
         uint64 = 0xcf,
         
         /// int 8 stores a 8-bit signed integer
         int8  = 0xd0,
         int16 = 0xd1,
         int32 = 0xd2,
         int64 = 0xd3,
         
         /// float 32 stores a floating point number in IEEE 754 single precision floating point number format:
         float32 = 0xca,
         float64 = 0xcb,
         
         /// fixstr stores a byte array whose length is upto 31 bytes:
         fixstr_id = 0b10100000,  ///101XXXXX
         fixstr_len_mask = 0b00011111,
         fixstr_id_mask = (uint8_t)~fixstr_len_mask,
         
         /// str 8 stores a byte array whose length is upto (2^8)-1 bytes:
         str8 = 0xd9,
         
         /// str 16 stores a byte array whose length is upto (2^16)-1 bytes:
         str16 = 0xda,
         
         /// str 32 stores a byte array whose length is upto (2^32)-1 bytes:
         str32 = 0xdb,
         
         /// bin 8 stores a byte array whose length is upto (2^8)-1 bytes:
         bin8 = 0xc4,
         
         /// bin 16 stores a byte array whose length is upto (2^16)-1 bytes:
         bin16 = 0xc5,
         
         /// bin 32 stores a byte array whose length is upto (2^32)-1 bytes:
         bin32 = 0xc6,
         
         /// fixarray stores an array whose length is upto 15 elements:
         fixarr_id = 0b10010000,  ///1001XXXX
         fixarr_len_mask = 0b00001111,
         fixarr_id_mask = (uint8_t)~fixarr_len_mask,
         
         /// array 16 stores an array whose length is upto (2^16)-1 elements:
         arr16 = 0xdc,
         
         /// array 32 stores an array whose length is upto (2^32)-1 elements:s
         arr32 = 0xdd,
         
         /// fixmap stores a map whose length is upto 15 elements
         fixmap_id = 0b10000000,  ///1001XXXX
         fixmap_len_mask = 0b00001111,
         fixmap_id_mask = (uint8_t)~fixmap_len_mask,
         
         /// map 16 stores a map whose length is upto (2^16)-1 elements
         map16 = 0xde,
         map32 = 0xdf,
         
         /// fixext 1 stores an integer and a byte array whose length is 1 byte
         fixext1  = 0xd4,
         fixext2  = 0xd5,
         fixext4  = 0xd6,
         fixext8  = 0xd7,
         fixext16 = 0xd8,
         
         /// ext 8 stores an integer and a byte array whose length is upto (2^8)-1 bytes:
         ext8  = 0xc7,
         ext16 = 0xc8,
         ext32 = 0xc9,
         
         /// timestamp 32 stores the number of seconds that have elapsed since 1970-01-01 00:00:00 UTC
         /// in an 32-bit unsigned integer:
         timestamp32 = 0xd6,
         
         /// timestamp 64 stores the number of seconds and nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
         /// in 32-bit unsigned integers:
         timestamp64 = 0xd7,
         
         /// timestamp 96 stores the number of seconds and nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
         /// in 64-bit signed integer and 32-bit unsigned integer:
         timestamp96 = 0xc7,
      };
      
      using byte_stream = byte_vector;
      
      byte_stream stream;
      
      friend byte_stream& operator<<(byte_stream&bs, enum format ef){
         return bs << (uint8_t)ef;
      }
      
      m5g& operator<<(std::nullptr_t n){
         stream << nil;
         return *this;
      }
      m5g& operator<<(std::nullopt_t n){
         return *this << std::nullptr_t{};
      }
      
      m5g& operator<<(bool b){
         stream << (b ? tru : fals);
         return *this;
      }
      
      m5g& operator<<(int64_t i){
         if((i & neg_fixnum_data_mask) == i){
            stream << int8_t(i | neg_fixnum_id);
         }else if(int8_t(i) == i){
            stream << int8 << int8_t(i);
         }else if(int16_t(i) == i){
            stream << int16 << int16_t(i);
         }else if(int32_t(i) == i){
            stream << int32 << int32_t(i);
         }else{
            stream << int64 << i;
         }
         return *this;
      }
      
      m5g& operator<<(uint64_t u){
         if((u & pos_fixnum_data_mask) == u){
            stream << uint8_t(u | pos_fixnum_id);
         }else if(uint8_t(u) == u){
            stream << uint8 << uint8_t(u);
         }else if(uint16_t(u) == u){
            stream << uint16 << uint16_t(u);
         }else if(uint32_t(u) == u){
            stream << uint32 << uint32_t(u);
         }else{
            stream << uint64 << u;
         }
         return *this;
      }
      
      m5g& operator<<(float f){
         stream << float32 << f;
         return *this;
      }
      m5g& operator<<(double f){
         stream << float64 << f;
         return *this;
      }
      
      m5g& operator<<(std::string_view const& str){
         auto size = str.size();
         if(size == (size & fixstr_len_mask)){
            stream << uint8_t(size | fixstr_id) << str;
         }else if(uint8_t(size) == size){
            stream << str8 << uint8_t(size) << str;
         }else if(uint16_t(size) == size){
            stream << str16 << uint16_t(size) << str;
         }else if(uint32_t(size) == size){
            stream << str32 << uint32_t(size) << str;
         }
         return *this;
      }
      
      m5g& operator<<(byte_range const& bytes){
         auto size = bytes.size();
         if(uint8_t(size) == size){  //(bytes.size() <= UINT8_MAX){
            stream << bin8 << uint8_t(size) << bytes;
         }else if(uint16_t(size) == size){
            stream << bin16 << uint16_t(size) << bytes;
         }else if(uint32_t(size) == size){
            stream << bin32 << uint32_t(size) << bytes;
         }
         return *this;
      }
      
      template<class T>
      m5g& operator<<(std::array<T,3> const& arr){
         auto size = arr.size();
         if(size == (size & fixarr_len_mask)){
            stream << uint8_t(size | fixarr_id) << arr;
         // }else if(uint8_t(size) == size){
         //    stream << str8 << uint8_t(size) << arr;
         }else if(uint16_t(size) == size){
            stream << str16 << uint16_t(size) << arr;
         }else if(uint32_t(size) == size){
            stream << str32 << uint32_t(size) << arr;
         }
         return *this;
      }
      
      template<class T>
      m5g& operator<<(std::optional<T> const& o){
         if(o.has_value()){
            *this << *o;
         }else{
            stream << nil;
         }
         return *this;
      }
      
   }; //struct m5gpack
   
   
// } //namespace m5gpack
