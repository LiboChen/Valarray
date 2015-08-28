/* Put your solution in this file, we expect to be able to use
 * your epl::valarray class by simply saying #include "Valarray.h"
 *
 * We will #include "Vector.h" to get the epl::vector<T> class
 * before we #include "Valarray.h". You are encouraged to test
 * and develop your class using std::vector<T> as the base class
 * for your epl::valarray<T>
 * you are required to submit your project with epl::vector<T>
 * as the base class for your epl::valarray<T>
 */

#ifndef _Valarray_h
#define _Valarray_h

#include <vector>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <complex>
#include <climits>
#include <cmath>
#include <iterator>
#include "Vector.h"

//using std::vector; // during development and testing
using epl::vector; // after submission

namespace epl {

	template <typename T, typename OP>
    struct UnaProxy;

	template<typename T>
	struct mysqrt;

	template<typename T>
	struct is_scalar;

	template <typename T>
	class scalar;

    template <typename T>
    class vec_wrapper: public T{
    	using T::T;

    public:
    	vec_wrapper(){ }

    	//copy construct from vec_wrapper
    	template<typename T2>
    	vec_wrapper(const vec_wrapper<T2>& rhs){
    		for(int k = 0; k < rhs.size(); k++)
    			this->push_back((typename T::value_type)rhs[k]);
    	}

    	//vec_wrapper assignment
    	template <typename T2>
    	vec_wrapper<T>& operator=(const vec_wrapper<T2>& rhs){
    		int this_size = this->size();
    		int min_size = this_size;
    		if(min_size > rhs.size()) min_size = rhs.size();
    		for(int k = 0; k < min_size; k++)
    			(*this)[k] = rhs[k];
    		return *this;
    	}

    	//scalar assignment
    	template <typename T2, typename Unused = typename is_scalar<T2>::type>
    	vec_wrapper<T>& operator=(const T2& rhs){
    		using value_type = typename T::value_type;
    		int this_size = this->size();
    		for(int k = 0; k < this_size; k++)
    			(*this)[k] = (value_type)rhs;
    		return *this;
    	}

    	//same type assignment
    	vec_wrapper<T>& operator=(const vec_wrapper<T>& rhs){
    		int this_size = this->size();
    		int min_size = this_size;
    		if(min_size > rhs.size()) min_size = rhs.size();
    		for(int k = 0; k < min_size; k++)    //k < this->size()  can't work
    			(*this)[k] = rhs[k];
    		return *this;
    	}

    	//sum
    	typename T::value_type sum(){
    		return accumulate(std::plus<typename T::value_type>{});
    	}

    	//accumulate
    	template <typename OP>
    	typename OP::result_type accumulate(OP op){  //vec_wrapper<vector<int>>
    		using result_type = typename OP::result_type;
    		result_type result;
    		int size = this->size();
    		if(size == 0) return 0;
    		else if(size == 1) return (*this)[0];
    		else{
    			result = op((*this)[0],(*this)[1]);
    			for(int k = 2; k < this->size(); k++)
    			    result = op(result, (*this)[k]);
    		}
    		return result;
    	}

    	//sqrt
    	template<typename OP = mysqrt<typename T::value_type>>  //OP should be default initialized,
    	vec_wrapper<UnaProxy<T, OP>> sqrt(){		            //template parameters can't be deduced
    		return apply(mysqrt<typename T::value_type>{});		//from return expression.
    	}

    	//apply
    	template <typename OP>
    	vec_wrapper<UnaProxy<T, OP>> apply(OP op){
    		return vec_wrapper<UnaProxy<T, OP>> {(*this), op};
    	}
    };

    template <typename T, typename Unused = typename is_scalar<T>::type>
    using valarray = vec_wrapper<vector<T>>;

    template <typename T>
    std::ostream& operator<< (std::ostream& os, const vec_wrapper<T>& v){
    	os << "{";
    	for(int i = 0; i < v.size(); i++)
    		os << v[i] << ", ";
    	os << "}";
    	return os;
    }

    /**************************************my sqrt struct **********************************************/
    template <typename T>
    struct sqrt_return_type;
    template <>	struct sqrt_return_type<int> {	using type = double; };
    template <> struct sqrt_return_type<float> { using type = double; };
    template <>	struct sqrt_return_type<double>{ using type = double; };
    template <> struct sqrt_return_type<std::complex<float>> { using type = std::complex<double>; };
    template <> struct sqrt_return_type<std::complex<double>> { using type = std::complex<double>; };

    template <typename T>
    struct mysqrt{
    	using result_type = typename sqrt_return_type<T>::type;
    	result_type operator()(T val) const{
    		return (result_type)(std::sqrt<T>(val));
    	}
    };

    /**********************************choose appropriate type*****************************************/
    template <typename T>
    struct rank {static constexpr int value = 0;};

    template <> struct rank<int> {	static constexpr int value = 1; };
    template <>	struct rank<float> {	static constexpr int value = 2;	};
    template <> struct rank<double> {	static constexpr int value = 3; };
    template <typename T> struct rank<std::complex<T>> {	static constexpr int value = rank<T>::value; };

