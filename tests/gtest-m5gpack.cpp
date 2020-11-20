#include <sstream>
#include "gtest/gtest.h"
#include "m5gpack.hpp"

using namespace std;
using namespace std::literals;
// using namespace m5g;

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
   
   
   
   // m5g_value;
   // m5g_array;
   // m5g_map;
   // m5g_string;
   // m5g_bin;
   // m5g_blob;
   
   // m5g::value;
   // m5g::array;
   // m5g::map;
   // m5g::string;
}

/// nil < boolean < (signed int, unsigned int, float) < str < bin < arr < map < ext < time
TEST(m5gpack,compare_less)
{
   EXPECT_LT(m5g::value{} , false);
   EXPECT_LT(m5g::value{false} , true);
   EXPECT_LT(m5g::value{true} , int(-1));
   EXPECT_LT(m5g::value{int(-1)} , 2);
   EXPECT_LT(m5g::value{2} , double(3.14));
   EXPECT_LT(m5g::value{3.14} , "hello"s);
   EXPECT_LT(m5g::value{"hello"} , m5g::value{"helly"});
   EXPECT_LT(m5g::value{"hello"} , (m5g::array{1,"222"}));
   EXPECT_LT((m5g::value{m5g::array{1,2,3}}) , (m5g::array{1,2,4}));
   EXPECT_LT((m5g::value{m5g::array{1,2,3}}) , (m5g::array{1,2,3,1}));
   EXPECT_LT(m5g::value{m5g::array{}} , m5g::map{});
   EXPECT_LT((m5g::value{m5g::map{{1,"a"}}}) , (m5g::map{{2,"a"}}));
   EXPECT_LT((m5g::value{m5g::map{{1,"a"}}}) , (m5g::map{{1,"a"},{}}));
   
   using namespace m5gpack;
   EXPECT_LT(m5gval{} , false);
   EXPECT_LT(m5gval{false}, m5gval{3});
   EXPECT_LT(m5g::value{-2} , -1);
   EXPECT_LT(m5g::value{} , 2);
   
   // array
   // EXPECT_EQ(m5garr{1,2,3} , m5garr{1,2,3});
   // EXPECT_EQ(m5garr{1,2} , m5garr{1,2,3});
   // EXPECT_LT(m5garr{1,2,3} , m5garr{1,2,4});
   // EXPECT_LT(m5gval{} , false);
}

TEST(m5gpack,compare_equal)
{
   EXPECT_EQ(m5g::value{true} , true);
   EXPECT_EQ(m5g::value{int(2)} , 2.);
   EXPECT_EQ(m5g::value{int(2)} , 2ul);
}


TEST(m5gpack,stream_common)
{
   // empty types
   {
      m5g::stream ms;
      ms << m5g::value{m5g::map{
               {},
               {"", m5g::arr{}},
      }};
      byte_vector expected {0x82,0xC0,0xC0,0xA0,0x90};
      EXPECT_EQ(ms,expected);
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
TEST(m5gpack,stream_int)
{
   // nil
   m5g::stream ms;
   ms << m5g::value(m5g::nil);
   EXPECT_EQ(ms,(byte_vector{0xc0}));
   
   // bool
   ms = {};
   ms << false << true;
   EXPECT_EQ(ms,(byte_vector{0xc2,0xc3}));
   
   // uint
   ms = {};
   ms << 0u << 127u << 255u << unsigned(65535) << uint64_t(0x1234ABCD) << UINT64_MAX;
   EXPECT_EQ(ms,(byte_vector{0x00,0x7F,0xcc,0xFF,0xcd,0xFF,0xFF,0xce,0x12,0x34,0xAB,0xCD,
                              0xcf,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}));
   
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
   
   // fixstr
   ms = {};
   ms << "" << "Hello";
   EXPECT_EQ(ms,(m5g::stream{0xa0, 0xa5,'H','e','l','l','o'}));
}
TEST(m5gpack,stream_str)
{
   { // fixstr max size
      m5g::stream ms;
      ms << "1234567890qwertyuiopasdfghjklzx";
      EXPECT_EQ(ms[0], 0xBF);
      EXPECT_EQ(ms[1], '1');
      EXPECT_EQ(*--ms.end(), 'x');
   }
   { // str 8
      m5g::stream ms;
      ms << "1234567890qwertyuiopasdfghjklzxc";
      EXPECT_EQ(ms[0], 0xD9);
      EXPECT_EQ(ms[1], 32);
      EXPECT_EQ(ms[2], '1');
      EXPECT_EQ(*--ms.end(), 'c');
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
   }
}