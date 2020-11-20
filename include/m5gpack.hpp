#pragma once

//todo compare presision for floats/doubles
//todo printer densed, printer pretty, printer json
//todo 

// #include <endian>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>
#include <string_view>
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
   // template<class...Ts> using stdmap  = std::map<Ts...>;
   using ostream = std::ostream;
   
   struct nil_t { 
      constexpr bool operator< (nil_t const&)const{return true;}
      constexpr bool operator==(nil_t const&)const{return true;}
   };
   static constexpr nil_t nil{};

   using bin_t = byte_vector;
   using blob_t = bin_t;
   
   struct array_t : std::vector<m5g_value>{ //declare struct instead of using type directly for better compile-time messages
      using std::vector<m5g_value>::vector;
   };
   
   using pair_t = std::pair<m5g_value,m5g_value>;
   struct map_t : std::vector<pair_t>{ //declare struct instead of using type directly for better compile-time messages
      using std::vector<pair_t>::vector;
   };
   
   struct ext_t{
      uint8_t type;
      byte_vector data;
      auto operator<=>(const ext_t&) const = default;
   };
   
   using time_t =  std::chrono::nanoseconds;
   
   
   // enum kind_t {
   //    nil=0, boolean, sigint, unsint, float32, float64, str, bin, arr, map, ext, time
   // } kind = nil;
   
   using var_t = std::variant<nil_t,bool,int64_t,uint64_t,float,double,string,bin_t,array_t,map_t,ext_t>;//,time_t>;
   
   var_t var;  // the only variable member
   
   template<class...Ts>
   constexpr m5g_value(Ts&&...vs) : var(std::forward<Ts>(vs)...) {}
   
   // template<class> constexpr m5g_value(m5g_value&&val) : var(std::forward<m5g_value>(val).var) {}
   constexpr m5g_value(m5g_value&&val)     { *this = std::move(val); }
   constexpr m5g_value(m5g_value &val)     { *this = val; }
   constexpr m5g_value(m5g_value const&val){ *this = val; }
   /* constexpr */ m5g_value& operator=(m5g_value&&val)      { this->var=std::move(val.var); val.var={}; return *this; }
   /* constexpr */ m5g_value& operator=(m5g_value const&val) { this->var=val.var;                        return *this; }
   
   constexpr m5g_value(unsigned u):var(uint64_t(u)){}
   
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

struct m5g_byte_stream : std::vector<uint8_t> 
{
   using std::vector<uint8_t>::vector;
   
   enum format : uint8_t {
      /// Nil format stores nil in 1 byte.
      nil = 0xc0,
      falso = 0xc2,
      truo = 0xc3,
      
      /// positive fixnum stores 7-bit positive integer 0XXXXXXX
      pos_fixnum_id  = 0b00000000,
      pos_fixnum_max = 127,
      // pos_fixnum_data_mask = 0b01111111,
      // pos_fixnum_id_mask = (uint8_t)~pos_fixnum_data_mask,
      
      /// negative fixnum stores 5-bit negative integer 111YYYYY
      neg_fixnum_id = (uint8_t)0b11100000,
      neg_fixnum_id_mask = (uint8_t)0b11100000,
      // neg_fixnum_min = (uint8_t)~neg_fixnum_id_mask,
      // neg_fixnum_data_mask = (uint8_t)0b00011111,
      
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
      fixstr_id_mask  = 0b11100000,
      fixstr_len_mask = (uint8_t)~fixstr_id_mask,
      fixstr_len_max = fixstr_len_mask,
      // fixstr_id_mask = (uint8_t)~fixstr_len_mask,
      
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
      
      /// array 32 stores an array whose length is upto (2^32)-1 elements:
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
   
