#ifndef PLAYER_H
#define PLAYER_H

#include "Input.h"
#include "Object.h"

class Player {
private:
  Input::PlayerKeybindProfiles keybindProfiles;
  Input::Joystick*             joystick;
  Object*                      followedObject;

public:
  Player();
  ~Player();

  void                          setKeybindProfiles(Input::PlayerKeybindProfiles keybindProfiles);
  Input::PlayerKeybindProfiles* getKeybindProfiles();
  void                          setJoystick(Input::Joystick* joystick);
  void                          setFollowedObject(Object* object);
  Object*                       getFollowedObject();

  void processLocalInput(Input* input);
};

#endif
