#include <sstream>
#include "gtest/gtest.h"
#include "m5gpack.hpp"
#include "byte_range_ascii.hpp"
#include "byte_vector.hpp"

using namespace std;
using namespace std::literals;
// using namespace m5g;


auto operator<<(ostream&os, m5g::stream const&ms) ->ostream&
{
   return os << byte_range_ascii(ms);
}


TEST(m5gpack,construction)
{
   // 1. default constructable
   m5g::value v1;
   EXPECT_TRUE(v1 == m5g::nil);
   EXPECT_TRUE(std::holds_alternative<m5g::nil_t>(v1.var));
   
   // 2. copy construct
   m5g::value v2(v1);
   EXPECT_TRUE(v2 == m5g::nil);
   EXPECT_TRUE(std::holds_alternative<m5g::nil_t>(v1.var));
   
   // 3. move construct
   m5g::value v3(std::move(v1));
   
   // 4. copy assign
   v2 = -4;
   v1 = v2;
   EXPECT_EQ(v1 , v2);
   EXPECT_TRUE(std::holds_alternative<int64_t>(v1.var));
   
   // 5. move assign
   v1 = std::move(v2);
   EXPECT_EQ(v1 , m5g::value{-4});
   EXPECT_TRUE(std::holds_alternative<int64_t>(v1.var));
   EXPECT_EQ(v2 , m5g::nil);
   EXPECT_TRUE(std::holds_alternative<m5g::nil_t>(v2.var));
   
   // 6. map
   m5g::value v = m5g::map { {"a", m5g::arr{}},
                       {"help", m5g::array{ 
                              "11d", 33.6, m5g_value::array_t{1,2} 
                           }
                       },
                       {m5g::nil, m5g_value::nil},
                       {2,54.3},
                       {0.9,21.3},
                     };
}

/// nil < boolean < (signed int, unsigned int, float, double) < str < bin < arr < map < ext < time
TEST(m5gpack,compare)
{
   EXPECT_LT(m5g::value{} , false);
   EXPECT_LT(m5g::value{false} , true);
   EXPECT_LT(m5g::value{true} , int(-1));
   EXPECT_LT(m5g::value{int(-1)} , 2);
   EXPECT_LT(m5g::value{2} , double(3.14));  // numbers are compared as numbers type does not matter
   EXPECT_LT(m5g::value{3.14} , "hello"s);
   EXPECT_LT(m5g::value{"hello"} , m5g::value{"helly"});
   EXPECT_LT(m5g::value{"hello"} , (m5g::array{1,"222"}));
   EXPECT_LT((m5g::value{m5g::array{1,2,3}}) , (m5g::array{1,2,4}));
   EXPECT_LT((m5g::value{m5g::array{1,2,3}}) , (m5g::array{1,2,3,1}));
   EXPECT_LT(m5g::value{m5g::array{}} , m5g::map{});
   EXPECT_LT((m5g::value{m5g::map{{1,"a"}}}) , (m5g::map{{2,"a"}}));
   EXPECT_LT((m5g::value{m5g::map{{1,"a"}}}) , (m5g::map{{1,"a"},{}}));
   
   EXPECT_EQ(m5g::value{} , m5g::nil);
   EXPECT_EQ(m5g::value{true} , true);
   EXPECT_EQ(m5g::value{int(2)} , 2.);
   EXPECT_EQ(m5g::value{int(2)} , 2ul);
   
   EXPECT_EQ(m5g::value{"hello"}, "hello");
   EXPECT_NE(m5g::value{"hello"}, "Hello");
   
   EXPECT_EQ(m5g::ext{}, m5g::ext{});
   EXPECT_EQ(m5g::ext(1,{'w','o'}), (m5g::value{m5g::ext{1,{'w','o'}}}));
   EXPECT_NE(m5g::value{}, (m5g::value{m5g::ext{1,{'w','o'}}}));
   EXPECT_NE(m5g::nil, (m5g::value{m5g::ext{0xdf,m5g::bin{0x17}}}));
}


