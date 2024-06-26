#include <iostream>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

// Отправка сообщения на сервер по шаблону.
void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
std::string ProcessRegistration(tcp::socket& aSocket)
{
    std::string name;
    std::cout << "Hello! Enter your name: ";
    std::cin >> name;

    // Для регистрации Id не нужен, заполним его нулём
    SendMessage(aSocket, "0", Requests::Registration, name);
    return ReadMessage(aSocket);
}
//Проверка валидности данных для заявки
template <typename T>
T get_valid_input(const std::string& prompt) {
    T value;
    std::cout << prompt;
    std::cin >> value;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore();
        std::cout << "Invalid input\n";
        return 0;
    } else {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // отбросить оставшуюся часть входных данных
        return value;
    }
}
// Добавляем заявку
bool AddOrder(tcp::socket& s, const std::string& my_id){
    int volume;
    double price;
    std::string orderType;
    OrderType type;

    std::cout << "Enter order type (1 = buy/ 2 = sell): \n";
    std::cin >> orderType;
    if(orderType == "1")
        type = OrderType::Buy;
    else if(orderType == "2")
        type = OrderType::Sell;
    else
    {
        std::cout << "Error in selecting the type of order\n";
        return false;
    }

    volume = get_valid_input<int>("Enter volume: \n");
    if(!volume) return false;
    price = get_valid_input<double>("Enter price: \n");
    if(!price) return false;

    nlohmann::json order;
    order["Volume"] = volume;
    order["Price"] = price;
    order["TypeOrder"] = type;

    SendMessage(s, my_id, Requests::Order, order.dump());
    std::cout << "Order sent\n";
    return true;
}


int main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        // Мы предполагаем, что для идентификации пользователя будет использоваться ID.
        // Тут мы "регистрируем" пользователя - отправляем на сервер имя, а сервер возвращает нам ID.
        // Этот ID далее используется при отправке запросов.
        std::string my_id = ProcessRegistration(s); // "Создаём" пользователя, получаем его ID.

        while (true)
        {
            // Тут реализовано "бесконечное" меню.
            std::cout << "Menu:\n"
                         "1) Hello Request\n"
                         "2) Balance\n"
                         "3) Add order\n"
                         "4) Exit\n"
                         << std::endl;

            short menu_option_num;
            std::cin >> menu_option_num;
            switch (menu_option_num)
            {
                case 1:
                {
                    // Для примера того, как может выглядить взаимодействие с сервером
                    // реализован один единственный метод - Hello.
                    // Этот метод получает от сервера приветствие с именем клиента,
                    // отправляя серверу id, полученный при регистрации.
                    SendMessage(s, my_id, Requests::Hello, "");
                    std::cout << ReadMessage(s);
                    break;
                }
                case 2: // Баланс
                {
                    SendMessage(s, my_id, Requests::Balance, "");
                    std::cout << ReadMessage(s);
                    break;

                }
                case 3: // Заявка
                {
                    bool value = AddOrder(s, my_id);
                    if(value)
                        std::cout << ReadMessage(s);
                    break;
                }
                case 4: // Выход
                {
                     exit(0);
                     break;
                }
                default:
                {
                    std::cout << "Unknown menu option\n" << std::endl;
                }
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
