#pragma once
#if defined(_WIN32) || defined(_WIN64)
	#define __WINDOWS_OS__
#elif defined(__unix__) || defined(__unix)  || defined(unix)
	#define __UNIX_OS__
#endif


#if defined(__CMAKE_BUILD__) // __WINDOWS_OS__ or __UNIX_OS__
#include <asio.hpp>
#define __BOOST_NS 
#define __BOOST_SYSTEM_NS asio
#else
#if defined(__WINDOWS_OS__)
#include <boost/asio.hpp>
#define __BOOST_NS boost
#define __BOOST_SYSTEM_NS boost::system
#endif
#endif