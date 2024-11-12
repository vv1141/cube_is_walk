#include "Camera.h"

Camera::Camera() {
  position = glm::vec3(0, 0, 0);
  viewMatrix = glm::mat4(1.0f);
  orbitCameraPosition = glm::vec3(0, 0, 0);
  followCameraObject = nullptr;
  followMode = true;
}

Camera::~Camera() {
}

void Camera::setupOrbitCamera(glm::vec3 position, float initialDistance) {
  this->orbitCameraPosition = position;
  this->orbitCameraDistance = initialDistance;
  this->orbitCameraAngleHorizontal = 0.0f;
  this->orbitCameraAngleVertical = Utility::tau * 0.125f;
}

void Camera::setupFollowCamera(Object* object, glm::vec3 targetOffset, float speed) {
  this->followCameraObject = object;
  this->followCameraTargetOffset = targetOffset;
  this->followCameraSpeed = speed;
  this->followPosition = glm::vec3(0.0f);
}

void Camera::setPosition(glm::vec3 position) {
  this->position = position;
}

glm::vec3 Camera::getPosition() {
  return position;
}
glm::mat4 Camera::getViewMatrix() {
  return viewMatrix;
}

void Camera::updatePosition(double dt, double alpha) {
  if(followMode && followCameraObject != nullptr) {
    glm::vec3   targetPosition = followCameraObject->getInterpolatedPosition(alpha) + followCameraTargetOffset;
    glm::vec3   dist = targetPosition - position;
    const float coefficient = dt * followCameraSpeed;
    position += coefficient * dist;
    if(glm::length2(targetPosition - position) < 0.00001f) {
      position = targetPosition;
    }
    targetPosition = followCameraObject->getInterpolatedPosition(alpha);
    dist = targetPosition - followPosition;
    followPosition += coefficient * dist;
    if(glm::length2(targetPosition - position) < 0.00001f) {
      followPosition = targetPosition;
    }
    this->viewMatrix = glm::lookAt(
      position,
      followPosition,
      glm::vec3(0.0f, 1.0f, 0.0f));
  } else {
    orbitCameraAngleHorizontal -= dt * 0.1f;
    glm::vec3 unitHorizontal(sin(orbitCameraAngleHorizontal), 0.0f, cos(orbitCameraAngleHorizontal));
    glm::vec3 unitRotated = glm::rotate(unitHorizontal, orbitCameraAngleVertical, glm::cross(unitHorizontal, glm::vec3(0.0f, 1.0f, 0.0f)));
    position = orbitCameraPosition + unitRotated * orbitCameraDistance;
    this->viewMatrix = glm::lookAt(
      position,
      orbitCameraPosition,
      glm::vec3(0.0f, 1.0f, 0.0f));
  }
}

void Camera::setFollowMode(bool value) {
  followMode = value;
}

bool Camera::isFollowMode() {
  return followMode;
}
