#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"
#include <vector>

using boost::asio::ip::tcp;

// Заявка на покупку/продажу
struct Order
{
    Order(const std::string& userId, double volume, double price, OrderType type)
    {
        this->userId = userId;
        this->volume = volume;
        this->price = price;
        this->type = type;
    };

//private:
    std::string userId;
    double volume;
    double price;
    OrderType type;
};

class Core
{
public:
    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& aUserName)
    {
        size_t newUserId = mUsers.size();
        mUsers[newUserId] = aUserName;  // сохраняет пользователя в map
        balance[newUserId] = {0.0, 0.0}; // изначальный баланс пользователя(хардкод потому что пока нет БД для хранения)

        return std::to_string(newUserId); //Возвращает его id
    }

    // Запрос имени клиента по ID
    std::string GetUserName(const std::string& aUserId)
    {
        const auto userIt = mUsers.find(std::stoi(aUserId)); // Ищем пользователя по id
        if (userIt == mUsers.cend())
        {
            return "Error! Unknown User";
        }
        else
        {
            return userIt->second;
        }
    }
    // Получить баланс пользователя
    std::string GetBalance(const std::string& userId) {
        const auto balanceIt = balance.find(std::stoi(userId));
        if (balanceIt == balance.cend()) {
            return "Error! Unknown User";
        } else {
            auto balance = balanceIt->second;
            //return std::to_string(balance.first) + " " + std::to_string(balance.second);
            return "Balance: " + std::to_string(balance.first) + " USD, " + std::to_string(balance.second) + " RUB \n";
        }
    }
    // Добавление заявок
    std::string AddOrder(const std::string& userId, double volume, double price, OrderType type) {
        Order order(userId, volume, price, type);   // Создаём ордер
        std::string response = MatchOrder(order);   // Считаем
        if (order.volume > 0) {                     // Если у заявки остался объём, сохраняем её в нужный вектор
            if (type == OrderType::Buy) {
                buyOrders.push_back(order);
                std::sort(buyOrders.begin(), buyOrders.end(), [](const Order& a, const Order& b) { return a.price > b.price; });
            } else {
                sellOrders.push_back(order);
                std::sort(sellOrders.begin(), sellOrders.end(), [](const Order& a, const Order& b) { return a.price < b.price; });
            }
        }
        return response;
    }

private:
    std::map<size_t, std::string> mUsers; // Храним пользователей: ключ-id, значение-имя
    std::map<size_t, std::pair<double, double>> balance; // Баланс пользователя: ключ-id, Значение <USD, RUB>
    //std::vector<Order> orders; // Все заявки
    std::vector<Order> buyOrders; // Все заявки на покупку
    std::vector<Order> sellOrders; // Все заявки на продажу


    // Обновляем баланс после сделки
    void UpdateBalance(const std::string& buyerId, const std::string& sellerId, double volume, double price) {
        balance[std::stoi(buyerId)].first += volume;            // Увеличиваем количество USD у покупателя
        balance[std::stoi(buyerId)].second -= volume * price;   // Уменьшаем количество RUB у покупателя
        balance[std::stoi(sellerId)].first -= volume;           // Уменьшаем количество USD у продавца
        balance[std::stoi(sellerId)].second += volume * price;  // Увеличиваем количество RUB у продавца
    }
    // Обработка нового запроса
    std::string MatchOrder(Order& order) {
        std::vector<Order>& orders = (order.type == OrderType::Buy) ? sellOrders : buyOrders;   // Определяем вектор нужного типа ордера
        std::string response;                                                                   // Строка для хранения результата выполнения функции
        double remainingVolume = order.volume;                                                  // Оставшийся объем текущего ордера, который еще не был выполнен

        for (auto it = orders.begin(); it != orders.end();) {                                   // Проходим по выбранному вектору ордеров
            if ((order.type == OrderType::Buy && order.price >= it->price) ||                   // Может ли текущий ордер быть выполнен с рассматриваемым ордером
                (order.type == OrderType::Sell && order.price <= it->price)) {

                double matchedVolume = std::min(remainingVolume, it->volume);                   // Рассчитывается объем, который может быть выполнен
                if (order.type == OrderType::Buy) {                                             // Обновляются балансы пользователей, участвующих в сделке
                    UpdateBalance(order.userId, it->userId, matchedVolume, it->price);
                } else {
                    UpdateBalance(it->userId, order.userId, matchedVolume, it->price);
                }
                // Информация о выполненной сделке в строку ответа
                response += "Matched " + std::to_string(matchedVolume) + " USD at " + std::to_string(it->price) + " RUB with User " + it->userId + "\n";
                remainingVolume -= matchedVolume;   // Обновляются оставшийся объем текущего ордера и объем рассматриваемого ордера
                it->volume -= matchedVolume;
                if (it->volume <= 0) {              // Если объем рассматриваемого ордера исчерпан, он удаляется из вектора ордеров
                    it = orders.erase(it);
                } else {
                    ++it;
                }
                if (remainingVolume <= 0) {         // Если оставшийся объем текущего ордера исчерпан, цикл прерывается.
                    break;
                }
            } else {
                ++it;
            }
        }

        order.volume = remainingVolume;             // Обновляется объем текущего ордера (если он не полностью выполнен).
        return response.empty() ? "No matching orders found\n" : response;
    }
};

Core& GetCore()
{
    static Core core;
    return core;
}

// обработка одного соединения с клиентом
class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket() //Возвращает ссылку на сокет.
    {
        return socket_;
    }

    // Запускает ожидание новых сообщений
    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error)
        {
            data_[bytes_transferred] = '\0';

            // Парсим json, который пришёл нам в сообщении.
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];

            std::string reply = "Error! Unknown request type";
            if (reqType == Requests::Registration)
            {
                // Это реквест на регистрацию пользователя.
                // Добавляем нового пользователя и возвращаем его ID.
                reply = GetCore().RegisterNewUser(j["Message"]);
            }
            else if (reqType == Requests::Hello)
            {
                // Это реквест на приветствие.
                // Находим имя пользователя по ID и приветствуем его по имени.
                reply = "Hello, " + GetCore().GetUserName(j["UserId"]) + "!\n";
            }
            else if (reqType == Requests::Balance)
            {
                // Это реквест на просмотр баланса.
                reply = GetCore().GetBalance(j["UserId"]);
            }
            else if (reqType == Requests::Order)
            {
                // Это реквест на заявку.
                std::string messageStr = j["Message"];
                auto orderMessage = nlohmann::json::parse(messageStr);
                double volume = orderMessage["Volume"];
                double price = orderMessage["Price"];
                OrderType type = orderMessage["TypeOrder"];
                reply = GetCore().AddOrder(j["UserId"], volume, price, type);
            }

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply, reply.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    //Запуск нового чтения данных от клиента.
    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    // Создает объект acceptor, который слушает входящие подключения на определенном порту.
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    //Cоздает новый объект session для каждого нового подключения и запускает его.
    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
