//==============================================================================
#pragma once
#include "TypeAndConcepts.h"
#include <iostream>
#include <cassert>
#include <type_traits>
//==============================================================================
/*
	Compile-time
 */
//==============================================================================

 /**
  * @brief		CRST_REQUIRE macro
  * @details	A wrapper for C++20 requires-expressions. 
  *				Leads to SFINAE \em "(Substitution Failure Is Not An Error)" when constraints are not met.
  * @param		CRSTConcept The concept or predicate to verify.
  * @param		... Arbitrary template parameters for the concept.
  */
#define CRST_REQUIRE(CRSTConcept, ...) \
	CRSTConcept<__VA_ARGS__>

  /**
   * @brief		CRST_CONSTRAIN macro
   * @details	A wrapper for static_assert.
   *			Causes a hard compile-time error when constraints are not met.
   * @param		condition The boolean constant expression to verify.
   * @param		msg The hint message displayed during compilation failure.
   */
#define CRST_CONSTRAIN(condition, msg) \
	static_assert(condition, msg)

//==============================================================================
/*
   Run-time
   These macros handle the "Laws of Physics" during engine execution.
*/
//==============================================================================

#ifdef _DEBUG
/**
 * @brief		CRST_EXPECT macro (Debug)
 * @details		Logs failure details to stderr and performs a non-fatal return.
 * @param		condition Condition that should be true for normal execution.
 * @param		msg Detailed debug message to print on failure.
 * @param		... Values to return as a fallback.
 */
#define CRST_EXPECT(condition, msg, ...) \
		do { \
			if (!(condition)) { \
				std::cerr << "[CRYSTAL_EXPECT_FAILED]\n" \
						  << "Condition: " << #condition << "\n" \
						  << "Message:   " << msg << "\n" \
						  << "Location:  " << __FILE__ << ":" << __LINE__ << std::endl; \
				return __VA_ARGS__; \
			} \
		} while (0)

/**
 * @brief		CRST_ASSERT macro (Debug)
 * @details		Logs failure details and triggers a hardware breakpoint/assertion.
 *				Used for fatal logic errors that should stop development execution.
 * @param		condition Condition that MUST be met.
 * @param       msg Error message to display before aborting.
 */
#define CRST_ASSERT(condition, msg) \
		do { \
			if (!(condition)) { \
				std::cerr << "[CRYSTAL_ASSERT_FAILED]\n" \
						  << "Message:  " << msg << "\n" \
						  << "Location: " << __FILE__ << ":" << __LINE__ << std::endl; \
				assert(false && msg); \
			} \
		} while (0)

#else
/**
 * @brief		CRST_EXPECT macro (Release)
 * @details		Optimized version that removes I/O overhead but maintains the safety return logic.
 * @param		condition Condition to verify.
 * @param		msg Message (ignored in release mode to save binary space).
 * @param		... Values to return to prevent catastrophic engine failure.
 */
#define CRST_EXPECT(condition, msg, ...) \
		do { \
			if (!(condition)) { \
				return __VA_ARGS__; \
			} \
		} while (0)

/**
 * @brief		CRST_ASSERT macro (Release)
 * @details		Eradicates assertions for zero-overhead performance.
 *				Optionally provides optimization hints to the compiler via __assume.
 * @param		condition Condition to assume or discard.
 * @param		msg Message (completely removed in release build).
 */
#if defined(_MSC_VER)
#define CRST_ASSERT(condition, msg) __assume(condition)
#else
#define CRST_ASSERT(condition, msg) ((void)0) 
#endif

#endif

#define CRST_NON_COPYABLE(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

#define CRST_NON_MOVABLE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

#define CRST_SINGLETON(ClassName) \
    CRST_NON_COPYABLE(ClassName) \
    CRST_NON_MOVABLE(ClassName)