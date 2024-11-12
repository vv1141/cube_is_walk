
#include "Debug.h"

void Debug::log(int value) {
  std::cout << std::to_string(value) << std::endl;
}

void Debug::log(double value) {
  std::cout << std::to_string(value) << std::endl;
}

void Debug::log(glm::vec3 value) {
  std::cout << std::to_string(value.x) << ", " << std::to_string(value.y) << ", " << std::to_string(value.z) << std::endl;
}

void Debug::log(std::string value) {
  std::cout << value << std::endl;
}

void Debug::log(void* value) {
  std::cout << value << std::endl;
}

void Debug::log(std::string message, int value) {
  std::cout << message << std::to_string(value) << std::endl;
}

void Debug::log(std::string message, double value) {
  std::cout << message << std::to_string(value) << std::endl;
}

void Debug::log(std::string message, glm::vec3 value) {
  std::cout << message << std::to_string(value.x) << ", " << std::to_string(value.y) << ", " << std::to_string(value.z) << std::endl;
}

void Debug::log(std::string message, std::string value) {
  std::cout << message << value << std::endl;
}

void Debug::log(std::string message, void* value) {
  std::cout << message << value << std::endl;
}
