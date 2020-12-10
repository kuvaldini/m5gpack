#pragma once

namespace m5g {
   
   template<class T, class Allocator = std::allocator<T>>
   class circular {
   public:
      // types
      using value_type             = T;
      using allocator_type         = Allocator;
      using pointer                = typename allocator_traits<Allocator>::pointer;
      using const_pointer          = typename allocator_traits<Allocator>::const_pointer;
      using reference              = value_type&;
      using const_reference        = const value_type&;
      using size_type              = implementation-defined; // see [container.requirements]
      using difference_type        = implementation-defined; // see [container.requirements]
      using iterator               = implementation-defined; // see [container.requirements]
      using const_iterator         = implementation-defined; // see [container.requirements]
      using reverse_iterator       = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;

      // [deque.cons], construct/copy/destroy
      deque() : deque(Allocator()) { }
      explicit deque(const Allocator&);
      explicit deque(size_type n, const Allocator& = Allocator());
      deque(size_type n, const T& value, const Allocator& = Allocator());
      template<class InputIterator>
         deque(InputIterator first, InputIterator last, const Allocator& = Allocator());
      deque(const deque& x);
      deque(deque&&);
      deque(const deque&, const Allocator&);
      deque(deque&&, const Allocator&);
      deque(initializer_list<T>, const Allocator& = Allocator());

      ~deque();
      deque& operator=(const deque& x);
      deque& operator=(deque&& x)
         noexcept(allocator_traits<Allocator>::is_always_equal::value);
      deque& operator=(initializer_list<T>);
      template<class InputIterator>
         void assign(InputIterator first, InputIterator last);
      void assign(size_type n, const T& t);
      void assign(initializer_list<T>);
      allocator_type get_allocator() const noexcept;

      // iterators
                    iterator begin() noexcept;
              const_iterator begin() const noexcept;
                    iterator end()   noexcept;
              const_iterator end()   const noexcept;
            reverse_iterator rbegin() noexcept;
      const_reverse_iterator rbegin() const noexcept;
            reverse_iterator rend() noexcept;
      const_reverse_iterator rend() const noexcept;

      const_iterator         cbegin() const noexcept;
      const_iterator         cend() const noexcept;
      const_reverse_iterator crbegin() const noexcept;
      const_reverse_iterator crend() const noexcept;

      // [deque.capacity], capacity
      [[nodiscard]] bool empty() const noexcept;
      size_type size() const noexcept;
      size_type max_size() const noexcept;
      void      resize(size_type sz);
      void      resize(size_type sz, const T& c);
      void      shrink_to_fit();

      // element access
      reference       operator[](size_type n);
      const_reference operator[](size_type n) const;
      reference       at(size_type n);
      const_reference at(size_type n) const;
      reference       front();
      const_reference front() const;
      reference       back();
      const_reference back() const;

      // [deque.modifiers], modifiers
      template<class... Args> reference emplace_front(Args&&... args);
      template<class... Args> reference emplace_back(Args&&... args);
      template<class... Args> iterator emplace(const_iterator position, Args&&... args);

      void push_front(const T& x);
      void push_front(T&& x);
      void push_back(const T& x);
      void push_back(T&& x);

      iterator insert(const_iterator position, const T& x);
      iterator insert(const_iterator position, T&& x);
      iterator insert(const_iterator position, size_type n, const T& x);
      template<class InputIterator>
         iterator insert(const_iterator position, InputIterator first, InputIterator last);
      iterator insert(const_iterator position, initializer_list<T>);

      void pop_front();
      void pop_back();

      iterator erase(const_iterator position);
      iterator erase(const_iterator first, const_iterator last);
      void     swap(deque&)
         noexcept(allocator_traits<Allocator>::is_always_equal::value);
      void     clear() noexcept;
   };//class
   
   //deduct 
   template<class InputIterator, class Allocator = allocator<iter-value-type<InputIterator>>>
      deque(InputIterator, InputIterator, Allocator = Allocator())
         -> deque<iter-value-type<InputIterator>, Allocator>;

   // swap
   template<class T, class Allocator>
      void swap(deque<T, Allocator>& x, deque<T, Allocator>& y)
         noexcept(noexcept(x.swap(y)));
   
}//namespace
