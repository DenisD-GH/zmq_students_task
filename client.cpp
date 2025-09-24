#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

struct Student {
    std::vector<int> ids;
    std::string firstName;
    std::string lastName;
    std::string birthDate;
    
    bool operator<(const Student& other) const {
        if (lastName != other.lastName) {
            return lastName < other.lastName;
        }
        return firstName < other.firstName;
    }
};

std::vector<Student> parseStudents(const std::string& data) {
    std::vector<Student> students;
    
    if (data.find("STUDENTS_START") == std::string::npos) {
        std::cout << "Получено не студенческое сообщение" << std::endl;
        return students;
    }
    
    size_t start_pos = data.find("STUDENTS_START");
    size_t end_pos = data.find("STUDENTS_END");
    
    if (start_pos == std::string::npos || end_pos == std::string::npos) {
        std::cout << "Ошибка формата сообщения" << std::endl;
        return students;
    }
    
    start_pos += 14;
    std::string students_data = data.substr(start_pos, end_pos - start_pos - 1);
    
    size_t count_pos = students_data.find('|');
    if (count_pos != std::string::npos) {
        std::string count_str = students_data.substr(0, count_pos);
        try {
            int expected_count = std::stoi(count_str);
            std::cout << "Ожидается " << expected_count << " студентов" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Ошибка парсинга количества студентов: " << e.what() << std::endl;
        }
        students_data = students_data.substr(count_pos + 1);
    }
    
    size_t pos = 0;
    while ((pos = students_data.find(';')) != std::string::npos) {
        std::string student_str = students_data.substr(0, pos);
        students_data.erase(0, pos + 1);
        
        Student student;
        size_t pipe1 = student_str.find('|');
        size_t pipe2 = student_str.find('|', pipe1 + 1);
        size_t pipe3 = student_str.find('|', pipe2 + 1);
        
        if (pipe1 != std::string::npos && pipe2 != std::string::npos && pipe3 != std::string::npos) {
            std::string ids_str = student_str.substr(0, pipe1);
            size_t id_pos = 0;
            while ((id_pos = ids_str.find(',')) != std::string::npos) {
                try {
                    int id = std::stoi(ids_str.substr(0, id_pos));
                    student.ids.push_back(id);
                } catch (const std::exception& e) {
                    std::cout << "Ошибка парсинга ID: " << e.what() << std::endl;
                }
                ids_str.erase(0, id_pos + 1);
            }
            if (!ids_str.empty()) {
                try {
                    student.ids.push_back(std::stoi(ids_str));
                } catch (const std::exception& e) {
                    std::cout << "Ошибка парсинга ID: " << e.what() << std::endl;
                }
            }
            
            student.firstName = student_str.substr(pipe1 + 1, pipe2 - pipe1 - 1);
            student.lastName = student_str.substr(pipe2 + 1, pipe3 - pipe2 - 1);
            student.birthDate = student_str.substr(pipe3 + 1);
            
            students.push_back(student);
        }
    }
    
    if (!students_data.empty()) {
        Student student;
        size_t pipe1 = students_data.find('|');
        size_t pipe2 = students_data.find('|', pipe1 + 1);
        size_t pipe3 = students_data.find('|', pipe2 + 1);
        
        if (pipe1 != std::string::npos && pipe2 != std::string::npos && pipe3 != std::string::npos) {
            std::string ids_str = students_data.substr(0, pipe1);
            size_t id_pos = 0;
            while ((id_pos = ids_str.find(',')) != std::string::npos) {
                try {
                    int id = std::stoi(ids_str.substr(0, id_pos));
                    student.ids.push_back(id);
                } catch (const std::exception& e) {
                    std::cout << "Ошибка парсинга ID: " << e.what() << std::endl;
                }
                ids_str.erase(0, id_pos + 1);
            }
            if (!ids_str.empty()) {
                try {
                    student.ids.push_back(std::stoi(ids_str));
                } catch (const std::exception& e) {
                    std::cout << "Ошибка парсинга ID: " << e.what() << std::endl;
                }
            }
            
            student.firstName = students_data.substr(pipe1 + 1, pipe2 - pipe1 - 1);
            student.lastName = students_data.substr(pipe2 + 1, pipe3 - pipe2 - 1);
            student.birthDate = students_data.substr(pipe3 + 1);
            
            students.push_back(student);
        }
    }
    
    return students;
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5555");
    
    subscriber.set(zmq::sockopt::subscribe, "");
    
    std::cout << "Клиент подключен к localhost:5555..." << std::endl;
    std::cout << "Ожидание данных о студентах..." << std::endl;
    
    zmq::message_t message;
    subscriber.recv(message, zmq::recv_flags::none);
    
    std::string received_data(static_cast<char*>(message.data()), message.size());
    
    auto students = parseStudents(received_data);
    
    std::cout << "Получено " << students.size() << " студентов" << std::endl;
    
    std::sort(students.begin(), students.end());
    
    std::cout << "\n=== ОТСОРТИРОВАННЫЙ СПИСОК СТУДЕНТОВ ===" << std::endl;
    for (const auto& student : students) {
        std::cout << "ID: ";
        for (size_t i = 0; i < student.ids.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << student.ids[i];
        }
        std::cout << " | " << student.firstName << " " << student.lastName << " | " << student.birthDate << std::endl;
    }
    
    std::cout << "\nКлиент завершил работу." << std::endl;
    
    return 0;
}