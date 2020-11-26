#include <concepts>
#include <iterator>
#include <compare>
// #include <iterator_concept>


template<typename T>
struct my_iterator
{
   using iterator_category = std::random_access_iterator_tag; //bidirectional_iterator_tag;
   using difference_type = ssize_t;
   using value_type = T;
   // using pointer = T*;
   // using reference = T&;
   
   ///
   /// input_iterator
   ///
   T& operator*()const{
      return data_begin_[index_];
   }
   
   ///
   /// forward_iterator
   ///
   my_iterator& operator++(){
      index_ = ++index_ < capacity_ ?: 0;
      return *this;
   }
   my_iterator operator++(int){
      auto copy = *this;
      ++*this;
      return copy;
   }
   
   ///
   /// bidirectional_iterator
   ///
   my_iterator& operator--(){
      // index_ = index_ > 0 ? index_ - 1 : capacity_ - 1;
      // index_ = index_ == 0 ? capacity_ :;
      --index_;
      index_ = std::min(index_,capacity_-1);
      return *this;
   }
   my_iterator operator--(int){
      auto copy = *this;
      --*this;
      return copy;
   }
   
   /// 
   /// std::random_access_iterator
   ///
   my_iterator& operator+=(difference_type n){
      index_ += n;
      if(index_ >= capacity_)
         index_ -= capacity_;
      return *this;
   }
   my_iterator operator+(difference_type n)const{
      auto copy = *this;
      return copy += n;
   }
   friend my_iterator operator+(difference_type n, my_iterator const& i){
      return i+n;
   }
   T& operator[](difference_type n)const{  //const std::iter_difference_t<I> n){
      return *(*this + n);
   }
   my_iterator& operator-=(difference_type n){
      index_ -= n;
      if(index_ >= capacity_)
         index_ += capacity_;
      return *this;
   }
   my_iterator operator-(difference_type n)const{
      auto copy = *this;
      return copy-=n;
   }
   difference_type operator-(my_iterator const& i) const{
      return this->index_ - i.index_;
   }
   
   ///
   /// equality_comparable
   /// 
   auto operator<=>(const my_iterator&) const = default;
   // bool operator==(const my_iterator& other)const{
   //    return pointer == other.pointer;
   // }
   // bool operator!=(const my_iterator& other)const{
   //    return pointer != other.pointer;
   // }
   
private:
   // T*pointer,*databegin_,*dataend_;
   T*data_begin_;
   size_t index_, capacity_;
};
// template<class T> constexpr bool always_false = false;
// static_assert(always_false<std::iter_reference_t<my_iterator<int>>>, "asd");
// typedef typename std::iter_reference_t<my_iterator<int>>::something_made_up X;
static_assert(std::weakly_incrementable<my_iterator<int>>);
static_assert(std::equality_comparable<my_iterator<int>>);
static_assert(std::input_iterator<my_iterator<int>>);
static_assert(std::forward_iterator<my_iterator<int>>);
static_assert(std::bidirectional_iterator<my_iterator<int>>);
static_assert(std::random_access_iterator<my_iterator<int>>);


template<typename T, typename A=std::allocator<T>>
struct ring_buffer {
   using container = ring_buffer<T,A>;
   using value_type = T;
   using allocator_type = A;
   using size_type = std::size_t;
   using reference = T&;
   using const_reference = const T&;
   using pointer = std::allocator_traits<A>::pointer;
   using const_pointer = std::allocator_traits<A>::const_pointer;
   using iterator = my_iterator<T>;
   using const_iterator = my_iterator<const T>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   static_assert(std::is_convertible_v<iterator,const_iterator>);
   using difference_type = iterator::difference_type;
   
   constexpr ring_buffer() = default;
   constexpr ring_buffer(ring_buffer const&) = default;
   constexpr ring_buffer(ring_buffer &&) = default;
   constexpr ring_buffer& operator=(ring_buffer const&);
   constexpr ring_buffer& operator=(ring_buffer &&);
   constexpr ~ring_buffer();
   
   /// Alias to operator=
   template<typename...Ts>
   constexpr void assign(Ts&&...vs){ *this = std::forward<Ts...>(vs...); }
   
   constexpr allocator_type get_allocator() const noexcept{ return allocator_; }
   
   ///
   ///  Element access 
   ///
   // constexpr reference at( size_type pos );
   // constexpr const_reference at( size_type pos ) const;
   // constexpr reference operator[]( size_type pos );
   // constexpr const_reference operator[]( size_type pos ) const;
   // constexpr reference front();
   // constexpr const_reference front() const;
   // constexpr reference back();
   // constexpr const_reference back() const;
   constexpr T* data() noexcept;
   constexpr const T* data() const noexcept;
   
