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
   // 3. move construct
   
   // 4. copy assign
   m5g::value v2 = -4;
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


TEST(m5gpack,byte_stream)
{
   m5g::stream ms;
   ms << m5g::value{};
   cerr<< byte_range_ascii{ms} <<endl;
   
   ms ={};
   ms << m5g::value{m5g::map{
            {{},{}},
            {"a", m5g::arr{}},
            {"help", m5g::array{ 
                  "11d", 33.6, m5g_value::array_t{1,2} 
               }
            },
            {m5g::nil, m5g_value::nil},
            {2,54.3},
            {0.9,21.3},
         }};
   cerr<< byte_range_ascii{ms} <<endl;
}