TEST(m5gpack,stream_common)
{
   // empty types
   {
      m5g::stream ms;
      auto val = m5g::value{m5g::map{
               {},
               {"", m5g::arr{}},
      }};
      ms << val;
      byte_vector expected {0x82,0xC0,0xC0,0xA0,0x90};
      EXPECT_EQ(ms,expected);
      
      m5g::value received;
      ms >> received;
      EXPECT_EQ(received,val);
   }
   
   // a map of different types
   {
      m5g::stream ms;
      ms << m5g::value{m5g::map{
            {},
            {"HELLO", m5g::arr{1ul,2,3,-1,0x1233456789}},
            {"help", m5g::array{ 
                  "11d", 33.6, m5g_value::array_t{1,-22} 
               }
            },
            {m5g::nil, m5g_value::nil},
            {2, 54.3f},
            {0.9, 21.3},
         }};
      byte_vector expected {
         0x86, 0xC0, 0xC0, 0xA5, 0x48, 0x45, 0x4C, 0x4C, 
         0x4F, 0x95, 0x01, 0xD0, 0x02, 0xD0, 0x03, 0xFF, 
         0xD3, 0x00, 0x00, 0x00, 0x12, 0x33, 0x45, 0x67, 
         0x89, 0xA4, 0x68, 0x65, 0x6C, 0x70, 0x93, 0xA3, 
         0x31, 0x31, 0x64, 0xCB, 0x40, 0x40, 0xCC, 0xCC, 
         0xCC, 0xCC, 0xCC, 0xCD, 0x92, 0xD0, 0x01, 0xEA, 
         0xC0, 0xC0, 0xD0, 0x02, 0xCA, 0x42, 0x59, 0x33, 
         0x33, 0xCB, 0x3F, 0xEC, 0xCC, 0xCC, 0xCC, 0xCC, 
         0xCC, 0xCD, 0xCB, 0x40, 0x35, 0x4C, 0xCC, 0xCC, 
         0xCC, 0xCC, 0xCD, 
      };
      // cerr<<byte_range_ascii(ms)<<endl;
      // cerr<<byte_range_ascii(expected)<<endl;
      EXPECT_EQ(ms,expected);
   }
}
TEST(m5gpack,stream_number)
{
   // nil
   m5g::stream ms;
   ms << m5g::value(m5g::nil);
   EXPECT_EQ(ms,(byte_vector{0xc0}));
   m5g::value recved;
   ms >> recved;
   EXPECT_EQ(recved,m5g::nil);
   
   // bool
   ms << false << true;
   EXPECT_EQ(ms,(byte_vector{0xc2,0xc3}));
   m5g::value recv_f, recv_t;
   ms >> recv_f >> recv_t;
   EXPECT_EQ(recv_f,false);
   EXPECT_EQ(recv_t,true);
   // EXPECT_FALSE(recv_f);
   // EXPECT_TRUE(recv_t);
   
   // uint
   ms << 0u << 127u << 255u << unsigned(65535) << uint64_t(0x1234ABCD) << UINT64_MAX;
   EXPECT_EQ(ms,(byte_vector{0x00,0x7F,0xcc,0xFF,0xcd,0xFF,0xFF,0xce,0x12,0x34,0xAB,0xCD,
                              0xcf,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}));
   // uint deserialize
   m5g::value recv;
   ms >> recv;
   EXPECT_EQ(recv,0u);
   ms >> recv;
   EXPECT_EQ(recv,127u);
   ms >> recv;
   EXPECT_EQ(recv,255u);
   ms >> recv;
   EXPECT_EQ(recv,65535);
   ms >> recv;
   EXPECT_EQ(recv,0x1234ABCD);
   ms >> recv;
   EXPECT_EQ(recv,UINT64_MAX);
   EXPECT_TRUE(ms.empty());
   
   // int
   ms = {};
   ms << -32 << -3 << -1 << int8_t(-33);
   EXPECT_EQ(ms,(m5g::stream{0b11100000,0b11111101,0b11111111,0xd0,uint8_t(-33)}));
   
   ms = {};
   ms << 64 << 127 << 128 << int(1000) << -1000;
   EXPECT_EQ(ms,(m5g::stream{0xd0,64,0xd0,127,0xd1,0,128,0xd1,0x03,0xe8,0xd1,0xfc,0x18}));
   
   ms = {};
   ms << int64_t(-0x1234ABCD) << INT64_MIN;
   EXPECT_EQ(ms,(m5g::stream{0xd2,0xED,0xCB,0x54,0x33, 0xd3,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00}));
   
   // float32
   ms = {};
   ms << 0.f << -1.f << 3.1415926f;
   EXPECT_EQ(ms,(m5g::stream{0xca,0,0,0,0, 0xca,0xBF,0x80,0x00,0x00, 0xca,0x40,0x49,0x0f,0xda}));
   
   // float64
   ms = {};
   ms << double(0) << (double)-1 << 3.1415926;
   EXPECT_EQ(ms,(m5g::stream{0xcb,0,0,0,0,0,0,0,0,
      0xcb,0xBF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
      0xcb,0x40,0x09,0x21,0xfb,0x4d,0x12,0xd8,0x4a}));
}
TEST(m5gpack,stream_str)
{
   { // fixstr
      m5g::stream ms;
      ms << "" << "Hello";
      EXPECT_EQ(ms,(m5g::stream{0xa0, 0xa5,'H','e','l','l','o'}));
      // deserialize
      m5g_value empty,hello;
      ms >> empty >> hello;
      EXPECT_TRUE(std::get<std::string>(empty.var).empty());
      EXPECT_EQ(hello,"Hello");
   }
   { // fixstr max size 15
      m5g::stream ms;
      ms << "1234567890qwertyuiopasdfghjklzx";
      EXPECT_EQ(ms[0], 0xBF);
      EXPECT_EQ(ms[1], '1');
      EXPECT_EQ(*--ms.end(), 'x');
      // deserialize
      m5g_value str;
      ms >> str;
      EXPECT_EQ(str,"1234567890qwertyuiopasdfghjklzx");
   }
   { // str 8
      m5g::stream ms;
      ms << "1234567890qwertyuiopasdfghjklzxc";
      EXPECT_EQ(ms[0], 0xD9);
      EXPECT_EQ(ms[1], 32);
      EXPECT_EQ(ms[2], '1');
      EXPECT_EQ(*--ms.end(), 'c');
      // deserialize
      m5g_value str;
      ms >> str;
      EXPECT_EQ(str,"1234567890qwertyuiopasdfghjklzxc");
   }
   { // str 16
      m5g::stream ms;
      std::string s;
      for(int i=0;i<1000;++i)
         s += "1234567890qwertyuiopasdfghjklzxc";
      ms << s;
      EXPECT_EQ(ms[0], 0xDa);
      EXPECT_EQ(ms[1], s.size()>>8);
      EXPECT_EQ(ms[2], s.size()&0xFF);
      EXPECT_EQ(ms[3], '1');
      EXPECT_EQ(*--ms.end(), 'c');
      // deserialize
      m5g_value str;
      ms >> str;
      EXPECT_EQ(str,s);
   }
   { // str 32
      m5g::stream ms;
      std::string s;
      for(int i=0;i<2048;++i)
         s += "1234567890qwertyuiopasdfghjklzxc";
      ms << s;
      EXPECT_EQ(ms[0], 0xdb);
      EXPECT_EQ(ms[1], s.size()>>24&0xFF);
      EXPECT_EQ(ms[2], s.size()>>16&0xFF);
      EXPECT_EQ(ms[3], s.size()>>8 &0xFF);
      EXPECT_EQ(ms[4], s.size()>>0 &0xFF);
      EXPECT_EQ(ms[5], '1');
      EXPECT_EQ(*--ms.end(), 'c');
      // deserialize
      m5g_value str;
      ms >> str;
      EXPECT_EQ(str,s);
   }
}

