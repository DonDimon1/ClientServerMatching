CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(ClienServerEcn)
SET(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_MULTITHREADED ON)

FIND_PACKAGE(Boost COMPONENTS system REQUIRED)

#Проверка нахождения Boost
if(Boost_FOUND)
    message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
else()
    set(BOOST_ROOT "Path/to/Boost") # Путь к библиотеке Boost вручную
    set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
    set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
    message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
endif()

#для подключения к проекту необходимых системных библиотек windows
if (WIN32)
    link_libraries(ws2_32 wsock32)
endif()

# Найдите библиотеки Boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem) # Добавьте необходимые компоненты Boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS unit_test_framework) # Для теста
FIND_PACKAGE(Threads REQUIRED)
# Убедитесь, что компоненты Boost найдены
if(Boost_FOUND)
    message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
endif()

# Подключите директории с заголовочными файлами
include_directories(${Boost_INCLUDE_DIRS})
# Добавьте исходные файлы проекта
ADD_EXECUTABLE(Server Server.cpp Common.hpp json.hpp Core.hpp Core.cpp)
ADD_EXECUTABLE(Client Client.cpp Common.hpp json.hpp)
ADD_EXECUTABLE(Tests Tests.cpp Core.hpp Core.cpp)

# Линковка с библиотеками Boost
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(Tests PRIVATE Threads::Threads ${Boost_LIBRARIES})

# Добавление тестов
enable_testing()
add_test(NAME MyTests COMMAND Tests)






