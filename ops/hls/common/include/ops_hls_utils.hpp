#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @file 
  * @brief HLS related utilitis for L1 layer.
  * @author Beniel Thileepan
  * @details Contains utility functions used in other L1 layer 
  * components as a common collectives.
  */

#include <cstdarg>
#include <math.h>

#define NUMARGS(T, ...)  (sizeof((T[]){__VA_ARGS__})/sizeof(T))

static int add(unsigned int N, ...)
{
    unsigned int total = 0;

    std::va_list args;
    va_start(args, N);

    for (unsigned int i = 0; i < N; i++)
    {
        total +=  va_arg(args, int);
    }

    va_end(args);

    return total;
}

static const int multiply(unsigned int N, ...)
{
    unsigned int total = 0;

    std::va_list args;
    va_start(args, N);

    for (unsigned int i = 0; i < N; i++)
    {
        total *=  va_arg(args, int);
    }

    va_end(args);

    return total;
}

template <typename T>
static T register_it(T x){
	#pragma HLS inline off
	T tmp = x;
	return tmp;
}

#define INT_SUM(...) (add(NUMARGS(int, __VA_ARGS__), __VA_ARGS__))
#define INT_MUL(...) (multiply(NUMARGS(int, __VA_ARGS__), __VA_ARGS__))

// constexpr unsigned short log2_int(unsigned int num) {
//     unsigned short res = 0;
//     while (num > 1) {
//         num >>= 1;
//         ++res;
//     }
//     return res;
// }

// #define LOG2(num) (log2_int(num))


// This fails at synthesis due to HLS compiler unable to use it only on compilation stage
constexpr unsigned short log2_int(unsigned int num) {
    return (num <= 1) ? 0: 1 + log2_int(num >> 1);
}

#define LOG2_NON_CONSTEXPR(num) (log2_int(num))

// this implementation is template metaprogramming forcing on compile time
template <unsigned int N>
struct Log2 {
    static const unsigned short value = 1 + Log2<N / 2>::value;
};

// Base case for 1
template <>
struct Log2<1> {
    static const unsigned short value = 0;
};

// Base case for 0 (to prevent infinite loops on invalid input)
template <>
struct Log2<0> {
    static const unsigned short value = 0; 
};

#define LOG2(num) (Log2<num>::value)



static inline unsigned int pow2_int(unsigned int num) {
    return (num < sizeof(unsigned int)*8u) ? (1u << num) : 0u;
}

#define POW2(num) (pow2_int(num))
#define DUMP_VAR_NAME(var)(#var);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
