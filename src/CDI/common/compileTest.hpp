/** \file compileTest.hpp
 * \brief Common for all C++ files that are compile time unit tests.
 */

#ifndef CDI_COMMON_COMPILETEST_HPP
#define CDI_COMMON_COMPILETEST_HPP

/** \brief Used in a `consteval bool` function to return an error.  */
#define ASSERT(cond) if(!(cond)) return false

#endif // CDI_COMMON_COMPILETEST_HPP
