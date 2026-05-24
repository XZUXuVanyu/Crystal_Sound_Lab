//==============================================================================
#pragma once
#include <iostream>
#include <cassert>
#include <utility>
//==============================================================================
/*
	Compile-time Constraints
 */
//==============================================================================

 /**
  * @brief		CRST_REQUIRE macro
  * @details	A wrapper for C++20 requires-expressions. 
  *				Leads to SFINAE (Substitution Failure Is Not An Error) when constraints are not met.
  */
#define CRST_REQUIRE(CRSTConcept, ...) \
	CRSTConcept<__VA_ARGS__>

  /**
   * @brief		CRST_CONSTRAIN macro
   * @details	A wrapper for static_assert.
   *			Causes a hard compile-time error when constraints are not met.
   */
#define CRST_CONSTRAIN(condition, msg) \
	static_assert(condition, msg)

#ifdef _DEBUG
/**
 * @brief		CRST_EXPECT macro (Debug)
 */
#define CRST_EXPECT(condition, msg, ...) \
		do { \
			if (condition) [[likely]] {} \
			else [[unlikely]] { \
				std::cerr << \
				"[CRYSTAL_EXPECT_FAILED]\n" \
				<< "Condition: " << #condition << "\n" \
				<< "Message:   " << msg << "\n" \
				<< "Location:  " << __FILE__ << ":" << __LINE__ << std::endl; \
				return __VA_ARGS__; \
			}\
		} while (0)

/**
 * @brief		CRST_ASSERT macro (Debug)
 */
#define CRST_ASSERT(condition, msg) \
		do { \
			if (condition) [[likely]] {} \
			else [[unlikely]] { \
				std::cerr \
				<< "[CRYSTAL_ASSERT_FAILED]\n" \
				<< "Message:  " << msg << "\n" \
				<< "Location: " << __FILE__ << ":" << __LINE__ << std::endl; \
				assert(false and msg); \
			} \
		} while (0)

#else
/**
 * @brief		CRST_EXPECT macro (Release)
 */
#define CRST_EXPECT(condition, msg, ...) \
		do { \
			if (condition) [[likely]] \
			else { \
				return __VA_ARGS__; \
			} \
		} while (0)

/**
 * @brief		CRST_ASSERT macro (Release)
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

#define CRST_BIND_CALLBACK(func) [this](auto&&... args) -> decltype(auto) \
    { return this->func(std::forward<decltype(args)>(args)...); }

#define CRST_BIND_CALLBACK_WITH_INSTANCE(func, instance) [instance](auto&&... args) -> decltype(auto) \
    { return instance->func(std::forward<decltype(args)>(args)...); }
