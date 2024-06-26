#ifndef CLIENT_H
#define CLIENT_H
#include <iostream>
#include <boost/asio.hpp>
#include "Common.hpp"
#include "json.hpp"
#include <boost/test/unit_test.hpp>

using boost::asio::ip::tcp;

void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage);

std::string ReadMessage(tcp::socket& aSocket);
std::string ProcessRegistration(tcp::socket& aSocket);
void AddOrder(tcp::socket& s, const std::string& my_id);
//int main();
#endif // CLIENT_H
