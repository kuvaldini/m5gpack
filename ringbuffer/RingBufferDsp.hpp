/**
 * \author 	Ivan "Kyb Sledgehammer" Kuvaldin <i.kyb[2]ya,ru>
 * \brief   DSP functions for RingBuffer
 */
 
#pragma once


#include "RingBuffer.hpp"
//#include "Sledge/utils.h"
//#include "Sledge/assert.h"

//#include <memory>
#include <cmath>
//#include <cstddef>
//#include <limits>
//#include "arm_math.h"
#include "./Math.hpp"


namespace Sledge { //namespace DSP {
	
	using std::size_t;
	//using Index_t = int;
	//using Math::qNaN;

	
	template< typename T >
	struct Analyzed 
	{
		using Data_t = T;
		struct Entry { 
			size_t index; 
			Data_t value; 
		};
		Entry min;
		Entry max;
		Data_t peakToPeak;
		Data_t avg;
		Data_t rms;
		Data_t middle;
	};
	
	
	//TODO use iterator
	///\note Be careful with overflow
	template< typename Data_t >
	Data_t average( RingBuffer<Data_t> const& rb )
	{
		Data_t avg = 0;
		const size_t len = rb.length();
		for( size_t i=0; i<len; i++ )
			avg += rb[i];
		avg /= len;
		return avg;
	}
	
	
	/// Iterative mean. http://www.heikohoffmann.de/htmlthesis/node134.html
	/// cons: Less efficient when recalculating much data
	/// plus: disallow overflow
	/// plus: more efficient when calculating mean over many data but admission is not often
	template< typename Data_t >
	Data_t mean( RingBuffer<Data_t> const& rb )
	{
		Data_t avg = 0;
		const size_t len = rb.length();
		for( size_t i=0; i<len; i++) {
			avg += (rb[i] - avg) / (i + 1);
		}
		return avg;
	}

	
	/*
	 * 
	 *
	Data_t filterOneValueKalman( Index_t numK, const Data_t koefs[] ) //const
	{
		//assert_amsg( numK < length() );
		//Data_t koefs[numK] = { 0.4, 0.3, 0.2, 0.1 };  //сумма должна быть равна 1.0
		Data_t filtered = 0;
		for( Index_t i=length()-1, j=0; 
				i>=0 && j<numK;
				i--, j++ ){
			filtered += operator[](i) * koefs[j];  //koefs[i-length()+1]
		}
		return filtered;
	}*/
	
	
	/// 
	template< typename Data_t >
	Data_t rms( RingBuffer<Data_t> const& rb )
	{
		Data_t r = 0;
		const size_t len = rb.length();
		for( size_t i=0; i<len; ++i ){
			r += rb[i] * rb[i];
		}
		//r = __sqrtf( r / length() );  //should expand as the native VFP square root instructions. not specified in any standard
		r = std::sqrt( r / len );
		return r;
	}
	
	
	/// Требует, чтобы данные начинались с начала внутреннего массива данных => head_ == 0
	/// Или же чтобы весь буфер, весь нижележащий массив, был заполнен под завязку.
	/// ToDo Ещё один рабочий вариант это когда кольцевой буфер не закручивается а остаётся прямым.
	///\note Зависимость <arm_math.h>
	template< typename Data_t >
	Data_t arm_rms( RingBuffer<Data_t> /*const*/& rb )
	{
		if( !rb.isAligned() || rb.length() != rb.capacity()  )
			return Math::qNaN<Data_t>();
		Data_t ret;
		arm_rms_f32( rb.data(), rb.length(), &ret );  // тупо, но эти дибилы берут не const на вход.
		return ret;
	}
	
	/**
	 *
	 */
	template< typename Data_t >
	Data_t peakToPeak( RingBuffer<Data_t> const& rb )
	{
		size_t len = rb.length();
		float val = rb[0];
		float max = val, min = val;  //float max = -INFINITY, min = +INFINITY;
		for( int i=1; i<len; i++ ){
			val = rb[i];
			if( max < val )
				max = val;
			if( min > val )
				min = val;
		}
		return max-min;
	}
	
	/*/// окресность
	bool okrestnost( const Data_t &val, const Data_t &main, const Data_t &okr ){
		return (main-okr < val && val < main+okr);
	}*/
	
	/*/// отсев выбросов больше
	///\todo возникнет ошибка с j, когда length <=1
	///\note рассчитано на небольшое количество выбросов.
	void filter_otsev( Data_t main, Data_t okr )
	{
		for( int i=0; i<length(); i++ ){
			Data_t &val = (*this)[i];
			if( !okrestnost(val,main,okr) ){
				Index_t j = i>=1 ? i-1 : i+1;
				while( j<length() && !okrestnost((*this)[j],main,okr) ){
					j++;
				}
				val = (*this)[j];
			}
		}
	}*/
	
	
	/// Полный анализ, чтоб не прогонять массив по нескольку раз. Эффективнее, если нужно получить несколько параметров.
	///\param  an : where to store data
	///\param  rb : input buffer
	///\return an
	template< typename Data_t >
	Analyzed<Data_t>& analyze( Analyzed<Data_t> &an, RingBuffer<Data_t> const& rb )
	{
		if( rb.isEmpty() ){
			//an = {0};  //todo fill with NaN
			return an; 
		}
		size_t len = rb.length();
		an.min.index = an.max.index = 0;
		Data_t val = rb[0];
		an.min.value = an.max.value = val;
		an.avg = val; 
		an.rms = val*val;
		for( size_t i=1; i<len; i++ ){
			val = rb[i];
			if( an.max.value < val || std::isnan(an.max.value) ){
				an.max.value = val;
				an.max.index = i;
			}
			if( an.min.value > val || std::isnan(an.min.value) ){
				an.min.value = val;
				an.min.index = i;
			}
			an.avg += val;
			an.rms += val * val;
		}
		an.peakToPeak = an.max.value - an.min.value;
		an.avg /= len;
		an.rms = std::sqrt( an.rms / len );
		an.middle = an.min.value + an.peakToPeak /2;
		return an;
	}
	
	
	/// 
	template< typename Data_t >
	Analyzed<Data_t> analyze( RingBuffer<Data_t> const& rb )
	{
		Analyzed<Data_t> an; 
		analyze(an,rb);
		return an;
	}

} // namespace Sledge


namespace slg = Sledge;
