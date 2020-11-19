#pragma once

//todo compare presision for floats/doubles
//todo printer densed, printer pretty, printer json
//todo 


#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include <string_view>
#include <map>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <type_traits>

#include "byte_vector.hpp"
#include "byte_range.hpp"
#include "byte_range_ascii.hpp"
#include "byte_range_hex.hpp"


/// helper constant to fail static_assert and report type
template<class> inline constexpr bool always_false_v = false;
   
/// From https://en.cppreference.com/w/cpp/utility/variant/visit
/// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
/// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
   
// template<class T=void>
struct m5g_value 
{
   using string = std::string;
   template<class...Ts> using vector  = std::vector<Ts...>;
   template<class...Ts> using variant = std::variant<Ts...>;
   template<class...Ts> using stdmap  = std::map<Ts...>;
   using ostream = std::ostream;
   
   struct nil_t { 
      constexpr bool operator< (nil_t const&)const{return true;}
      constexpr bool operator==(nil_t const&)const{return true;}
   };
   static constexpr nil_t nil{};

   using bin_t = byte_vector;
   using blob_t = bin_t;
   
   // using array_t = std::vector<m5g_value>;
   struct array_t : std::vector<m5g_value>{ //declare struct instead of using type directly for better compile-time messages
      using std::vector<m5g_value>::vector;
   };
   
   // using map_t  = std::map<m5g_value,m5g_value>;
   struct map_t : std::map<m5g_value,m5g_value>{ //declare struct instead of using type directly for better compile-time messages
      using std::map<m5g_value,m5g_value>::map;
   };
   
   struct ext_t{}; //todo
   
   using time_t =  std::chrono::nanoseconds;
   
   
   // enum kind_t {
   //    nil=0, boolean, sigint, unsint, float32, float64, str, bin, arr, map, ext, time
   // } kind = nil;
   
   using var_t = std::variant<nil_t,bool,int64_t,uint64_t,float,double,string,bin_t,array_t,map_t>;//,ext_t,time_t>;
   
   var_t var;  // the only variable member
   
   template<class...Ts>
   constexpr m5g_value(Ts&&...vs) : var(std::forward<Ts>(vs)...) {}
   
   // template<class> constexpr m5g_value(m5g_value&&val) : var(std::forward<m5g_value>(val).var) {}
   constexpr m5g_value(m5g_value&&val)     { *this = std::move(val); }
   constexpr m5g_value(m5g_value &val)     { *this = val; }
   constexpr m5g_value(m5g_value const&val){ *this = val; }
   /* constexpr */ m5g_value& operator=(m5g_value&&val)      { this->var=std::move(val.var); val.var={}; return *this; }
   /* constexpr */ m5g_value& operator=(m5g_value const&val) { this->var=val.var;                        return *this; }
   
   
   auto get_as_optional_long_double()const{
      return std::visit(
         [&](auto const& arg)->std::optional<long double>{
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_arithmetic_v<T> and not std::is_same_v<T,bool>){
               return arg;
            }else{
               return {};
            }
         }, this->var);
   };
   
   /// Compare for sorting in map
   /// bin_t,array_t,map_t,ext_t should never be a key in map
   bool operator<(m5g_value const& other) const{
      // if(this->var.index() == other.var.index()){
      //    return std::visit(
      //       [&](auto const& arg)->bool{
      //          using T = std::decay_t<decltype(arg)>;
      //          return arg < std::get<T>(other.var);
      //       }, this->var);
      // }else{
      //    auto this_opt_ld  = this->get_as_optional_long_double();
      //    auto other_opt_ld = other.get_as_optional_long_double();
      //    if(this_opt_ld and other_opt_ld){
      //       return *this_opt_ld < *other_opt_ld;
      //    }
      //    return this->var.index() < other.var.index();
      // }
      auto this_opt_ld  = this->get_as_optional_long_double();
      auto other_opt_ld = other.get_as_optional_long_double();
      if(this_opt_ld and other_opt_ld){
         return *this_opt_ld < *other_opt_ld;
      }
      return this->var < other.var;
   }
   
   bool operator==(m5g_value const& other) const{
      auto this_opt_ld  = this->get_as_optional_long_double();
      auto other_opt_ld = other.get_as_optional_long_double();
      if(this_opt_ld and other_opt_ld){
         return *this_opt_ld == *other_opt_ld;
      }
      return this->var == other.var;
   }
   
   friend auto operator<<(ostream&os, m5g_value const& val) ->ostream& {
      std::visit([&](auto&& arg) {
         using T = std::decay_t<decltype(arg)>;
         using std::is_same_v;
         if constexpr (is_same_v<T, nil_t>)
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
               os << comma << k << ": " << w, comma = ", ";
            os << "}";
         }else if constexpr (is_same_v<T, time_t>){
            os << arg.count() << "ns";
         }else if constexpr (is_same_v<T, string>){
            os << std::quoted(arg);
         }else
            os << arg;
            //static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }, val.var);
      return os;
   }
};


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

