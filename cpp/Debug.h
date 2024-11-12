#ifndef DEBUG_H
#define DEBUG_H

#include <glm/glm.hpp>
#include <iostream>

#include "Utility.h"

class Debug {
private:
public:
  static void log(int value);
  static void log(double value);
  static void log(glm::vec3 value);
  static void log(std::string value);
  static void log(void* value);
  static void log(std::string message, int value);
  static void log(std::string message, double value);
  static void log(std::string message, glm::vec3 value);
  static void log(std::string message, std::string value);
  static void log(std::string message, void* value);
};

#endif