    template <int R>
    struct stype;

    template <> struct stype<1> {	using type = int; };
    template <> struct stype<2> {	using type = float; };
    template <> struct stype<3> {	using type = double; };


    template <typename T> struct is_complex: public std::false_type {};
    template <typename T> struct is_complex<std::complex<T>>: public std::true_type {};

    template <bool p, typename T>
    struct ctype;

    template<typename T> struct ctype<true, T> {	using type = std::complex<T>; };
    template<typename T> struct ctype<false, T> {	using type = T;	};

    template <typename T1, typename T2>
    struct choose_type{
    	static constexpr int t1_rank = rank<T1>::value;
    	static constexpr int t2_rank = rank<T2>::value;
    	static constexpr int max_rank = (t1_rank > t2_rank)? t1_rank: t2_rank;
    	using my_stype = typename stype<max_rank>::type;

    	static constexpr bool isT1Complex = is_complex<T1>::value;
    	static constexpr bool isT2Complex = is_complex<T2>::value;
    	static constexpr bool isComplex = isT1Complex || isT2Complex;

    	using type = typename ctype<isComplex, my_stype>::type;
    };
    /***************************************choose appropriate type************************************/

    /***************************************check if it's scalar***************************************/
    template <typename T> struct is_scalar {static constexpr bool value = 0;};
    template <> struct is_scalar<int>				{ using type = int; static constexpr bool value = 1;};
    template <> struct is_scalar<float>				{ using type = float; static constexpr bool value = 1;};
    template <> struct is_scalar<double>			{ using type = double; static constexpr bool value = 1;};
    template <> struct is_scalar<std::complex<float>>	{ using type = std::complex<float>; static constexpr bool value = 1;};
    template <> struct is_scalar<std::complex<double>>	{ using type = std::complex<double>; static constexpr bool value = 1;};


    /*************************************ref or not ***************************************************/
    template <typename T>
    struct to_ref {	using type = T;	};

    template <typename T>
    struct to_ref<vector<T>> {	using type = const vector<T>&; };

    template <typename T>
    struct to_ref<vec_wrapper<vector<T>>> {	using type = const vec_wrapper<vector<T>>&; };

    template <typename T>	using Ref = typename to_ref<T>::type;


    /**************************************scalar type**************************************************/
    template <typename T>
    class scalar{
    public:
    	using value_type = T;
    	value_type val;
    	scalar(T _val): val(_val) {}
    	uint size() const {	return UINT_MAX; }
    	T operator[](int k) const {	return val;	}
    };


    /*************************************my_iteraor*********************************************************/
    template<typename Proxy>
    class my_iterator{
    	Proxy parent;
        uint index;

    public:
    	using iterator_category = std::random_access_iterator_tag;
    	using value_type = typename Proxy::value_type;
    	using same = my_iterator;

        //Make sure the members appear in the initializer list in the same order as they appear in the class
        my_iterator(const Proxy& _parent, const uint& _index): parent(_parent), index(_index) {}

    	value_type operator*(){	return parent[index];}

    	same& operator++(){
    		this->index++;
    		return *this;
    	}

    	same operator++(int){
    		same tmp{*this};
    		this->operator++();
    		return tmp;
    	}

    	bool operator==(const same& rhs) { return (this->index == rhs.index);}
    	bool operator!=(const same& rhs) { return (this->index != rhs.index);}

    	same& operator--(){
    		this->index--;
    		return *this;
    	}

    	same operator--(int){
    		same tmp{*this};
    		this->operator--();
    		return tmp;
    	}

    	same operator+(int k){
    		return my_iterator(this->parent, (this->index + k));
    	}

    	int operator-(const same& rhs) { return (this->index - rhs.index);}

    };

    /**************************************BinProxy type****************************************************/
    template <typename T1, typename T2, typename OP>
    struct BinProxy{
    	BinProxy(const T1& _l, const T2& _r, OP _op): l(_l), r(_r), op(_op) {}
    	Ref<T1> l;
    	Ref<T2> r;
    	OP op;

    	using value_type = typename choose_type<typename T1::value_type,typename T2::value_type>::type;
    	value_type operator[](uint k) const{
    		return (value_type)(op(l[k],r[k]));
    	}

    	uint size() const{
    		uint size = l.size();
    		if(r.size() < size) size = r.size();
    		return size;
    	}

    	/*************************iterator class*****************************************/
    	using const_iterator = my_iterator<BinProxy>;
    	const_iterator begin() const{ return my_iterator<BinProxy>((*this), 0);}
    	const_iterator end() const{	return my_iterator<BinProxy>((*this), this->size());}
    };

      /**************************************UnaProxy type****************************************************/
      template <typename T, typename OP>
      struct UnaProxy{
    	UnaProxy(const T& _r, OP _op): r(_r), op(_op) {}
    	Ref<T> r;
    	OP op;
    	using value_type = typename T::value_type;
    	using result_type = typename OP::result_type;

    	result_type operator[](uint k) const { return op(r[k]);}
    	uint size() const{ return r.size();}

