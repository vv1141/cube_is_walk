#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "Debug.h"
#include "Input.h"
#include "Object.h"
#include "Utility.h"

class Camera {
private:
  glm::vec3 position;
  glm::mat4 viewMatrix;

  Input::PlayerKeybindProfiles* keybindProfiles;
  Input::Joystick*              joystick;
  float*                        fov;

  glm::vec3 orbitCameraPosition;
  float     orbitCameraInitialDistance;
  float     orbitCameraMinDistance;
  float     orbitCameraMaxDistance;
  float     orbitCameraMinAngleVertical;
  float     orbitCameraMaxAngleVertical;
  float     orbitCameraDistance;
  float     orbitCameraAngleHorizontal;
  float     orbitCameraAngleVertical;
  Object*   followCameraObject;
  glm::vec3 followCameraTargetOffset;
  float     followCameraSpeed;
  glm::vec3 followPosition;
  bool      followMode;

public:
  Camera();
  ~Camera();

  void setupOrbitCamera(glm::vec3 position, float initialDistance);
  void setupFollowCamera(Object* object, glm::vec3 targetOffset, float speed);

  void      setPosition(glm::vec3 position);
  glm::vec3 getPosition();
  glm::mat4 getViewMatrix();
  void      processInput(double dt, Input* input);
  void      updatePosition(double dt, double alpha);
  void      setFollowMode(bool value);
  bool      isFollowMode();
};

#endif
