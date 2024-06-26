#ifndef CORE_HPP
#define CORE_HPP

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "json.hpp"
#include "Common.hpp"
#include <vector>

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
    Core(const Core&) = delete;                         // Защита от копирования
    Core(Core&&) noexcept = delete;                     // Защита от перемещения
    Core& operator=(const Core&) = delete;              // Защита от оператора присваивания
    Core& operator=(const Core&&) noexcept = delete;    // Защита от оператора перемещения
    //static Core& GetCoreInstance();
    static Core* GetCoreInstance();

    std::string RegisterNewUser(const std::string& aUserName);
    std::string GetUserName_(const std::string& aUserId);
    std::string GetBalance(const std::string& userId);
    std::string AddOrder(const std::string& userId, double volume, double price, OrderType type);

private:
    Core() = default;
    static Core* instancePtr;
    std::map<size_t, std::string> mUsers;                   // Храним пользователей: ключ-id, значение-имя
    std::map<size_t, std::pair<double, double>> balance;    // Баланс пользователя: ключ-id, Значение <USD, RUB>
    std::vector<Order> buyOrders;                           // Все заявки на покупку
    std::vector<Order> sellOrders;                          // Все заявки на продажу

    void UpdateBalance(const std::string& buyerId, const std::string& sellerId, double volume, double price);
    std::string MatchOrder(Order& order);
};

#endif // CORE_HPP
