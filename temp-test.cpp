#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <iomanip>
#include <string>
#include <variant>


using namespace std;

struct Foo{
   string s;
   int i;
   // auto operator<=>(Foo const&) const = default;
   
   // struct hash{
   //    size_t operator()(Foo const& f){
   //       return hash(f.s) + hash(f.i);
   //    }
   // };
   // unordered_set<Foo,Foo::hash> x;
   
   bool operator<(Foo const& f) const{
      if(s < f.s)
         return true;
      else if(f.s < s)
         return false;
      else
         return i < f.i;
   }
};

int main()
{
   variant<int64_t,uint64_t,double> v{1ul};
}
