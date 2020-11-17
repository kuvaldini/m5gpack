#pragma once


#include <memory>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>


// namespace m5g {

	using std::size_t;
	
	
	/**	
	 * Шаблон кольцевого буфера.
	 * @param T         : тип элементов хранящихся в буфере, по умолчанию unsigned char
	 * @param Allocator : 
	 * @param IndexT_   : используйте volatile, чтобы обеспечить лайтовую потоко-безопасноть
	 */
	template< typename T, typename Allocator = std::allocator<T> >
	class ring_buffer //: private Allocator
	{
	public:  // == TYPES as `std::vector` ==
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = value_type const&;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
		//using iterator = LegacyRandomAccessIterator;
		//using const_iterator = Constant LegacyRandomAccessIterator;
		//using reverse_iterator = std::reverse_iterator<iterator>;
		//using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	
		using ThisT = ring_buffer< T, Allocator >;
		
		using SizeT = size_type;
		using Data_t = T;
		
		/// Определяем псевдоним для индексов. int позволит использовать отрицательные индексы
		using IndexT = difference_type;
	
	public:  // == STATIC CONSTANTS ==
		static constexpr SizeT DEFAULT_CAPACITY = 16;
		
	public:  // == CONSTRUCTORS and DESTRUCTOR
		
		ring_buffer(SizeT capacity, IndexT brn/*=0*/) 
			: capacity_(capacity), bourn_(brn)//(brn!=0?brn:size)
		{
			_data = allocator_.allocate(capacity_); 
		}
		
		ring_buffer(SizeT capacity = DEFAULT_CAPACITY) 
			: capacity_(capacity)//, bourn_(size)
		{
			_data = allocator_.allocate(capacity_); 
		}
		
		// Для наследования. Разрешает полиморфизм Base *b = new Derived(); delete b;
		virtual ~ring_buffer(){
			allocator_.deallocate(_data, capacity_);
		}

	private:  // == VARIABLES ==
		SizeT            capacity_ = DEFAULT_CAPACITY;
		Allocator        allocator_;
		Data_t          *_data = nullptr;
		
		IndexT head_     = 0; 	/// количество чтений, индекс головы
		IndexT tail_     = 0; 	/// количество записей, индекс хвоста, индекс куда будет записан следующий элемент
		IndexT length_   = 0;	/// количество элементов
		IndexT bourn_    = capacity_; 	/// предельное наполнение, позволяет задать предел наполнения буфера
		//TODO bool autoReallocate_ = false;  /// Самостоятельно выделять новый кусок памяти при переполнении буфера (недостаточно емкости), копируя в него весь буффер при переполнеии.	
		
	private:  // == УПРАВЛЕНИЕ ИНДЕКСАМИ ==  //todo remove and use RingIndex
		/// Cледующий индекс, изменяет значение входного аргумента.
		/// ++idx
		IndexT& nextIndex( IndexT & idx) // volatile & было.
		{
			return idx = ++idx < bourn_ ? idx : 0;
		}
		
		/// Увеличение индекса на inc, изменяет значение входного параметра.
		/// idx += inc
		IndexT& incIndex( IndexT &idx, IndexT inc ) const  // volatile & было.
		{
			idx += inc;
			if( idx >= bourn_ )
				idx -= bourn_;
			return idx;
		}
		
		/// Cледующий индекс, НЕ изменяет значение входного параметра.
		/// idx + 1
		IndexT nextIndexOf( IndexT const& idx) const 
		{
			return (idx+1) < bourn_ ? (idx+1) : 0;
		}
		
		/// Инкремент idx на inc, НЕ изменяет значение входного параметра.
		/// idx + inc
		IndexT incIndexOf( IndexT const& idx, IndexT const& inc ) const 
		{
			IndexT ret = idx + inc;
			if( ret >= bourn_ )
				ret -= bourn_;
			return ret;
		}
		
	public:  // == Data storage METHODS ==
		
		/*TODO /// Like std::vector::resize() <http://en.cppreference.com/w/cpp/container/vector/resize>
		void resize(std::size_t siz)
		{
			if( capacity < siz ){
				auto newdata = allocator_.allocate(siz);
				std::memcpy( newdata, _data, size*sizeof(T) )
			}
		};
	
		void shrink_to_data()
		*/
		
		/*ToDo
		SizeT capacity();
		*/
	
	protected:
		void push_noCheck( Data_t const& value )
		{
			_data[ tail_ ] = value;
			nextIndex(tail_);  // ++tail_;
			++length_;
		}
		
	public:  // == METHODS ==
		/** Запись в буфер одного элемента
		  * \return true если значение записано, false при переполнении
		  */
		bool push(Data_t const& value) noexcept 
		{
			if( isFull() )	
				return false;
			else{
				push_noCheck(value);
				return true;
			}
		}
		
		/** Запись в буфер массива элементов. Будет записано не более чем свободно.
		  * \return Количество вставленных элементов
		  */
		IndexT push( const Data_t array[], IndexT length )
		{
			IndexT i=0;
			while( i<length && push(array[i]) )
				i++;
			return i;
		}
	
	#define BE_FASTER_OPTIMIZE  true
	#if BE_FASTER_OPTIMIZE == 0
		/** Запись в буфер, выталкаивая первый элемент, если буффер заполнен.
		  * Цивильный вариант с небольшим оверхэдом. Опирается на готовые функции.
		  * \return true, если буффер полон и 1ый элемент был вытолкнут; false - если не полон.
		  */
		bool pushForce( Data_t const& value) 
		{
			bool isfull = isFull();
			if( isfull )	
				pop();
			push(value);
			return isfull;
		}
	#else
		/**	Запись в буфер, выталкаивая первый, если буффер заполнен.
		  * Облегчённый и ускоренный вариант
		  * \return true, если буффер полон и 1ый элемент был вытолкнут; false - если не полон.
		  */
		bool pushForce( Data_t const& value )
		{
			if( isFull() ){
				_data[head_] = value; 
				nextIndex(head_);  //todo ++head_, ++tail;
				nextIndex(tail_);
				//assert( tail_ = prevHead );
				return true;
			}else{
				push_noCheck(value);
				return false;
			}
		}
	#endif
		
		/*bool pushForce( Data_t && value) {
			bool isful = isFull();
			if( isful )	
				pop();
			push( std::move(value) );
			return isful;
		}*/
			
		/** Запись в буфер массива элементов, выталкивая все предыдущие если буффер заполнен.
		  * \return number of elements pushed
		  */
		IndexT pushForce( Data_t const array[], IndexT length )
		{
			IndexT i=0;
			while( i<length ){
				pushForce( array[i++] );
			}
			return i;
		}
		
		/** Запись в буфер массива элементов, выталкивая все предыдущие если буффер заполнен.
		  * \return number of elements pushed
		  */
		IndexT pushForce( ring_buffer<Data_t> inRB, IndexT nToPush = -1 )
		{
			if( nToPush == -1 || nToPush > inRB.length() )
				nToPush = inRB.length();
			IndexT i=0;
			while( i<nToPush ){
				pushForce( inRB[i++] );
			}
			return i;
		}
		
		/** Достать элемент, удаляя из буфера, и записать в value
		  * \return Возвращает true если значение прочитано (буфер не пуст), value изменяется.
		  */
		bool pop(Data_t &value) {
			if( isEmpty() ){
				//value = 0; //Data_t();
				return false;
			}
			value = _data[head_];
			nextIndex(head_);
			length_--;
			return true;
		}
		
		/** Достать и удалить из буфера
		 *  Не рекомендуется для больших данных.
		 *  \return возвращает элемент или новый элемент.
		 */
		Data_t pop() {
			if( isEmpty() )
				return Data_t();  // throw "Nothing to pop"
			Data_t value = _data[head_];
			nextIndex(head_);
			length_--;
			return value;
		}
		
		/** Достать элементы, удаляя из буфера, и записать в массив
		  * \return Возвращает количество выплюнутых/записанных элементов
		  */
		IndexT pop(Data_t array[], size_t length) {
			if( isEmpty() ){
				return 0;
			}
			size_t i=0;
			while( i<length && pop(array[i++]) );
			return i;
		}
		
		/// Псевдоним для вариации pop()
		IndexT popToArray(Data_t array[], size_t length) {
			return pop(array,length);
		}
		
		///\todo
		//IndexT popToring_buffer(RB &rb, len)
		
		///\todo оптимизировать. молча выкинуть лишнее
		/** Достать и удалить из буфера len элементов
		 *  \param len количество элементов
		 *  \return возвращает количество вытолкнутых элементов; 0, если выталкивать нечего (пустой).
		 */
		IndexT pop(size_t len) {
			/*IndexT i=0;
			Data_t dummy;
			while( len>0 && pop(dummy) ){ 
				len--; i++;
			};
			return i;*/
			if( len >= length() ){
				len = length();
				clear();
				return len;
			}else {
				head_ += len;
				length_ -= len;
			}
		}
		
		/** Вытолкнуть элементы из буфера, и записать в ring_buffer rb
		  * \param rb куда записывать
		  * \return Возвращает количество вытолкнутых/вставленных комнонентов.
		  */
		SizeT pop( ring_buffer<Data_t> &rb ){
			SizeT s=0;
			while( !isEmpty() && rb.push(pop()) ) 
				s++;
			return s;
		}
		
		/// возвращает первый элемент из буфера, не удаляя его
		Data_t& getFirst() {
			return operator[](0);
		}
		
		/// возвращает первый элемент из буфера, не удаляя его
		const Data_t& getFirst() const {
			return operator[](0);
		}
		
		/// возвращает последний элемент из буфера, не удаляя его
		Data_t& getLast() {
			return operator[](length()-1);
		}
		
		/// возвращает последний элемент из буфера, не удаляя его
		const Data_t& getLast() const {
			return operator[](length()-1);
		}
		
		/// эквивалент getLast()
		Data_t& peek() {
			return getFirst();
		}
		
		/// эквивалент getLast()
		const Data_t& peek() const {
			return getFirst();
		}
		
		/// 
		//IndexT copyTo( Data_t &d ) /*const*/ {
		//	abort_msg(__func__##" not implemented yet.\n");
		//	return 0;
		//}
		
		/// Copy elements `len` elements from buffer to `array`.
		///\param offset  the start index of ring buffer
		IndexT copyToArray( Data_t array[], IndexT len, IndexT offset=0 ) /*const*/ {
			int i=0, j; if( -length()<=offset && offset<length() )  j=offset; else j=0;
			while( i<len && j<length() ){
				array[i] = operator[](j);
				i++; j++;
			}
			return i;
		}
		
		/// Copy elements last `len` elements from buffer to `array`.
		IndexT copyLastToArray( Data_t array[], IndexT len ) /*const*/ {
			IndexT i=0, j = length() > len ? length()-len : 0;
			while( i<len && j<length() ){
				array[i] = operator[](j);
				i++; j++;
			}
			return i;
		}
		
		/// Обрезать буфер.
		IndexT trim( IndexT fromBegin/*, IndexT fromEnd=0*/ ){
			if( fromBegin > length() )
				fromBegin = length();
			incIndex(head_, fromBegin);
			length_ -= fromBegin;
			return length_;
		}
		
		/** Возвращает элемент (ссылку) по индексу
		 *  Отрицательный индекс означает отсчёт с хвоста, [-1] это последний, [-2] предпоследний */ 
		Data_t& operator[](IndexT i)
		{
			if( i<0 )
				i += length();
			return _data[ incIndexOf(head_, i) ];
		}
		
		/** Возвращает элемент (ссылку) по индексу
		 *  Отрицательный индекс означает отсчёт с хвоста, [-1] это последний, [-2] предпоследний
		 *  НЕТ проверки индекса
		 */ 
		const Data_t& operator[](IndexT i) const
		{
			if( i<0 )
				i += length();
			return _data[ incIndexOf(head_, i) ];
		}
		
		/// Пуст ли буфер
		bool isEmpty() const 
		{
			return length_ == 0; //return tail_ == head_;
		}
		
		/// Полон ли буфер
		bool isFull() const 
		{
			return length_ == bourn_; //return length_ == SIZE; //return nextIndexOf(tail_) == head_;
		}
		
		/// Количество элементов в буфере
		IndexT length() const {
			return length_; //return (tail_ >= head_) ? (tail_ - head_) : (SIZE+1 - head_ + tail_);
		}
		
		/// @return the number of elements that can be held in currently allocated storage
		size_t capacity() const {
			return capacity_;
		}
		
		/// Получить предельный размер для записи. Удобно при динамическом изменении длины кольцевого буфера.
		/// Например, чтобы укоротить/удлиннить фильтр во время выполнения.
		size_t bourn() const {
			return bourn_;
		}
		
		/// Установить предельный размер для записи. Удобно при динамическом изменении длины кольцевого буфера.
		/// Не может привышать SIZE.
		/// При уменьшении, Буфер выплюнет в пустоту лишнее (более старое).
		/// \return установленное значение
		size_t bourn(size_t b) 
		{
			if( b <= size() )  bourn_ = b;
			else bourn_ = size();
			// выбросить лишние
			if( length() > bourn_ ) 
				trim( length() - bourn_ );  //pop( length() - bourn_ );
			return bourn_;
		}
		
		bool isAligned() const {
			return head_ == 0;
		}
		
		/*/// Перенос данных, таким образом чтобы голова head_ указывала в на начало массива данных (head_=0)
		//TODO unit test
		ThisT& align() { 
			if( isFull() )
				; //use additional storage стакан
			else
			return *this; 
		}*/
	
	public:  // == `std::vector` COMPATIBLE METHODS. Element access  ==
		
		/// Access specified element with bounds checking
		Data_t& at( IndexT pos )
		{
			//assert( !isEmpty() );
			//assert( (-length()) <= i && i < length() );
			if( isEmpty() || !(-length() <= pos && pos < length() ) )
				throw std::out_of_range("Index out of range");
			return operator[](pos);
		}
		
		/// Access specified element with bounds checking
		const Data_t& at( IndexT pos ) const
		{
			if( pos < size() )
				throw std::out_of_range("Index out of range");
			return operator[](pos);
		}
		
		/* implemented///  access specified element
		operator[]*/
		
		/// access the first element
		T& front(){ return this->operator[](0); }
		const T& front() const { return this->operator[](0); }
		
		/// access the last element
		T& back(){ return this->operator[](-1); }
		const T& back() const { return this->operator[](-1); }
		
		/// direct access to the underlying array
		T* data(){ return _data; }
		const T* data() const { return _data; }
	
	public:  // TODO == `std::vector` COMPATIBLE METHODS. ITERATORS == 
		
		/*///@returns an iterator to the beginning
		begin cbegin
		
		///@returns an iterator to the end
		end cend
		
		///@returns a reverse iterator to the beginning 
		rbegin crbegin
				
		///@returns a reverse iterator to the end
		rend crend*/
	
	public:  // == `std::vector` COMPATIBLE METHODS. CAPACITY == 
		
		/// Checks whether the container is empty
		bool  empty() const { return isEmpty(); };
		
		/// @return the number of elements
		SizeT size()  const { return length(); };
		
		//ToDo///@return the maximum possible number of elements
		//ToDo SizeT max_size() const {return bourn_max;};
		
		//ToDo /// Reserves storage
		//ToDo reserve()
		
		//ToDo /// @return the number of elements that can be held in currently allocated storage
		//ToDo capacity()
		
		//ToDo /// reduces memory usage by freeing unused memory
		//ToDo shrink_to_fit()
	
	public:  // == `std::vector` COMPATIBLE METHODS. MODIFIERS == 
		
		/// Clears the contents
		/// Очистить буфер
		ThisT& clear()
		{
			head_ = tail_ = length_ = 0;
			return *this;
		}
		// std::vector  return void 
		
		/*// inserts elements
		insert 
				
		/// constructs element in-place
		emplace
				
		/// erases elements
		erase*/
		
		/// adds an element to the end
		void push_back( const T& value ){ push(value); };
		//TODO void push_back( T&& value );
				
		/*/// constructs an element in-place at the end
		emplace_back
		
		/// removes the last element
		pop_back
				
		/// changes the number of elements stored
		resize
		
		/// swaps the contents
		void swap( vector& other )*/
	
	public:  // TODO == COMPARISON OPERATORS == 
		bool operator==( ThisT const& other ) const {
			if( this == &other )
				return true;
			else if( this->length() == other.length() ){
				//todo check all contents
				return false;
			}else{
				return false;
			}
				
		}
		/*bool operator!=( ThisT const& other ) const {}
		bool operator< ( ThisT const& other ) const {}
		bool operator<=( ThisT const& other ) const {}
		bool operator> ( ThisT const& other ) const {}
		bool operator>=( ThisT const& other ) const {}*/
	};
	

// }  // namespace m5g
