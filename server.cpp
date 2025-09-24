#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <sstream>

struct Student {
    std::vector<int> ids;
    std::string firstName;
    std::string lastName;
    std::string birthDate;
    
    bool isSame(const Student& other) const {
        return firstName == other.firstName && 
               lastName == other.lastName && 
               birthDate == other.birthDate;
    }
    
    std::string toString() const {
        std::stringstream ss;
        for (size_t i = 0; i < ids.size(); i++) {
            if (i > 0) ss << ",";
            ss << ids[i];
        }
        ss << "|" << firstName << "|" << lastName << "|" << birthDate;
        return ss.str();
    }
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
        // Пропускаем пустые строки
        if (line.empty()) continue;
        
        Student student;
        
        size_t pos1 = line.find(' ');
        size_t pos2 = line.find(' ', pos1 + 1);
        size_t pos3 = line.find(' ', pos2 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            try {
                int id = std::stoi(line.substr(0, pos1));
                student.ids.push_back(id);
                student.firstName = line.substr(pos1 + 1, pos2 - pos1 - 1);
                student.lastName = line.substr(pos2 + 1, pos3 - pos2 - 1);
                student.birthDate = line.substr(pos3 + 1);
                
                students.push_back(student);
            } catch (const std::exception& e) {
                std::cout << "Ошибка парсинга строки: " << line << " - " << e.what() << std::endl;
            }
        } else {
            std::cout << "Ошибка в строке: " << line << std::endl;
        }
    }
    
    return students;
}

std::vector<Student> mergeStudents(const std::vector<Student>& students1, 
                                  const std::vector<Student>& students2) {
    std::vector<Student> merged = students1;
    
    for (const auto& student2 : students2) {
        bool found = false;
        
        for (auto& student1 : merged) {
            if (student1.isSame(student2)) {
                student1.ids.insert(student1.ids.end(), 
                                  student2.ids.begin(), student2.ids.end());
                found = true;
                break;
            }
        }
        
        if (!found) {
            merged.push_back(student2);
        }
    }
    
    return merged;
}

int main() {
    auto students1 = readStudentFile("student_file_1.txt");
    auto students2 = readStudentFile("student_file_2.txt");
    auto allStudents = mergeStudents(students1, students2);
    
    std::cout << "Сервер: прочитано " << allStudents.size() << " уникальных студентов" << std::endl;
    
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");
    
    std::cout << "Сервер запущен на порту 5555..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    int send_count = 0;
    
    while (true) {
        std::stringstream message;
        
        message << "STUDENTS_START|" << allStudents.size() << "|";
        
        for (size_t i = 0; i < allStudents.size(); i++) {
            if (i > 0) message << ";";
            message << allStudents[i].toString();
        }
        
        message << "|STUDENTS_END";
        
        std::string message_str = message.str();
        zmq::message_t zmq_message(message_str.size());
        memcpy(zmq_message.data(), message_str.c_str(), message_str.size());
        
        publisher.send(zmq_message, zmq::send_flags::none);
        
        std::cout << "Отправлено " << allStudents.size() << " студентов (пакет #" << send_count++ << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    return 0;
}