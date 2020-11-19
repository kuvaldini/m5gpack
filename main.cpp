#include "m5gpack.hpp"
#include "m5gpack/ring_buffer.hpp"

#include <iostream>
#include <cassert>

using namespace std;
// using namespace m5gpack;


int main()
{
   // union {
   //    int32_t i;
   //    int64_t i64;
   // } u = {0x12345678};
   // assert( u.i == u.i64 );
   
   // cout<<"hello "<<sizeof(string_view)<<" "<<sizeof(int*)<<endl;
   // cout<< u.i << "  " << u.i64 <<endl;
   
   m5g_byte_stream m5s;
   
   // m5value mv;
   
   m5g_value v;
   v = 4;
   cout<<"v:"<<v<<endl;
   v = 32.1;
   cout<<"v:"<<v<<endl;
   m5g_value::array_t arr { 1, 33.6, "345"};
   v = arr;
   cout<<"v:"<<v<<endl;
   
   m5g_value w{v};
   cout<<"w:"<<w<<endl;
   w=std::move(v);
   cout<<"w:"<<w<<endl;
   cout<<"moved v:"<<v<<endl; //v must be nil
   
   auto y{move(w)};
   cout<<"y:"<<y<<endl;
   cout<<"moved w:"<<w<<endl;
   
   v = m5g_value::map_t{ {-9,45.6} }; //, {"help", m5g_value::array_t{1,2,"3"}} };
   cout<<"map v:"<<v<<endl;
   
   v = m5g_value::map_t{ {"a", arr} 
                       , {"help", m5g_value::array_t{ "11d", 33.6, m5g_value::array_t{1,2} }} 
                       , {m5g_value::nil, m5g_value::nil}
                       , {2,54.3}
                       , {0.9,21.3}
                       };
   cout<<"map2 v:"<<v<<endl;
   
   
   // m5gval::map_t mapa;
   // v = mapa;
   
   // m5gval mval;
   // mval = 55;
   // mval = m5gval{m5gval::map_t{
   //    {m5gval("aaa"), m5gval(123)}
   // }};
}