struct m5g_byte_stream :byte_vector
{
   enum format : uint8_t {
      /// Nil format stores nil in 1 byte.
      nil = 0xc0,
      falso = 0xc2,
      truo = 0xc3,
      
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
      fixarr_id_mask = 0b11110000, //(uint8_t)~fixarr_len_mask,
      // fixarr_len_mask = (uint8_t)~fixarr_id_mask, //0b00001111,
      fixarr_len_max = 0b00001111,
      
      /// array 16 stores an array whose length is upto (2^16)-1 elements:
      arr16 = 0xdc,
      
      /// array 32 stores an array whose length is upto (2^32)-1 elements:s
      arr32 = 0xdd,
      
      /// fixmap stores a map whose length is upto 15 elements
      fixmap_id = 0b10000000,  ///1001XXXX
      fixmap_id_mask = 0b11110000,
      fixmap_len_max = 0b00001111,
      // fixmap_len_mask = 0b00001111,
      // fixmap_id_mask = (uint8_t)~fixmap_len_mask,
      
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
   
   // byte_vector stream;
   
   friend byte_vector& operator<<(byte_vector& bv, enum format ef){
      return bv << (uint8_t)ef;
   }
   
   m5g_byte_stream& operator<<(m5g_value const& val){
      byte_vector& bv = *this;
      std::visit(overloaded{
         [](auto&&a){
            static_assert(always_false_v<decltype(a)>,"expected overload for type");
         },
         [&](m5g_value::nil_t arg){ 
            bv << nil;
         },
         [&](bool b){ 
            bv << (b ? truo : falso); 
         },
         [&](int64_t const& i){
            if((i & neg_fixnum_data_mask) == i){
               bv << int8_t(i | neg_fixnum_id);
            }else if(int8_t(i) == i){
               bv << int8 << int8_t(i);
            }else if(int16_t(i) == i){
               bv << int16 << int16_t(i);
            }else if(int32_t(i) == i){
               bv << int32 << int32_t(i);
            }else{
               bv << int64 << i;
            }
         },
         [&](uint64_t const& u){
            if((u & pos_fixnum_data_mask) == u){
               bv << uint8_t(u | pos_fixnum_id);
            }else if(uint8_t(u) == u){
               bv << uint8 << uint8_t(u);
            }else if(uint16_t(u) == u){
               bv << uint16 << uint16_t(u);
            }else if(uint32_t(u) == u){
               bv << uint32 << uint32_t(u);
            }else{
               bv << uint64 << u;
            }
         },
         [&](float const& f){
            bv << float32 << f;
         },
         [&](double const& f){
            bv << float64 << f;
         },
         [&](std::string const& str){
            auto size = str.size();
            if(size == (size & fixstr_len_mask)){
               bv << uint8_t(size | fixstr_id) << str;
            }else if(uint8_t(size) == size){
               bv << str8 << uint8_t(size) << str;
            }else if(uint16_t(size) == size){
               bv << str16 << uint16_t(size) << str;
            }else if(uint32_t(size) == size){
               bv << str32 << uint32_t(size) << str;
            }
         },
         [&](m5g_value::bin_t const& bytes){
            auto size = bytes.size();
            if(uint8_t(size) == size){
               bv << bin8 << uint8_t(size);
            }else if(uint16_t(size) == size){
               bv << bin16 << uint16_t(size);
            }else if(uint32_t(size) == size){
               bv << bin32 << uint32_t(size);
            }
            bv << bytes;
         },
         [&](m5g_value::array_t const& arr){
            auto size = arr.size();
            if(size <= fixarr_len_max)
               bv << uint8_t(size | fixarr_id);
            else if(uint16_t(size) == size)
               bv << arr16 << uint16_t(size);
            else if(uint32_t(size) == size)
               bv << arr32 << uint32_t(size);
            for(auto&each:arr)
               *this << each;
         },
         [&](m5g_value::map_t const& m){
            auto size = m.size();
            if(size <= fixmap_len_max)
               bv << uint8_t(size | fixmap_id);
            else if(uint16_t(size) == size)
               bv << map16 << uint16_t(size);
            else if(uint32_t(size) == size)
               bv << map32 << uint32_t(size);
            for(auto&[k,v]:m)
               *this << k << v;
         }
      },val.var);
      return *this;
   }
   
   
   