TEST(m5gpack,stream_bin)
{
   { // bin 8
      m5g::stream ms;
      m5g::bin b(255);
      std::generate(b.begin(), b.end(), [n=0]()mutable{ return n++; });
      ms << b;
      EXPECT_EQ(ms[0], 0xc4);
      EXPECT_EQ(ms[1], b.size());
      EXPECT_EQ(ms[2], 0);
      EXPECT_EQ(*--ms.end(), b.size()-1);
      // deserialize
      m5g_value binval;
      ms >> binval;
      EXPECT_EQ(binval,b);
   }
   { // bin 16
      m5g::stream ms;
      m5g::bin b(4444);
      std::generate(b.begin(), b.end(), [n=0]()mutable{ return n++; });
      ms << b;
      EXPECT_EQ(ms[0], 0xc5);
      EXPECT_EQ(ms[1], b.size()>>8);
      EXPECT_EQ(ms[2], b.size()&0xFF);
      EXPECT_EQ(ms[3], 0);
      EXPECT_EQ(*--ms.end(), *--b.end());
      // deserialize
      m5g_value binval;
      ms >> binval;
      EXPECT_EQ(binval,b);
   }
   { // bin 32
      m5g::stream ms;
      m5g::bin b(70000);
      std::generate(b.begin(), b.end(), [n=0]()mutable{ return n++; });
      ms << b;
      EXPECT_EQ(ms[0], 0xc6);
      EXPECT_EQ(ms[1], b.size()>>24&0xFF);
      EXPECT_EQ(ms[2], b.size()>>16&0xFF);
      EXPECT_EQ(ms[3], b.size()>>8 &0xFF);
      EXPECT_EQ(ms[4], b.size()>>0 &0xFF);
      EXPECT_EQ(ms[5], 0);
      EXPECT_EQ(*--ms.end(), *--b.end());
      // deserialize
      m5g_value binval;
      ms >> binval;
      EXPECT_EQ(binval,b);
   }
}

