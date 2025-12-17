#pragma once
#include <Helpers/BoostIsSupported.h>

// This header must be included first in .cpp file to have no errors with boost/asio and winsock

#ifdef BOOST_SUPPORTED
// order in which boost sees the correct value of _WIN32_WINNT
// https://stackoverflow.com/questions/9750344/boostasio-winsock-and-winsock-2-compatibility-issue
// https://github.com/boostorg/beast/issues/1895
#include <winsock2.h>
#include <boost/asio.hpp>
#endif
