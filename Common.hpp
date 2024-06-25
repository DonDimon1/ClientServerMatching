#pragma once

#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#pragma once
#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Balance = "Bal";
    static std::string Order = "Ord";
}

enum class OrderType { // Обозначения покупки и продажи
    Buy,
    Sell
};

#endif //CLIENSERVERECN_COMMON_HPP