   m5g_byte_stream& operator<<(m5g_value const& val){
      std::visit(overloaded{
         [](auto&&a){
            static_assert(always_false_v<decltype(a)>,"expected overload for type");
         },
         [&](m5g_value::nil_t arg){ 
            detail() << nil;
         },
         [&](bool b){ 
            detail() << (b ? truo : falso); 
         },
         [&](int64_t const& i){
            if(i < 0 and i >= -32){  // 5 bit allowed
               detail() << int8_t(i);
            }else if(int8_t(i) == i){
               detail() << int8 << int8_t(i);
            }else if(int16_t(i) == i){
               detail() << int16 << int16_t(i);
            }else if(int32_t(i) == i){
               detail() << int32 << int32_t(i);
            }else{
               detail() << int64 << i;
            }
         },
         [&](uint64_t const& u){
            if(u <= 127){  // 7 bit allowed
               detail() << uint8_t(u);
            }else if(uint8_t(u) == u){
               detail() << uint8 << uint8_t(u);
            }else if(uint16_t(u) == u){
               detail() << uint16 << uint16_t(u);
            }else if(uint32_t(u) == u){
               detail() << uint32 << uint32_t(u);
            }else{
               detail() << uint64 << u;
            }
         },
         [&](float const& f){
            detail()<< float32 << f;
         },
         [&](double const& f){
            detail() << float64 << f;
         },
         [&](std::string const& str){
            auto size = str.size();
            if(size == (size & fixstr_len_mask)){
               detail() << uint8_t(size | fixstr_id) << str;
            }else if(uint8_t(size) == size){
               detail() << str8 << uint8_t(size) << str;
            }else if(uint16_t(size) == size){
               detail() << str16 << uint16_t(size) << str;
            }else if(uint32_t(size) == size){
               detail() << str32 << uint32_t(size) << str;
            }else
               std::terminate(); //throw m5g_exception("wrong size is more than uint32_t");
         },
         [&](m5g_value::bin_t const& bytes){
            auto size = bytes.size();
            if(uint8_t(size) == size){
               detail() << bin8 << uint8_t(size);
            }else if(uint16_t(size) == size){
               detail() << bin16 << uint16_t(size);
            }else if(uint32_t(size) == size){
               detail() << bin32 << uint32_t(size);
            }else
               std::terminate(); //throw m5g_exception("wrong size is more than uint32_t");
            detail() << bytes;
         },
         [&](m5g_value::array_t const& arr){
            auto size = arr.size();
            if(size <= fixarr_len_max)
               detail() << uint8_t(size | fixarr_id);
            else if(uint16_t(size) == size)
               detail() << arr16 << uint16_t(size);
            else if(uint32_t(size) == size)
               detail() << arr32 << uint32_t(size);
            else
               std::terminate(); //throw m5g_exception("wrong size is more than uint32_t");
            for(auto&each:arr)
               *this << each;
         },
         [&](m5g_value::map_t const& m){
            auto size = m.size();
            if(size <= fixmap_len_max)
               detail() << uint8_t(size | fixmap_id);
            else if(uint16_t(size) == size)
               detail() << map16 << uint16_t(size);
            else if(uint32_t(size) == size)
               detail() << map32 << uint32_t(size);
            else
               std::terminate(); //throw m5g_exception("wrong size is more than uint32_t");
            for(auto&[k,v]:m)
               *this << k << v;
         },
         [&](m5g_value::ext_t const& e){
            auto size = e.data.size();
            if(size == 1)
               detail() << fixext1;
            else if(size == 2)
               detail() << fixext2;
            else if(size == 4)
               detail() << fixext4;
            else if(size == 8)
               detail() << fixext8;
            else if(size == 16)
               detail() << fixext16;
            else if(uint8_t(size) == size)
               detail() << ext8 << uint16_t(size);
            else if(uint16_t(size) == size)
               detail() << ext16 << uint16_t(size);
            else if(uint32_t(size) == size)
               detail() << ext32 << uint32_t(size);
            else
               std::terminate();//throw
            detail() << e.type << e.data;
         }
      },val.var);
      return *this;
   }
   
   auto operator<<(m5g_byte_stream const& bs) ->m5g_byte_stream&
   {
      insert(end(), bs.begin(), bs.end());
      return *this;
   }
   
private:
   struct detail_ /* : m5g_byte_stream */ {
      m5g_byte_stream& mbs;
      
      template<typename T>
      auto operator<<(T const& v) 
      ->std::enable_if_t<std::is_arithmetic_v<T> 
                        and not std::is_same_v<T,bool>, detail_& >
      {
       #if (BYTE_ORDER == LITTLE_ENDIAN) //C++20 if constexpr (std::endian::native == std::endian::little)
         std::reverse_iterator<uint8_t*> first{(uint8_t *) (&v + 1)}, last{(uint8_t *) (&v)};
       #else
         uint8_t *first=(uint8_t*)&v, *last=(uint8_t*)(&v+1);
       #endif
         mbs.insert(mbs.end(), first, last);
         return *this;
      }
      
      detail_& operator<<(enum format ef){
         return *this << (uint8_t)ef;
      }
      
      detail_& operator<<(std::string const& s)
      {
         mbs.insert(mbs.end(), s.begin(), s.end());  // Alternative way: vec.reserve(vec.size()+sv.size()); for(auto&c:sv) vec<<c;
         return *this;
      }
      
      detail_& operator<<(m5g_value::bin_t const& b)
      {
         mbs.insert(mbs.end(), b.begin(), b.end());  // Alternative way: vec.reserve(vec.size()+sv.size()); for(auto&c:sv) vec<<c;
         return *this;
      }
   };
   detail_ detail() { return {*this}; }
      
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
