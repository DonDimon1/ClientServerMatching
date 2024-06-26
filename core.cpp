#include "Core.hpp"
Core* Core::instancePtr = nullptr; // Определение статической переменной

// Получить единственный экземпляр класса
Core* Core::GetCoreInstance() {
    if(instancePtr == nullptr) {
        instancePtr = new Core();
        return instancePtr;
    }
    else{
        return instancePtr;
    }
};
// "Регистрирует" нового пользователя и возвращает его ID.
std::string Core::RegisterNewUser(const std::string& aUserName)
{
    size_t newUserId = Core::mUsers.size();
    mUsers[newUserId] = aUserName;  // сохраняет пользователя в map
    balance[newUserId] = {0.0, 0.0}; // изначальный баланс пользователя(хардкод потому что пока нет БД для хранения)

    return std::to_string(newUserId); //Возвращает его id
}
// Запрос имени клиента по ID
std::string Core::GetUserName_(const std::string& aUserId)
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
std::string Core::GetBalance(const std::string& userId) {
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
std::string Core::AddOrder(const std::string& userId, double volume, double price, OrderType type) {
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
// Обновляем баланс после сделки
void Core::UpdateBalance(const std::string& buyerId, const std::string& sellerId, double volume, double price) {
    balance[std::stoi(buyerId)].first += volume;            // Увеличиваем количество USD у покупателя
    balance[std::stoi(buyerId)].second -= volume * price;   // Уменьшаем количество RUB у покупателя
    balance[std::stoi(sellerId)].first -= volume;           // Уменьшаем количество USD у продавца
    balance[std::stoi(sellerId)].second += volume * price;  // Увеличиваем количество RUB у продавца
}
// Обработка нового запроса
std::string Core::MatchOrder(Order& order) {
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