   // // m5g_byte_stream& operator<<(std::nullptr_t n){
   // //    stream << nil;
   // //    return *this;
   // // }
   
   // // m5g_byte_stream& operator<<(std::nullopt_t n){
   // //    return *this << std::nullptr_t{};
   // // }
   
   // m5g_byte_stream& operator<<(bool b){
   //    stream << (b ? truo : falso);
   //    return *this;
   // }
   
   // m5g_byte_stream& operator<<(int64_t i){
   //    if((i & neg_fixnum_data_mask) == i){
   //       stream << int8_t(i | neg_fixnum_id);
   //    }else if(int8_t(i) == i){
   //       stream << int8 << int8_t(i);
   //    }else if(int16_t(i) == i){
   //       stream << int16 << int16_t(i);
   //    }else if(int32_t(i) == i){
   //       stream << int32 << int32_t(i);
   //    }else{
   //       stream << int64 << i;
   //    }
   //    return *this;
   // }
   
   // m5g_byte_stream& operator<<(uint64_t u){
   //    if((u & pos_fixnum_data_mask) == u){
   //       stream << uint8_t(u | pos_fixnum_id);
   //    }else if(uint8_t(u) == u){
   //       stream << uint8 << uint8_t(u);
   //    }else if(uint16_t(u) == u){
   //       stream << uint16 << uint16_t(u);
   //    }else if(uint32_t(u) == u){
   //       stream << uint32 << uint32_t(u);
   //    }else{
   //       stream << uint64 << u;
   //    }
   //    return *this;
   // }
   
   // m5g_byte_stream& operator<<(float f){
   //    stream << float32 << f;
   //    return *this;
   // }
   // m5g_byte_stream& operator<<(double f){
   //    stream << float64 << f;
   //    return *this;
   // }
   
   // m5g_byte_stream& operator<<(std::string_view const& str){
   //    auto size = str.size();
   //    if(size == (size & fixstr_len_mask)){
   //       stream << uint8_t(size | fixstr_id) << str;
   //    }else if(uint8_t(size) == size){
   //       stream << str8 << uint8_t(size) << str;
   //    }else if(uint16_t(size) == size){
   //       stream << str16 << uint16_t(size) << str;
   //    }else if(uint32_t(size) == size){
   //       stream << str32 << uint32_t(size) << str;
   //    }
   //    return *this;
   // }
   
   // m5g_byte_stream& operator<<(byte_range const& bytes){
   //    auto size = bytes.size();
   //    if(uint8_t(size) == size){  //(bytes.size() <= UINT8_MAX){
   //       stream << bin8 << uint8_t(size) << bytes;
   //    }else if(uint16_t(size) == size){
   //       stream << bin16 << uint16_t(size) << bytes;
   //    }else if(uint32_t(size) == size){
   //       stream << bin32 << uint32_t(size) << bytes;
   //    }
   //    return *this;
   // }
   
   // template<class T>
   // m5g_byte_stream& operator<<(std::array<T,3> const& arr){
   //    auto size = arr.size();
   //    if(size == (size & fixarr_len_mask)){
   //       stream << uint8_t(size | fixarr_id) << arr;
   //    // }else if(uint8_t(size) == size){
   //    //    stream << str8 << uint8_t(size) << arr;
   //    }else if(uint16_t(size) == size){
   //       stream << str16 << uint16_t(size) << arr;
   //    }else if(uint32_t(size) == size){
   //       stream << str32 << uint32_t(size) << arr;
   //    }
   //    return *this;
   // }
   
   // template<class T>
   // m5g_byte_stream& operator<<(std::optional<T> const& o){
   //    if(o.has_value()){
   //       *this << *o;
   //    }else{
   //       stream << nil;
   //    }
   //    return *this;
   // }
   
}; //struct m5g_byte_stream


namespace m5g {
   using value = m5g_value;
   using nil_t = m5g_value::nil_t;
   constexpr auto nil = m5g_value::nil;
   using bin   = m5g_value::bin_t;  
   // using blob  = m5g_value::blob_t;
   using array = m5g_value::array_t;
   using arr   = m5g_value::array_t;
   using map   = m5g_value::map_t;
   //using map = m5g_map;
   //using string = m5g_string;
   using stream = m5g_byte_stream;
   using byte_stream = m5g_byte_stream;
   
   // constexpr m5g::value nilval{}, valnil{};
}

namespace m5gpack {
   using m5gval   = m5g::value;
   using m5gnil_t = m5g::nil_t;
   constexpr auto m5gnil = m5g::nil;
   // using m5gstr = m5g::string;
   using m5gbin   = m5g::bin;
   // using m5gblob  = m5g::blob;
   using m5garr   = m5g::array;
   using m5gmap   = m5g::map;
};