TEST(m5gpack,stream_arr)
{
   { /// fixarray stores an array whose length is upto 15 elements
      m5g::stream ms;
      m5g::array a(15);
      ms << a;
      EXPECT_EQ(ms[0], 0x90+a.size());
      EXPECT_EQ(ms[1], 0xc0);
      EXPECT_EQ(*--ms.end(), 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,a);
   }
   { /// array 16 stores an array whose length is upto (2^16)-1 elements
      m5g::stream ms;
      m5g::array a(16);
      ms << a;
      EXPECT_EQ(ms[0], 0xdc);
      EXPECT_EQ(ms[1], a.size()>>8);
      EXPECT_EQ(ms[2], a.size()&0xFF);
      EXPECT_EQ(ms.size()-3, a.size());
      EXPECT_EQ(ms[3], 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,a);
   }
   { /// array 32 stores an array whose length is upto (2^32)-1 elements
      m5g::stream ms;
      m5g::array a(70000);
      ms << a;
      EXPECT_EQ(ms[0], 0xdd);
      EXPECT_EQ(ms[1], a.size()>>24&0xFF);
      EXPECT_EQ(ms[2], a.size()>>16&0xFF);
      EXPECT_EQ(ms[3], a.size()>>8 &0xFF);
      EXPECT_EQ(ms[4], a.size()>>0 &0xFF);
      EXPECT_EQ(ms.size()-5, a.size());
      EXPECT_EQ(ms[5], 0xC0);
      EXPECT_EQ(*--ms.end(), 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,a);
   }
}

