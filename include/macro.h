/**
 * @file macro.h
 * @brief 常用宏的封装
 * @date 2024-03-05
 */
#ifndef __INCLUDE_MACRO_H__
#define __INCLUDE_MACRO_H__

#include <string.h>
#include <assert.h>
//#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define SYLAR_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define SYLAR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define SYLAR_LIKELY(x) (x)
#define SYLAR_UNLIKELY(x) (x)
#endif

#endif