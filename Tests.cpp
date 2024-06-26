#define BOOST_TEST_MODULE MyTest
#include "Core.hpp"
#include <boost/test/unit_test.hpp>
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
// // Отправка сообщения на сервер
void SendMessage(tcp::socket& aSocket, const std::string& aId, const std::string& aRequestType, const std::string& aMessage) {
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}
// // Чтение сообщения от сервера
std::string ReadMessage(tcp::socket& aSocket) {
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

BOOST_AUTO_TEST_CASE(TestRegisterNewUser) { // Тест регистрации пользователя

    boost::asio::io_service io_service_1;
    tcp::socket socket_1 = ConnectClient(io_service_1);
    std::string name_1 = "TestUser_1";
    SendMessage(socket_1, "0", Requests::Registration, name_1);
    std::string userId_1 = ReadMessage(socket_1);
    bool check_1 = (userId_1 == "0") ? true : false;

    boost::asio::io_service io_service_2;
    tcp::socket socket_2 = ConnectClient(io_service_2);
    std::string name_2 = "TestUser_2";
    SendMessage(socket_2, "0", Requests::Registration, name_2);
    std::string userId_2 = ReadMessage(socket_2);
    bool check_2 = (userId_2 == "1") ? true : false;

    BOOST_CHECK(check_1 && check_2); // Предполагаем, что регистрация возвращает "0" как ID первого пользователя, и "1" как ID второго пользователя
};
BOOST_AUTO_TEST_CASE(BalanceUser) { // Проверка начального баланса

    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);
    std::string name = "TestUser_3";
    SendMessage(socket, "0", Requests::Registration, name);
    std::string userId = ReadMessage(socket);

    SendMessage(socket, userId, Requests::Balance, "");
    std::string balance = ReadMessage(socket);
    BOOST_CHECK(balance == "Balance: 0.000000 USD, 0.000000 RUB \n"); // Ожидаемый ответ
};
BOOST_AUTO_TEST_CASE(ProcessingOrderWithoutExecutingTransaction) { // Тест обработка заявки без совершения сделки

    boost::asio::io_service io_service_1;
    tcp::socket socket = ConnectClient(io_service_1);
    std::string name = "TestUser_4";
    SendMessage(socket, "0", Requests::Registration, name);
    std::string userId_1 = ReadMessage(socket);

    double volume = 10, price = 62;
    OrderType type = OrderType::Buy;
    nlohmann::json order;
    order["Volume"] = volume;
    order["Price"] = price;
    order["TypeOrder"] = type;

    SendMessage(socket, userId_1, Requests::Order, order.dump());
    std::string response = ReadMessage(socket);

    BOOST_CHECK(response == "No matching orders found\n");
};
BOOST_AUTO_TEST_CASE(ProcessingOrderTransaction) { // Тест обработка заявки с совершением сделки

    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);
    std::string name = "TestUser_5";
    SendMessage(socket, "0", Requests::Registration, name);
    std::string userId_1 = ReadMessage(socket);

    double volume = 20, price = 62;
    OrderType type = OrderType::Sell;
    nlohmann::json order;
    order["Volume"] = volume;
    order["Price"] = price;
    order["TypeOrder"] = type;

    SendMessage(socket, userId_1, Requests::Order, order.dump());
    std::string response = ReadMessage(socket);
    BOOST_CHECK(response == "Matched 10.000000 USD at 62.000000 RUB with User 3\n");
};
BOOST_AUTO_TEST_CASE(CheckingNonNullBalance) { // Тест проверки ненулевого баланса

    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);
    SendMessage(socket, "4", Requests::Balance, "");
    std::string response = ReadMessage(socket);
    BOOST_CHECK(response == "Balance: -10.000000 USD, 620.000000 RUB \n");
};
BOOST_AUTO_TEST_CASE(FullExecutionOfPartiallyExecutedOrder) { // Полное исполнение частично выполненой заявки

    boost::asio::io_service io_service;
    tcp::socket socket = ConnectClient(io_service);

    double volume = 10, price = 62;
    OrderType type = OrderType::Buy;
    nlohmann::json order;
    order["Volume"] = volume;
    order["Price"] = price;
    order["TypeOrder"] = type;

    SendMessage(socket, "3", Requests::Order, order.dump());
    std::string response = ReadMessage(socket);
    BOOST_CHECK(response == "Matched 10.000000 USD at 62.000000 RUB with User 4\n");
};

BOOST_AUTO_TEST_CASE(ExecutionWithMultipleOrders) { // Исполнение с несколькими заявками

    boost::asio::io_service io_service_1;
    tcp::socket socket_1 = ConnectClient(io_service_1);
    std::string name_1 = "TestUser_6";
    SendMessage(socket_1, "0", Requests::Registration, name_1);
    std::string userId_1 = ReadMessage(socket_1);
    double volume_1 = 10, price_1 = 62;
    OrderType type_1 = OrderType::Buy;
    nlohmann::json order_1;
    order_1["Volume"] = volume_1;
    order_1["Price"] = price_1;
    order_1["TypeOrder"] = type_1;
    SendMessage(socket_1, userId_1, Requests::Order, order_1.dump());
    std::string response_1 = ReadMessage(socket_1);
    bool check_1 = (response_1 == "No matching orders found\n") ? true : false;

    boost::asio::io_service io_service_2;
    tcp::socket socket_2 = ConnectClient(io_service_2);
    std::string name_2 = "TestUser_7";
    SendMessage(socket_2, "0", Requests::Registration, name_2);
    std::string userId_2 = ReadMessage(socket_2);
    double volume_2 = 20, price_2 = 63;
    OrderType type_2 = OrderType::Buy;
    nlohmann::json order_2;
    order_2["Volume"] = volume_2;
    order_2["Price"] = price_2;
    order_2["TypeOrder"] = type_2;
    SendMessage(socket_2, userId_2, Requests::Order, order_2.dump());
    std::string response_2 = ReadMessage(socket_2);
    bool check_2 = (response_2 == "No matching orders found\n") ? true : false;

    boost::asio::io_service io_service_3;
    tcp::socket socket_3 = ConnectClient(io_service_3);
    std::string name_3 = "TestUser_8";
    SendMessage(socket_3, "0", Requests::Registration, name_3);
    std::string userId_3 = ReadMessage(socket_3);
    double volume_3 = 50, price_3 = 61;
    OrderType type_3 = OrderType::Sell;
    nlohmann::json order_3;
    order_3["Volume"] = volume_3;
    order_3["Price"] = price_3;
    order_3["TypeOrder"] = type_3;
    SendMessage(socket_3, userId_3, Requests::Order, order_3.dump());
    std::string response_3 = ReadMessage(socket_3);
    std::string ExpectedResult = "Matched 20.000000 USD at 63.000000 RUB with User 6\nMatched 10.000000 USD at 62.000000 RUB with User 5\n";
    bool check_3 = (response_3 == ExpectedResult) ? true : false;

    BOOST_CHECK(check_1 && check_2 && check_3);
};