TEST(m5gpack,stream_map)
{
   { /// fixmap stores a map whose length is upto 15 elements
      m5g::stream ms;
      m5g::map m(15);
      ms << m;
      EXPECT_EQ(ms[0], 0x80+m.size());
      EXPECT_EQ(ms.size()-1, m.size()*2);
      EXPECT_EQ(ms[1], 0xc0);
      EXPECT_EQ(*--ms.end(), 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,m);
      
      ms = {};
      ms << m5g::map{};
      EXPECT_EQ(ms[0], 0x80);
      EXPECT_EQ(ms.size(), 1);
      // deserialize
      ms >> val;
      EXPECT_EQ(val,m5g::map{});
   }
   { /// map 16 stores a map whose length is upto (2^16)-1 elements
      m5g::stream ms;
      m5g::map m(16);
      ms << m;
      EXPECT_EQ(ms[0], 0xde);
      EXPECT_EQ(ms[1], m.size()>>8);
      EXPECT_EQ(ms[2], m.size()&0xFF);
      EXPECT_EQ(ms.size()-3, m.size()*2);
      EXPECT_EQ(ms[3], 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,m);
   }
   { /// map 32 stores a map whose length is upto (2^32)-1 elements
      m5g::stream ms;
      m5g::map m(70000);
      ms << m;
      EXPECT_EQ(ms[0], 0xdf);
      EXPECT_EQ(ms[1], m.size()>>24&0xFF);
      EXPECT_EQ(ms[2], m.size()>>16&0xFF);
      EXPECT_EQ(ms[3], m.size()>>8 &0xFF);
      EXPECT_EQ(ms[4], m.size()>>0 &0xFF);
      EXPECT_EQ(ms.size()-5, 2*m.size());
      EXPECT_EQ(ms[5], 0xc0);
      EXPECT_EQ(*--ms.end(), 0xc0);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_EQ(val,m);
   }
}

TEST(m5gpack,stream_ext)
{
   { /// fixext 1 stores an integer and a byte array whose length is 1 byte
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin{0x17}};
      ms << e;
      EXPECT_EQ(ms[0], 0xd4);
      EXPECT_EQ(ms[1], 0xdf);
      EXPECT_EQ(*--ms.end(), 0x17);
      EXPECT_EQ(ms.size(), 3);
      // deserialize
      m5g_value val;
      ms >> val;
      // EXPECT_EQ(m5g::value(m5g::ext{}),0);
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(m5g::value(m5g::nil),m5g::value(e));
   }
   { /// fixext 2
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin{0x17,0x18}};
      ms << e;
      EXPECT_EQ(ms[0], 0xd5);
      EXPECT_EQ(ms[1], 0xdf);
      EXPECT_EQ(*----ms.end(), 0x17);
      EXPECT_EQ(*--ms.end(), 0x18);
      EXPECT_EQ(ms.size(), 4);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// fixext 4
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(4)};
      ms << e;
      EXPECT_EQ(ms[0], 0xd6);
      EXPECT_EQ(ms[1], 0xdf);
      EXPECT_EQ(ms.size(), 2+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// fixext 8
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(8)};
      ms << e;
      EXPECT_EQ(ms[0], 0xd7);
      EXPECT_EQ(ms[1], 0xdf);
      EXPECT_EQ(ms.size(), 2+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// fixext 16
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(16)};
      ms << e;
      EXPECT_EQ(ms[0], 0xd8);
      EXPECT_EQ(ms[1], 0xdf);
      EXPECT_EQ(ms.size(), 2+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// ext 8
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(3)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc7);
      EXPECT_EQ(ms[1], e.data.size());
      EXPECT_EQ(ms[2], 0xdf);
      EXPECT_EQ(ms.size(), 3+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
      
      ms={};
      e = {0xde, m5g::bin(12)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc7);
      EXPECT_EQ(ms[1], e.data.size());
      EXPECT_EQ(ms[2], 0xde);
      EXPECT_EQ(ms.size(), 3+e.data.size());
      // deserialize
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
      
      ms={};
      e = {0xdd, m5g::bin(144)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc7);
      EXPECT_EQ(ms[1], e.data.size());
      EXPECT_EQ(ms[2], 0xdd);
      EXPECT_EQ(ms.size(), 3+e.data.size());
      // deserialize
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// ext 16
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(256)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc8);
      EXPECT_EQ(ms[1], e.data.size()>>8);
      EXPECT_EQ(ms[2], e.data.size()&0xFF);
      EXPECT_EQ(ms[3], 0xdf);
      EXPECT_EQ(ms.size(), 4+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
      
      ms={};
      e = {0xde, m5g::bin(1000)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc8);
      EXPECT_EQ(ms[1], e.data.size()>>8);
      EXPECT_EQ(ms[2], e.data.size()&0xFF);
      EXPECT_EQ(ms[3], 0xde);
      EXPECT_EQ(ms.size(), 4+e.data.size());
      // deserialize
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
      
      ms={};
      e = {0xdd, m5g::bin(65535)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc8);
      EXPECT_EQ(ms[1], e.data.size()>>8);
      EXPECT_EQ(ms[2], e.data.size()&0xFF);
      EXPECT_EQ(ms[3], 0xdd);
      EXPECT_EQ(ms.size(), 4+e.data.size());
      // deserialize
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
   { /// ext 32
      m5g::stream ms;
      m5g::ext e {0xdf,m5g::bin(65536)};
      ms << e;
      EXPECT_EQ(ms[0], 0xc9);
      EXPECT_EQ(ms[1], e.data.size()>>24&0xFF);
      EXPECT_EQ(ms[2], e.data.size()>>16&0xFF);
      EXPECT_EQ(ms[3], e.data.size()>>8 &0xFF);
      EXPECT_EQ(ms[4], e.data.size()>>0 &0xFF);
      EXPECT_EQ(ms[5], 0xdf);
      EXPECT_EQ(ms.size(), 6+e.data.size());
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == e); //SIGSEGV EXPECT_EQ(val,e);
   }
}

