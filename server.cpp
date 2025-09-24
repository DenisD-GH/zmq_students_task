#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>

struct Student {
    int id;
    std::string firstName;
    std::string lastName;
    std::string birthDate;
};

std::vector<Student> readStudentFile(const std::string& filename) {
    std::vector<Student> students;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Не могу открыть файл: " << filename << std::endl;
        return students;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        Student student;
        
        size_t pos1 = line.find(' ');
        size_t pos2 = line.find(' ', pos1 + 1);
        size_t pos3 = line.find(' ', pos2 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            student.id = std::stoi(line.substr(0, pos1));
            student.firstName = line.substr(pos1 + 1, pos2 - pos1 - 1);
            student.lastName = line.substr(pos2 + 1, pos3 - pos2 - 1);
            student.birthDate = line.substr(pos3 + 1);
            
            students.push_back(student);
        } else {
            std::cout << "Ошибка в строке: " << line << std::endl;
        }
    }
    
    return students;
}

int main() {
    std::cout << "Чтение student_file_1.txt:" << std::endl;
    auto students1 = readStudentFile("student_file_1.txt");
    for (const auto& s : students1) {
        std::cout << s.id << " " << s.firstName << " " << s.lastName << " " << s.birthDate << std::endl;
    }
    
    std::cout << "\nЧтение student_file_2.txt:" << std::endl;
    auto students2 = readStudentFile("student_file_2.txt");
    for (const auto& s : students2) {
        std::cout << s.id << " " << s.firstName << " " << s.lastName << " " << s.birthDate << std::endl;
    }
    
    std::cout << "\nВсего студентов: " << students1.size() + students2.size() << std::endl;
    
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");
    
    std::cout << "Сервер запущен на порту 5555..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    int message_count = 0;
    
    while (true) {
        std::string message = "Студентов: " + std::to_string(students1.size() + students2.size()) + 
                             " | Сообщение: " + std::to_string(message_count++);
        
        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.c_str(), message.size());
        publisher.send(zmq_message, zmq::send_flags::none);
        
        std::cout << "Отправлено: " << message << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    return 0;
}