    	/*************************iterator class*****************************************/
    	using const_iterator = my_iterator<UnaProxy>;
    	const_iterator begin() const{ return my_iterator<UnaProxy>((*this), 0);}
    	const_iterator end() const{ return my_iterator<UnaProxy>((*this), this->size());}
     };

    /***************************************unary operations***************************************/
      template <typename T, typename OP = std::negate<typename T::value_type> >   //must default initialize OP
      vec_wrapper<UnaProxy<T, OP>> operator-(const vec_wrapper<T>& rhs){		  //otherwise can't find operator-
    	return vec_wrapper<UnaProxy<T, OP>> {rhs, std::negate<typename T::value_type>{}};
      }

      /*************************************binary operations***************************************/
      //enable_if
      template <bool, typename T> struct enable_if;
      template<typename T> struct enable_if<true, T> {	using type = T; };
      template<typename T> struct enable_if<false, T> {};

      template <typename T1, typename T2, typename OP>
      struct can_do_binary_OP { static constexpr bool value = 0; };

      template<typename T1, typename T2, typename OP>
      struct can_do_binary_OP<vec_wrapper<T1>, vec_wrapper<T2>, OP> {
    	  static constexpr bool value = 1;
    	  using value_type = typename choose_type<typename T1::value_type,typename T2::value_type>::type;
    	  using return_type = vec_wrapper<BinProxy<T1, T2, OP>>;
      };

      template<typename T1, typename T2, typename OP>
      struct can_do_binary_OP<T1, vec_wrapper<T2>, OP>: public is_scalar<T1>{
    	  using value_type = typename choose_type<typename scalar<T1>::value_type,typename T2::value_type>::type;
    	  using return_type = vec_wrapper<BinProxy<scalar<T1>, T2, OP>>;
      };

      template<typename T1, typename T2, typename OP>
      struct can_do_binary_OP<vec_wrapper<T1>, T2, OP>: public is_scalar<T2>{
    	 // static constexpr bool value = 1;
    	  using value_type = typename choose_type<typename T1::value_type,typename scalar<T2>::value_type>::type;
          using return_type = vec_wrapper<BinProxy<T1, scalar<T2>, OP>>;
      };

      template<typename T1, typename T2>
      struct select_type{};

      template<typename T1, typename T2>
      struct select_type<vec_wrapper<T1>, vec_wrapper<T2>>{
    	using type = typename choose_type<typename T1::value_type,typename T2::value_type>::type;
      };

      template<typename T1, typename T2>
      struct select_type<T1, vec_wrapper<T2>>: public is_scalar<T1>{
    	  using type = typename choose_type<T1,typename T2::value_type>::type;
      };

      template<typename T1, typename T2>
      struct select_type<vec_wrapper<T1>, T2>: public is_scalar<T2>{
    	  using type = typename choose_type<typename T1::value_type, T2>::type;
      };

      //operator +
      template<typename T1, typename T2>
      typename enable_if<can_do_binary_OP<T1, T2, std::plus<typename select_type<T1, T2>::type>>::value, typename can_do_binary_OP<T1, T2, std::plus<typename select_type<T1, T2>::type>>::return_type>::type operator+(const T1& lhs, const T2& rhs){
    	  return typename can_do_binary_OP<T1, T2, std::plus<typename select_type<T1, T2>::type>>::return_type {lhs, rhs, std::plus<typename select_type<T1, T2>::type>{}};
      }

      //operator -
      template<typename T1, typename T2>
      typename enable_if<can_do_binary_OP<T1, T2, std::minus<typename select_type<T1, T2>::type>>::value, typename can_do_binary_OP<T1, T2, std::minus<typename select_type<T1, T2>::type>>::return_type>::type operator-(const T1& lhs, const T2& rhs){
    	  return typename can_do_binary_OP<T1, T2, std::minus<typename select_type<T1, T2>::type>>::return_type {lhs, rhs, std::minus<typename select_type<T1, T2>::type>{}};
      }

      //operator *
      template<typename T1, typename T2>
      typename enable_if<can_do_binary_OP<T1, T2, std::multiplies<typename select_type<T1, T2>::type>>::value, typename can_do_binary_OP<T1, T2, std::multiplies<typename select_type<T1, T2>::type>>::return_type>::type operator*(const T1& lhs, const T2& rhs){
    	  return typename can_do_binary_OP<T1, T2, std::multiplies<typename select_type<T1, T2>::type>>::return_type {lhs, rhs, std::multiplies<typename select_type<T1, T2>::type>{}};
      }

      //operator /
      template<typename T1, typename T2>
      typename enable_if<can_do_binary_OP<T1, T2, std::divides<typename select_type<T1, T2>::type>>::value, typename can_do_binary_OP<T1, T2, std::divides<typename select_type<T1, T2>::type>>::return_type>::type operator/(const T1& lhs, const T2& rhs){
    	  return typename can_do_binary_OP<T1, T2, std::divides<typename select_type<T1, T2>::type>>::return_type {lhs, rhs, std::divides<typename select_type<T1, T2>::type>{}};
      }


}


#endif /* _Valarray_h */
