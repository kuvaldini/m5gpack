
#pragma once

#include <limits>


//namespace Sledge::Math
namespace Sledge {
namespace Math {
	
	//todo constexpr, Keil is stupid, and has bad C++11 support.
	
	template< typename T >
	inline /*constexpr*/ T qNaN() { return std::numeric_limits<T>::quiet_NaN(); }
	
	inline /*constexpr*/ float qNaNf() { return qNaN<float>(); }
	
}} //namespace Sledge::Math


namespace slg = Sledge;
