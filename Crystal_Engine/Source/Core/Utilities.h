//==============================================================================
#pragma once
#include "TypeAndConcepts.h"

#include <cassert>
#include <iostream>
//==============================================================================
#if _DEBUG
/**
 * @brief               Do compiletime check
 * @details             
 */
#define CRST_COMPILETIME_CHECK(condition, message) \
	do {\
		static_assert(condition, message);\
    } while(0)
/**
 * @brief               Do runtime check
 * @details             Automatically triggers a breakpoint or returns if the condition is false
 * @param condition     Conditions that must be satisfied
 * @param message       The error message to display if the constraint is not met
 * @param ...           return vals
 */
#define CRST_RUNTIME_CHECK(condition, message, ...) \
    do { \
        if (!condition) { \
            std::cout<< "[CRYSTAL_ERROR]" << message << std::endl; \
            assert(false);\
            return __VA_ARGS__; \
        } \
    } while (0)
#else
#define CRST_COMPILETIME_CHECK(condition, message) \
	do {\
		static_assert(condition, message);\
    } while(0)
#define CRST_RUNTIME_CHECK(condition, message, ...) \
    do { \
        if (condition) { \
            std::cout<< "[CRYSTAL_ERROR]" << message << std::endl; \
            return __VA_ARGS__; \
        } \
    } while (0)
#endif