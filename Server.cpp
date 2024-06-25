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
    std::string AddOrder(const std::string& userId, double volume, double price, OrderType type)
    {
        Order order(userId, volume, price, type); // Создаём ордер
        std::string response = MatchOrder(order);
        //orders.push_back(order); // Сохраняем его
        //return "Purchase order added\n";
        return response;
    }


private:
    std::map<size_t, std::string> mUsers; // Храним пользователей: ключ-id, значение-имя
    std::map<size_t, std::pair<double, double>> balance; // Баланс пользователя: ключ-id, Значение <USD, RUB>
    std::vector<Order> orders; // Все заявки

    // Обновляем баланс после сделки
    void UpdateBalance(const std::string& buyerId, const std::string& sellerId, double volume, double price) {
        balance[std::stoi(buyerId)].first += volume;            // Увеличиваем количество USD у покупателя
        balance[std::stoi(buyerId)].second -= volume * price;   // Уменьшаем количество RUB у покупателя
        balance[std::stoi(sellerId)].first -= volume;           // Уменьшаем количество USD у продавца
        balance[std::stoi(sellerId)].second += volume * price;  // Увеличиваем количество RUB у продавца
    }

    // Обработка нового запроса
    std::string MatchOrder(Order& order) {                      // Ссылка на новый ордер, который нужно сопоставить с существующими ордерами.
        std::vector<Order> matchedOrders;                       // вектор для хранения ордеров, с которыми была выполнена сделка.
        double remainingVolume = order.volume;                  // оставшийся объем текущего ордера, который еще не был выполнен.
        std::string response;                                   // строка для хранения результата выполнения функции, которая будет возвращена.

        // Сортируем заявки по цене в нужном порядке
        if (order.type == OrderType::Buy) {
            std::sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
                return a.price < b.price;                       // Сортировка по возрастанию цены для заявок на продажу
            });
        } else if (order.type == OrderType::Sell) {
            std::sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
                return a.price > b.price;                       // Сортировка по убыванию цены для заявок на покупку
            });
        }

        for (auto it = orders.begin(); it != orders.end();) {    // Проходим по всем существующим ордерам
            // Проверяем, является ли текущий ордер заявкой на покупку, а существующий ордер заявкой на продажу.
            // Проверяем, пересекаются ли цены (цена покупки выше или равна цене продажи).
            if (order.type == OrderType::Buy && it->type == OrderType::Sell && order.price >= it->price) {
                // Определяем объем сделки как минимальный объем между оставшимся объемом текущего ордера и объемом существующего ордера.
                double matchedVolume = std::min(remainingVolume, it->volume);
                UpdateBalance(order.userId, it->userId, matchedVolume, it->price);  // Обновляем балансы покупателей и продавцов после сделки.
                //Добавляем информацию о выполненной сделке в ответ.
                response += "Matched " + std::to_string(matchedVolume) + " USD at " + std::to_string(it->price) + " RUB with User " + it->userId + "\n";
                remainingVolume -= matchedVolume;               //Уменьшаем оставшийся объем текущего ордера
                it->volume -= matchedVolume;                    //и объем существующего ордера.
                if (it->volume <= 0) {                          //Если объем существующего ордера стал нулевым,
                    it = orders.erase(it);                      //удаляем его из списка и продолжаем цикл с новым итератором.
                } else {                                        //Если объем не стал нулевым,
                    ++it;                                       //просто инкрементируем итератор.
                }
                if (remainingVolume <= 0) {                     //Если оставшийся объем текущего ордера стал нулевым
                    break;                                      //выходим из цикла.
                }
            } else if (order.type == OrderType::Sell && it->type == OrderType::Buy && order.price <= it->price) { // Аналогично для продажи
                double matchedVolume = std::min(remainingVolume, it->volume);
                UpdateBalance(it->userId, order.userId, matchedVolume, it->price);
                response += "Matched " + std::to_string(matchedVolume) + " USD at " + std::to_string(it->price) + " RUB with User " + it->userId + "\n";
                remainingVolume -= matchedVolume;
                it->volume -= matchedVolume;
                if (it->volume <= 0) {
                    it = orders.erase(it);
                } else {
                    ++it;
                }
                if (remainingVolume <= 0) {
                    break;
                }
            } else {
                ++it;
            }
        }

        // До этого места дойдёт только заявка, у которой не закончился объём, поэтому мы
        order.volume = remainingVolume;     // Обновляем объем текущего ордера на оставшийся объем
        orders.push_back(order);            // Сохраняем оставшийся активный ордер
        //Если не было найдено соответствующих ордеров, возвращаем сообщение о том, что совпадений не найдено.
        //В противном случае возвращаем сформированный ответ с информацией о выполненных сделках.
        return response.empty() ? "No matching orders found\n" : response; // результаты выполнения сопоставления ордеров.
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
