#include <zmq.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>


struct Student {
    std::vector<int> ids;
    std::string firstName;
    std::string lastName;
    std::string birthDate;
    
    bool isSame(const Student& other) const {
        return firstName == other.firstName && lastName == other.lastName && birthDate == other.birthDate;
    }
};

std::vector<Student> readStudentFile(const std::string& filename) {
    std::vector<Student> students;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Нельзя открыть файл: " << filename << std::endl;
        return students;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        Student student;
        
        size_t pos1 = line.find(' ');
        size_t pos2 = line.find(' ', pos1 + 1);
        size_t pos3 = line.find(' ', pos2 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            int id = std::stoi(line.substr(0, pos1));
            student.ids.push_back(id);
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

std::vector<Student> mergeStudents(const std::vector<Student>& students1, const std::vector<Student>& students2) {
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

void printStudents(const std::vector<Student>& students, const std::string& title) {
    std::cout << "\n" << title << ":" << std::endl;
    for (const auto& student : students) {
        std::cout << "ID: ";
        for (size_t i = 0; i < student.ids.size(); i++) {
            if (i > 0) std::cout << ",";
            std::cout << student.ids[i];
        }
        std::cout << " | " << student.firstName << " " << student.lastName << " " << student.birthDate << std::endl;
    }
}

int main() {
    std::cout << "Чтение файлов со студентами..." << std::endl;
    auto students1 = readStudentFile("student_file_1.txt");
    auto students2 = readStudentFile("student_file_2.txt");
    
    printStudents(students1, "Студенты из файла 1");
    printStudents(students2, "Студенты из файла 2");
    
    std::cout << "\nИтого в файле 1: " << students1.size() << " студентов" << std::endl;
    std::cout << "Итого в файле 2: " << students2.size() << " студентов" << std::endl;
    
    std::cout << "\nОбъединяем студентов..." << std::endl;
    auto allStudents = mergeStudents(students1, students2);
    
    printStudents(allStudents, "ОБЪЕДИНЕННЫЙ СПИСОК СТУДЕНТОВ");
    std::cout << "\nВсего уникальных студентов после объединения: " << allStudents.size() << std::endl;
    
    std::cout << "\nЗапуск ZMQ сервера..." << std::endl;
    
    zmq::context_t context(1);
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.bind("tcp://*:5555");
    
    std::cout << "Сервер запущен на порту 5555..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    int message_count = 0;
    
    while (true) {
        std::string message = "Уникальных студентов: " + std::to_string(allStudents.size()) + " | Сообщение: " + std::to_string(message_count++);
        
        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.c_str(), message.size());
        publisher.send(zmq_message, zmq::send_flags::none);
        
        std::cout << "Отправлено: " << message << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    return 0;
}