   constexpr reference at(size_type pos){
      if(pos < size())
         return *this[pos];
      else
         throw std::out_of_range();
   }
   constexpr reference operator[](size_type pos){
      return *(begin() + pos);
   }
   constexpr const_reference at(size_type pos)const{
      if(pos < size())
         return *this[pos];
      else
         throw std::out_of_range();
   }
   constexpr const_reference operator[](size_type pos)const{
      return *(begin() + pos);
   }
   constexpr       reference front()      { return *this[0]; }
   constexpr const_reference front() const{ return *this[0]; }
   constexpr       reference back()       { return *(this->end()-1); }
   constexpr const_reference back() const { return *(this->end()-1); }
   // constexpr T* data() noexcept;
   // constexpr const T* data() const noexcept;
   
   ///
   /// Iterators 
   ///
   constexpr       iterator  begin()       noexcept;
   constexpr const_iterator  begin() const noexcept;
   constexpr const_iterator cbegin() const noexcept { const_cast<const ring_buffer<T>&>(*this).begin(); }
   constexpr       iterator  end()       noexcept;
   constexpr const_iterator  end() const noexcept;
   constexpr const_iterator cend() const noexcept   { const_cast<const ring_buffer<T>&>(*this).end(); }
   constexpr       reverse_iterator  rbegin()       noexcept;
   constexpr const_reverse_iterator  rbegin() const noexcept;
   constexpr const_reverse_iterator crbegin() const noexcept;
   constexpr       reverse_iterator  rend()       noexcept;
   constexpr const_reverse_iterator  rend() const noexcept;
   constexpr const_reverse_iterator crend() const noexcept;
   
   ///
   /// Capacity
   ///
   [[nodiscard]] constexpr bool empty() const noexcept {
      return size_ == 0; //begin() == end();
   }
   constexpr size_type size() const noexcept{
      return size_;
   }
   constexpr size_type max_size() const noexcept{
      return std::numeric_limits<size_type>::max();
   }
   constexpr void reserve( size_type new_cap ){
      if(new_cap > capacity()){
         newdata = allocator_.allocate(new_cap);
         std::move(begin(),end(),newdata());
         ///note: see https://stackoverflow.com/questions/65022554/why-is-the-copy-ctor-preferred-over-move-ctor-when-stdvector-relocates-storage
      }
   }
   constexpr size_type capacity() const noexcept;
   constexpr void shrink_to_fit();
   
   ///
   /// Modifiers
   ///
   constexpr void clear() noexcept;
   constexpr iterator insert( const_iterator pos, const T& value );
   constexpr iterator insert( const_iterator pos, T&& value );
   constexpr iterator insert( const_iterator pos, size_type count, const T& value );
   template< class InputIt >
   constexpr iterator insert( const_iterator pos, InputIt first, InputIt last );
   constexpr iterator insert( const_iterator pos, std::initializer_list<T> ilist );
   
   template< class... Args >
   constexpr iterator emplace( const_iterator pos, Args&&... args );
   
   constexpr iterator erase( const_iterator pos );
   constexpr iterator erase( const_iterator first, const_iterator last );
   
   constexpr void push_back( const T& value ){
      if(size()==capacity()){
         reserve(capacity*2);
      }
   }
   constexpr void push_back( T&& value );
   
   template< class... Args >
   constexpr reference emplace_back( Args&&... args );
   
   constexpr void pop_back();
   
   constexpr void resize( size_type count );
   constexpr void resize( size_type count, const value_type& value );
   
   constexpr void swap( container& other ) noexcept/*( see below )*/;
   
   
   /// EqualityComparable
   bool operator==(ring_buffer const& other)const{
      // does it check ranges sizes are equal
      return std::equal(this->begin(), this->end(), other.begin(), other.end());
   }
   bool operator!=(ring_buffer const& other)const{
      return !(*this == other);
   }
   
   // size_type size()const{
   //    return std::distance(this->begin(), this->end());
   // }
   // size_type max_size()const{
   //    return std::numeric_limits<size_type>::max();
   // }
   
private:
   T* data_;
   A allocator_;
};
// static_assert(std::container<ring_buffer<int>>);

#include<vector>
#include<iostream>
using namespace std;

struct A{
   A(const A& a)noexcept{ i=a.i; cerr<<"copied "<<i<<endl; }
   A(     A&& a)noexcept{ i=a.i; cerr<<"moved "<<i<<endl; }
   A(int i)noexcept:i(i){cerr<<"created "<<i<<endl;};
   A()noexcept{};
private:
   int i=0;
};
static_assert(not std::is_trivial_v<A>);
int main()
{
   vector<A> v1;
   size_t prevcap=v1.capacity();
   for(int i=0; i<10; ++i){
      v1.emplace_back(i);
      if(prevcap!=v1.capacity()){
         cerr<<"capacity increased to "<<v1.capacity()<<endl;
         prevcap=v1.capacity();
      }
      cerr<<"------"<<endl;
   }
}
