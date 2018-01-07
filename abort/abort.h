/**
 *  \file   abort.h
 *  \author Jason Fernandez
 *  \date   10/18/2017
 *
 *  https://github.com/jfern2011/abort
 */

#ifndef __ABORT_H__
#define __ABORT_H__

#include <cerrno>
#include <cstdio>

#define AbortIf_2(cond, ret)                           \
{                                                      \
	if (cond)                                          \
	{                                                  \
		char errMsg[256];                              \
                                                       \
		std::sprintf(errMsg, "[abort] %s in %s:%d",    \
			         __PRETTY_FUNCTION__,              \
			         __FILE__, __LINE__ );             \
                                                       \
		if (errno)                                     \
		{                                              \
			std::perror(errMsg); std::fflush(stderr);  \
			errno = 0;                                 \
		}                                              \
		else                                           \
		{                                              \
			std::printf("%s\n", errMsg);               \
			std::fflush(stdout);                       \
		}                                              \
		return (ret);                                  \
	}                                                  \
}

#define AbortIf_3(cond, ret, msg)                              \
{                                                              \
	if (cond)                                                  \
	{                                                          \
		char errMsg[256];                                      \
                                                               \
		std::snprintf(errMsg, 256, "[abort] %s in %s:%d: %s",  \
			         __PRETTY_FUNCTION__,                      \
			         __FILE__, __LINE__,                       \
			         msg);                                     \
                                                               \
		                                                       \
		std::printf( "%s\n", errMsg ); std::fflush( stdout );  \
		return (ret);                                          \
	}                                                          \
}

#define AbortIfNot_2(cond, ret)      \
	AbortIf_2(!(cond), ret)

#define AbortIfNot_3(cond, ret, msg) \
	AbortIf_3(!(cond), ret, msg)


#define cat(a,b) a ## b
#define _select(name, nargin)  cat( name ## _, nargin )
#define get_nargin(_0, _1, nargin, ...) nargin
#define va_size(dummy, ...) \
					 get_nargin(0, ##__VA_ARGS__, 3, 2)

/**
 * @def AbortIf(cond, ret, ...)
 *
 * Triggers an abort in the event that the specified condition \a cond
 * is true. This will cause the currently executing function to exit
 * with the return value \a ret. A 3rd argument (optional) may be used
 * to provide an error message
 */
#define AbortIf(cond, ret, ...) \
	_select(AbortIf, \
		va_size(0, ##__VA_ARGS__))(cond, ret, ##__VA_ARGS__)

/**
 * @def Abort(ret, ...)
 *
 * Triggers an unconditional abort.  This will cause the currently
 * executing function to exit with the return value \a ret. A
 * 2nd argument (optional) may be used to provide an error message
 */
#define Abort(ret, ...) \
	_select(AbortIf, \
		va_size(0, ##__VA_ARGS__))(true, ret, ##__VA_ARGS__)

/**
 * @def AbortIfNot(cond, ret, ...)
 *
 * Triggers an abort in the event that the specified condition \a cond
 * is false. This will cause the currently executing function to exit
 * with the return value \a ret. A 3rd argument (optional) may be used
 * to provide an error message
 */
#define AbortIfNot(cond, ret, ...) \
	_select(AbortIfNot, \
		va_size(0, ##__VA_ARGS__))(cond, ret, ##__VA_ARGS__)


#endif
