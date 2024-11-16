#ifndef WORLD_H
#define WORLD_H

#include <algorithm>
#include <list>
#include <memory>
#include <vector>

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include "../c/flecs.h"

#include "Camera.h"
#include "Debug.h"
#include "Input.h"
#include "Light.h"
#include "Model.h"
#include "Player.h"
#include "RenderObject.h"
#include "Shader.h"
#include "Texture.h"
#include "Utility.h"

class World {
public:
private:
  flecs::world ecsWorld;

  // scene components and tags

  struct MenuScene {
    flecs::entity id;
  };
  struct IntermissionScene {
    flecs::entity id;
  };
  struct GameScene {
    flecs::entity id;
  };
  struct SceneRoot {};
  struct ActiveScene {};

  // general components

  struct Position {
    int x, y;
  };
  struct Animation {
    float alpha;
  };
  struct AnimationFinished {};
  struct ReverseAnimation {};
  struct Message {
    std::string text;
  };
  struct Dialogue {
    std::string text;
  };

  // menu components and tags

  struct Intermission {
    std::string text;
  };
  struct Selection {
    std::string text;
    std::string secondaryText;
  };
  struct State {
    int value;
  };
  struct PreviousState {
    int value;
  };
  struct ContinueDisabled {};

  // game object components

  struct GameObject {
    int type;
  };
  struct RenderObjectHandle {
    RenderObject* renderObject;
  };
  struct Elevation {
    int value;
  };
  struct MovementDirection {
    int x, y;
  };
  struct Orientation {
    glm::quat quaternion;
  };
  struct Carrying {
    flecs::entity id;
  };
  struct CarriedBy {
    flecs::entity id;
  };

  // game object tags

  struct Grass {};
  struct Grid {};
  struct Water {};
  struct Cube {};
  struct Eyes {};
  struct Goal {};
  struct Boulder {};
  struct Key {};
  struct Lock {};
  struct Helper {};

  struct Walkable {};
  struct Supporting {};
  struct Collectable {};
  struct Blocking {};
  struct Pushable {};

  struct Interrupted {};
  struct Moving {};
  struct Stationary {};
  struct Blink {};
  struct Peer {};

  // enums

  enum class TileType {
    cube = 0,
    grass,
    grid,
    water,
    boulder,
    key,
    lock,
    goal,
    helper,
    LAST_ENTRY
  };

  // handles

  static World*        world;
  static flecs::entity menuState;
  static flecs::entity intermission;
  static flecs::entity blurAnimation;
  static flecs::entity cube;
  static flecs::entity helper;
  static flecs::system elevationSystem;
  static flecs::system animationSystem;
  static flecs::system eyeAnimationSystem;
  static flecs::system eyeBlinkAnimationSystem;
  static flecs::system eyePeerAnimationSystem;
  static flecs::system dynamicUpdaterSystem;
  static flecs::system staticUpdaterSystem;
  static flecs::system eyeUpdaterSystem;
  static flecs::system menuRenderSystem;
  static flecs::system intermissionRenderSystem;
  static flecs::system messageRenderSystem;
  static flecs::system dialogueRenderSystem;
  static flecs::system editorRenderSystem;

  // queries

  flecs::query<Selection, State>                 querySelections;
  flecs::query<RenderObjectHandle>               queryRenderObjectHandles;
  flecs::query<GameObject, Position>             queryGameObjects;
  flecs::query<Walkable, Position, Elevation>    queryWalkable;
  flecs::query<Blocking, Position>               queryBlocking;
  flecs::query<Pushable, Position, Elevation>    queryPushable;
  flecs::query<Supporting, Position>             querySupporting;
  flecs::query<Collectable, Position, Elevation> queryCollectable;
  flecs::query<Lock, Position>                   queryLock;
  flecs::query<Goal, Position>                   queryGoal;
  flecs::query<Key, Position>                    queryKey;
  flecs::query<Helper, Position>                 queryHelper;

  // window, filesystem, input, and rendering

  int   depthMapResolution;
  int   shadowLevelCount;
  float zMult;
  float zBias;
  float fov;
  float nearClippingPlane;
  float farClippingPlane;
  bool  multisamplingEnabled;
  int   multisamplingSampleCount;

  sf::RenderWindow* renderWindow;
  Input*            input;
  sf::Vector2i      mousePosition;
  Camera            camera;

  int maxBlurIterations;

  glm::vec3 followCameraTargetOffset;
  float     followCameraSpeed;

  struct HelperData {
    std::string assetName;
    std::string displayName;
    sf::Color   colour;
  };

  std::string              levelDirectory;
  std::string              saveFile;
  std::vector<std::string> levelFiles;
  std::vector<std::string> levelNames;
  std::vector<std::string> helpDialogue;
  std::vector<int>         levelHelpers;
  std::vector<HelperData>  helpers;
  int                      currentLevel;
  int                      tileSize;

  GLuint              vertexArray;
  OpaqueShader        opaqueShader;
  ShadowMappingShader shadowMappingShader;
  BlurShader          blurShader;
  ScreenShader        screenShader;

  Player                                    player;
  Object*                                   cubeObject;
  Object*                                   helperObject;
  std::list<RenderObject::ModelTexturePair> modelTexturePairs;
  std::list<RenderObject>                   renderObjects;
  std::list<Object>                         objects;
  std::map<std::string, Model>              models;
  std::map<std::string, Texture>            textures;

  glm::vec3                    lightPosition;
  glm::mat4                    projectionViewMatrix;
  Input::PlayerKeybindProfiles testProfiles;

  // game world and scenes

  static void   resetScene(flecs::world& ecs);
  static void   resetGameScene(flecs::world& ecs);
  static void   initMenuScene(flecs::iter& it, size_t, ActiveScene);
  static void   initIntermissionScene(flecs::iter& it, size_t, ActiveScene);
  static void   initGameScene(flecs::iter& it, size_t, ActiveScene);
  void          initQueries();
  void          initScenes();
  flecs::entity addGameObject(int type, int x, int y);
  void          deleteRenderObject(flecs::entity& e);
  void          initSystems();

  // window, filesystem, input, and rendering

  void addRenderObject(Model* model, Texture* texture, Object* object);
  void deleteObject(RenderObject* renderObject);
  bool writeSaveFile(std::string path);
  int  readSaveFile(std::string path);
  bool writeLevelFile(std::string path);
  bool readLevelFile(std::string path);
  bool readAssetFile(std::string path, bool readShaders);

public:
  World();
  ~World();

  bool init(sf::RenderWindow* renderWindow, Input* input);
  bool update(double dt);
  void updateCameraPosition(double dt, double alpha);
  void renderGeometry(double alpha);
  void renderUi(int fps);
};

#endif
