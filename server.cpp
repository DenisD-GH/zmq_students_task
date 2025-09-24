#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main() {
    // 1. Создаем контекст ZMQ
    zmq::context_t context(1);
    
    // 2. Создаем сокет типа PUB (издатель)
    zmq::socket_t publisher(context, ZMQ_PUB);
    
    // 3. Привязываем сокет к адресу (порт 5555)
    publisher.bind("tcp://*:5555");
    
    std::cout << "Сервер запущен на порту 5555..." << std::endl;
    
    // 4. Даем время клиентам подключиться
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    int message_count = 0;
    
    // 5. Бесконечный цикл отправки сообщений
    while (true) {
        // Создаем простое сообщение
        std::string message = "Привет от сервера! Сообщение #" + std::to_string(message_count++);
        
        // 6. Создаем ZMQ сообщение
        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.c_str(), message.size());
        
        // 7. Отправляем сообщение
        publisher.send(zmq_message, zmq::send_flags::none);
        
        std::cout << "Отправлено: " << message << std::endl;
        
        // 8. Ждем 2 секунды перед следующей отправкой
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    return 0;
}