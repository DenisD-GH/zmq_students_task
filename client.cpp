#include <zmq.hpp>
#include <iostream>
#include <string>

int main() {
    // 1. Создаем контекст ZMQ
    zmq::context_t context(1);
    
    // 2. Создаем сокет типа SUB (подписчик)
    zmq::socket_t subscriber(context, ZMQ_SUB);
    
    // 3. Подключаемся к серверу
    subscriber.connect("tcp://localhost:5555");
    
    // 4. Устанавливаем фильтр подписки (пустая строка = все сообщения)
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    
    std::cout << "Клиент подключен к localhost:5555..." << std::endl;
    std::cout << "Ожидание сообщений..." << std::endl;
    
    // 5. Получаем 5 сообщений и завершаем работу
    for (int i = 0; i < 5; ++i) {
        // 6. Создаем пустое сообщение для приема
        zmq::message_t message;
        
        // 7. Ждем и получаем сообщение (блокирующий вызов)
        subscriber.recv(message, zmq::recv_flags::none);
        
        // 8. Преобразуем полученные данные в строку
        std::string received_message(static_cast<char*>(message.data()), message.size());
        
        std::cout << "Получено: " << received_message << std::endl;
    }
    
    std::cout << "Клиент завершил работу." << std::endl;
    
    return 0;
}