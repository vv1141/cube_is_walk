#include "Player.h"

Player::Player() {
  keybindProfiles.kbm = nullptr;
  keybindProfiles.controller = nullptr;
  joystick = nullptr;
  followedObject = nullptr;
}

Player::~Player() {
}

void Player::setKeybindProfiles(Input::PlayerKeybindProfiles keybindProfiles) {
  this->keybindProfiles = keybindProfiles;
}

Input::PlayerKeybindProfiles* Player::getKeybindProfiles() {
  return &keybindProfiles;
}

void Player::setJoystick(Input::Joystick* joystick) {
  this->joystick = joystick;
}

void Player::setFollowedObject(Object* object) {
  followedObject = object;
}

Object* Player::getFollowedObject() {
  return followedObject;
}

void Player::processLocalInput(Input* input) {
}
