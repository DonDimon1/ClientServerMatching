#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include "Common.hpp"
#include "json.hpp"
//#include <boost/test/included/unit_test.hpp>
using boost::asio::ip::tcp;

// Подключение клиента к серверу
tcp::socket ConnectClient(boost::asio::io_service& io_service) {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, iterator);
    return socket;
}
// Отправка сообщения на сервер
void SendMessage(tcp::socket& aSocket, const std::string& aId, const std::string& aRequestType, const std::string& aMessage) {
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}
// Чтение сообщения от сервера
std::string ReadMessage(tcp::socket& aSocket) {
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

BOOST_AUTO_TEST_CASE(TestRegisterNewUser) { // Тест регистрации пользователя

    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);
    std::string name = "TestUser";

    SendMessage(socket, "0", Requests::Registration, name);
    std::string userId = ReadMessage(socket);

    BOOST_CHECK(userId == "0"); // Предполагаем, что регистрация возвращает "0" как ID первого пользователя
};

BOOST_AUTO_TEST_CASE(BalanceTest) { // Тест просмотра нулевого баланса
    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);
    std::string name = "TestUser";

    SendMessage(socket, "0", Requests::Registration, name);
    std::string userId = ReadMessage(socket);

    SendMessage(socket, userId, Requests::Balance, "");
    std::string response = ReadMessage(socket);

    BOOST_CHECK(response == "Balance: 0.000000 USD, 0.000000 RUB \n");
}

