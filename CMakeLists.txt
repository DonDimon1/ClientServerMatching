CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(ClienServerEcn)
SET(CMAKE_CXX_STANDARD 17)

set (BOOST_ROOT "D:/Coding/boost-build_1_85/") # Путь к библиотеке Boost
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
#set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_MULTITHREADED ON)
link_libraries(ws2_32 wsock32) #для подключения к проекту необходимых системных библиотек


# Найдите библиотеки Boost
find_package(Boost REQUIRED COMPONENTS system filesystem) # Добавьте необходимые компоненты Boost
FIND_PACKAGE(Threads REQUIRED)
# Убедитесь, что компоненты Boost найдены
if(Boost_FOUND)
    message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
endif()

# Подключите директории с заголовочными файлами
include_directories(${Boost_INCLUDE_DIRS})
# Добавьте исходные файлы проекта
ADD_EXECUTABLE(Server Server.cpp Common.hpp json.hpp)
ADD_EXECUTABLE(Client Client.cpp Common.hpp json.hpp)


# Линковка с библиотеками Boost

TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES})
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})




#set(Boost_INCLUDE_DIR "D:/Coding/boost-build/include/boost-1_60/")
#set(Boost_LIBRARY_DIR "D:/Coding/boost-build/")
#FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)


# Подключаем необходимые модули. Для примера подключим program_options
#find_package (Boost REQUIRED COMPONENTS program_options REQUIRED)

# Если библиотека найдена, то
#if (Boost_FOUND)
    #include_directories (SYSTEM ${Boost_INCLUDE_DIR}) # подключаем заголовочные файлы
    #add_executable (Boost_test "main.cpp") # Создаем исполняемый файл для нашего проекта
    #target_link_libraries (Boost_test ${Boost_LIBRARIES}) # подключаем библиотеку
    #message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
#endif ()

#if(Boost_FOUND)
    #INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    #LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
    #message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    #message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    #message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    #target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})
    #target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
    #message(STATUS "Found BOOST ${Boost_VERSION_STRING}")
#else()
    #message(STATUS "WEBSOCKET BOOST Not Found")
#endif()



#FIND_PACKAGE(Threads REQUIRED)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})



