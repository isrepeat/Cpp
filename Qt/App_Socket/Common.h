#pragma once
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <string>

using boost::asio::ip::udp;
using boost::asio::ip::address;

#define IPADDRESS "127.0.0.1"
//#define IPADDRESS "192.168.0.101"
//#define IPADDRESS "88.154.78.66"
#define UDP_PORT 13200