TEST(m5gpack,stream_time)
{
   using namespace std::chrono;
   using namespace std::chrono_literals;
   /// timestamp 32 stores the number of seconds that have elapsed since 1970-01-01 00:00:00 UTC
   {
      m5g::stream ms;
      m5g::timestamp t = 1605949189s;
      // ASSERT_EQ(duration_cast<seconds>(t).count(),1605949189);
      // ASSERT_EQ(duration_cast<nanoseconds>(t).count(),0);
      ms << t;
      EXPECT_EQ(ms[0], 0xd6);
      EXPECT_EQ(ms[1], (uint8_t)-1);
      EXPECT_EQ(ms[2], 1605949189>>24&0xFF);
      EXPECT_EQ(ms[3], 1605949189>>16&0xFF);
      EXPECT_EQ(ms[4], 1605949189>>8 &0xFF);
      EXPECT_EQ(ms[5], 1605949189>>0 &0xFF);
      EXPECT_EQ(ms.size(), 6);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == t); // EXPECT_EQ(val,t);
   }
   {/// timestamp 64 stores the number of seconds and nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
      m5g::stream ms;
      m5g::timestamp t {1605949189s,123'456'789ns};
      ms << t;
      EXPECT_EQ(ms[0], 0xd7);
      EXPECT_EQ(ms[1], (uint8_t)-1);
      EXPECT_EQ(ms[2], 0x1d);
      EXPECT_EQ(ms[3], 0x6f);
      EXPECT_EQ(ms[4], 0x34);
      EXPECT_EQ(ms[5], 0x54);
      EXPECT_EQ(ms[6], 0x5f);
      EXPECT_EQ(ms[7], 0xb8);
      EXPECT_EQ(ms[8], 0xd7);
      EXPECT_EQ(ms[9], 0x05);
      EXPECT_EQ(ms.size()-2, sizeof(uint64_t));
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == t); // EXPECT_EQ(val,t);
   }
   {/// timestamp 96 stores the number of seconds and nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
      m5g::stream ms;
      m5g::timestamp t{98761605949189s, 123'456'789ns};
      ms << t;
      EXPECT_EQ(ms[0], 0xc7);
      EXPECT_EQ(ms[1], 12);
      EXPECT_EQ(ms[2], (uint8_t)-1);
      EXPECT_EQ(ms[3], 0x07);
      EXPECT_EQ(*--ms.end(), 0x05);
      EXPECT_EQ(ms.size(), 3+12);
      // deserialize
      m5g_value val;
      ms >> val;
      EXPECT_TRUE(val == t); // EXPECT_EQ(val,t);
   }
}
