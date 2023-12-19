// STUNServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <iostream>
#ifdef __WINDOWS_OS__
#include <asio.hpp>
#include "TcpSocketServer.h"
#include "UdpSocketServer.h"
#endif
#include "QuicSocketServer.h"
#include "Logger.h"

//#ifdef __UNIX_OS__
////#define BOOST_STACKTRACE_USE_BACKTRACE // require libbacktrace + backtrace.h
//#define BOOST_STACKTRACE_USE_ADDR2LINE
//#include <boost/stacktrace.hpp>
//#endif


void SignalHandler(int signal) {
	switch (signal) {
	case SIGABRT:
		WriteError("SIGABRT signal");
		break;
	case SIGFPE:
		WriteError("SIGFPE signal");
		break;
	case SIGILL:
		WriteError("SIGILL signal");
		break;
	case SIGINT:
		WriteError("SIGINT signal");
		break;
	case SIGSEGV:
		WriteError("SIGSEGV signal");
		break;
	case SIGTERM:
		WriteError("SIGTERM signal");
		break;
#ifdef __UNIX_OS__
	case SIGPIPE:
		WriteError("SIGPIPE signal --> ignore");
		return;
#endif
	}

//#ifdef __UNIX_OS__
//	auto st = boost::stacktrace::stacktrace();
//	WriteDebug("Stack trace:\n%s", boost::stacktrace::to_string(st).c_str());
//#endif

	P7_Exceptional_Flush();
	std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
	exit(0);
}

int main() {
	std::cout << "STUNServer started ..." << std::endl;
	std::signal(SIGABRT, SignalHandler);
	std::signal(SIGFPE, SignalHandler);
	std::signal(SIGILL, SignalHandler);
	std::signal(SIGINT, SignalHandler);
	std::signal(SIGSEGV, SignalHandler);
	std::signal(SIGTERM, SignalHandler);
#ifdef __UNIX_OS__
	std::signal(SIGPIPE, SignalHandler);
#endif

	//UserDataExchanger t;
	//t.AddNewUser(1, ActiveUse)
	auto server = QuicSocketServer{ 53488 };
	server.StartListening();
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	WriteError("Exit main loop");
	std::cout << "STUNServer finished" << std::endl;
	return 0;
}