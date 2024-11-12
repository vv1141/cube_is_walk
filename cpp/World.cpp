
#include "World.h"

#include <iostream>

World*        World::world;
flecs::entity World::menuState = flecs::entity::null();
flecs::entity World::intermission = flecs::entity::null();
flecs::entity World::blurAnimation = flecs::entity::null();
flecs::entity World::cube = flecs::entity::null();
flecs::entity World::helper = flecs::entity::null();
flecs::system World::elevationSystem;
flecs::system World::animationSystem;
flecs::system World::eyeAnimationSystem;
flecs::system World::eyeBlinkAnimationSystem;
flecs::system World::eyePeerAnimationSystem;
flecs::system World::dynamicUpdaterSystem;
flecs::system World::staticUpdaterSystem;
flecs::system World::eyeUpdaterSystem;
flecs::system World::menuRenderSystem;
flecs::system World::intermissionRenderSystem;
flecs::system World::messageRenderSystem;
flecs::system World::dialogueRenderSystem;

void World::resetScene(flecs::world& ecs) {
  Debug::log("resetScene");
  world->queryRenderObjectHandles.each([&](flecs::entity e, RenderObjectHandle r) {
    world->deleteRenderObject(e);
  });
  ecs.delete_with(flecs::ChildOf, ecs.entity<SceneRoot>());
  cube = flecs::entity::null();
  helper = flecs::entity::null();
}

void World::initMenuScene(flecs::iter& it, size_t, ActiveScene) {
  Debug::log("initMenuScene");
  flecs::world ecs = it.world();
  ecs.set_pipeline(ecs.get<MenuScene>()->id);
  resetScene(ecs);

  bool saveFileExists = Utility::checkFile(world->saveFile);
  int  readLevel = 0;
  if(saveFileExists) {
    readLevel = world->readSaveFile(world->saveFile);
  }
  if(readLevel < world->levelFiles.size()) {
    world->readLevelFile(world->levelFiles.at(readLevel));
  } else {
    world->readLevelFile(world->levelFiles.at(0));
  }
  world->camera.setupOrbitCamera(glm::vec3(20.0f, 0.0f, 10.0f), 5.0f);
  world->camera.setFollowMode(false);

  int x = world->renderWindow->getSize().x * 0.15f;
  int y = world->renderWindow->getSize().y * 0.5f;

  ecs.entity()
    .set(Selection{"New game", saveFileExists ? "|  Existing save will be overwritten" : "|  There are 5 levels in total"})
    .set(State{0})
    .set(Position{x, y})
    .child_of(ecs.component<SceneRoot>());

  ecs.entity()
    .set(Selection{"Continue game", saveFileExists ? "|  " + world->levelNames.at(readLevel) : "|  No existing save found"})
    .set(State{1})
    .set(Position{x, y + 100})
    .child_of(ecs.component<SceneRoot>());

  ecs.entity()
    .set(Selection{"Quit", "|  Thanks for playing!"})
    .set(State{2})
    .set(Position{x, y + 200})
    .child_of(ecs.component<SceneRoot>());

  menuState = ecs.entity()
                .set(State{0})
                .set(PreviousState{0})
                .set(Animation{-1.0f})
                .child_of(ecs.component<SceneRoot>());

  if(!saveFileExists || readLevel == world->levelNames.size() - 1) {
    menuState.add<ContinueDisabled>();
  }

  blurAnimation = ecs.entity()
                    .set(Animation{0.0f})
                    .child_of(ecs.component<SceneRoot>());
}

void World::initIntermissionScene(flecs::iter& it, size_t, ActiveScene) {
  Debug::log("initIntermissionScene");
  flecs::world ecs = it.world();
  ecs.set_pipeline(ecs.get<IntermissionScene>()->id);

  resetScene(ecs);

  intermission = ecs.entity()
                   .set(Intermission{world->levelNames.at(world->currentLevel)})
                   .set(Animation{0.0f})
                   .child_of(ecs.component<SceneRoot>());
}

void World::initGameScene(flecs::iter& it, size_t, ActiveScene) {
  Debug::log("initGameScene");
  flecs::world ecs = it.world();
  ecs.set_pipeline(ecs.get<GameScene>()->id);
  resetScene(ecs);

  world->readLevelFile(world->levelFiles.at(world->currentLevel));

  world->camera.setupFollowCamera(world->cubeObject, world->followCameraTargetOffset, world->followCameraSpeed);
  world->camera.setFollowMode(true);
  world->camera.setPosition(glm::vec3(0.0f, 100.0f, 0.0f));

  blurAnimation = ecs.entity()
                    .set(Animation{0.0f})
                    .add<ReverseAnimation>()
                    .child_of(ecs.component<SceneRoot>());
}

void World::initScenes() {
  ecsWorld.component<ActiveScene>()
    .add(flecs::Exclusive);

  flecs::entity menu = ecsWorld.pipeline()
                         .with(flecs::System)
                         .without<IntermissionScene>()
                         .without<GameScene>()
                         .build();

  flecs::entity intermission = ecsWorld.pipeline()
                                 .with(flecs::System)
                                 .without<MenuScene>()
                                 .without<GameScene>()
                                 .build();

  flecs::entity game = ecsWorld.pipeline()
                         .with(flecs::System)
                         .without<MenuScene>()
                         .without<IntermissionScene>()
                         .build();

  ecsWorld.set<MenuScene>({menu});
  ecsWorld.set<IntermissionScene>({intermission});
  ecsWorld.set<GameScene>({game});

  ecsWorld.observer<ActiveScene>("Scene Change to Menu")
    .event(flecs::OnAdd)
    .second<MenuScene>()
    .each(initMenuScene);

  ecsWorld.observer<ActiveScene>("Scene Change to Intermission")
    .event(flecs::OnAdd)
    .second<IntermissionScene>()
    .each(initIntermissionScene);

  ecsWorld.observer<ActiveScene>("Scene Change to Game")
    .event(flecs::OnAdd)
    .second<GameScene>()
    .each(initGameScene);
}

void World::initQueries() {
  querySelections = ecsWorld.query<Selection, State>();
  queryRenderObjectHandles = ecsWorld.query<RenderObjectHandle>();
  queryGameObjects = ecsWorld.query<GameObject, Position>();
  queryWalkable = ecsWorld.query<Walkable, Position, Elevation>();
  queryBlocking = ecsWorld.query<Blocking, Position>();
  queryPushable = ecsWorld.query<Pushable, Position, Elevation>();
  querySupporting = ecsWorld.query<Supporting, Position>();
  queryCollectable = ecsWorld.query<Collectable, Position, Elevation>();
  queryLock = ecsWorld.query<Lock, Position>();
  queryGoal = ecsWorld.query<Goal, Position>();
  queryKey = ecsWorld.query<Key, Position>();
  queryHelper = ecsWorld.query<Helper, Position>();
}

flecs::entity World::addGameObject(int type, int x, int y) {
  flecs::entity newEntity = ecsWorld.entity()
                              .child_of(ecsWorld.component<SceneRoot>())
                              .set(GameObject{type})
                              .set(Position{x, y})
                              .set(Elevation{0})
                              .add<Moving>();
  objects.emplace_back();
  objects.back().setPosition(glm::vec3(x, 0.0f, y));
  if(type == (int)TileType::grass) {
    addRenderObject(&(grassModel), &(grassTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Grass>();
    newEntity.add<Walkable>();
  } else if(type == (int)TileType::grid) {
    addRenderObject(&(gridModel), &(gridTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Grid>();
    newEntity.add<Walkable>();
    newEntity.add<Supporting>();
  } else if(type == (int)TileType::water) {
    addRenderObject(&(waterModel), &(waterTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Water>();
  } else if(type == (int)TileType::cube) {
    cube = newEntity;
    cubeObject = &(objects.back());
    cubeObject->setOrientationFromDirection(glm::vec3(0.0f, 0.0f, -1.0f));
    addRenderObject(&(cubeModel), &(cubeTexture), cubeObject);
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Cube>();
    newEntity.set(Carrying{flecs::entity::null()});
    newEntity.set(Animation{0.0f});
    newEntity.set(MovementDirection{0, 0});
    newEntity.set<Moving, Orientation>({cubeObject->getOrientation()});
    newEntity.set<Stationary, Orientation>({cubeObject->getOrientation()});

    objects.emplace_back();
    objects.back().setPosition(glm::vec3(x, 0.0f, y));
    addRenderObject(&(eyesModel), &(eyesTexture), &(objects.back()));
    ecsWorld.entity()
      .set(RenderObjectHandle{&(renderObjects.back())})
      .add<Eyes>()
      .child_of(cube)
      .set<Blink, Animation>({0.0f})
      .set<Peer, Animation>({0.0f});
  } else if(type == (int)TileType::goal) {
    addRenderObject(&(goalModel), &(goalTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Goal>();
    newEntity.set(Carrying{flecs::entity::null()});
    newEntity.add<Walkable>();
  } else if(type == (int)TileType::boulder) {
    addRenderObject(&(boulderModel), &(boulderTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Boulder>();
    newEntity.add<Blocking>();
    newEntity.add<Pushable>();
  } else if(type == (int)TileType::key) {
    addRenderObject(&(keyModel), &(keyTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Key>();
    newEntity.set(Animation{0.0f});
    newEntity.set(CarriedBy{flecs::entity::null()});
    newEntity.add<Blocking>();
    newEntity.add<Collectable>();
  } else if(type == (int)TileType::lock) {
    addRenderObject(&(lockModel), &(lockTexture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.add<Lock>();
    newEntity.set(Carrying{flecs::entity::null()});
    newEntity.add<Blocking>();
    newEntity.add<Pushable>();
  } else if(type == (int)TileType::helper) {
    helper = newEntity;
    helperObject = &(objects.back());
    helperObject->setOrientationFromDirection(glm::vec3(0.0f, 0.0f, -1.0f));
    int helperIndex = levelHelpers.at(currentLevel);
    addRenderObject(&(helpers.at(helperIndex).model), &(helpers.at(helperIndex).texture), &(objects.back()));
    newEntity.set(RenderObjectHandle{&(renderObjects.back())});
    newEntity.set(Dialogue{helpDialogue.at(currentLevel)});
    newEntity.add<Helper>();
    newEntity.add<Blocking>();
    newEntity.set(Animation{0.0f});
    newEntity.set(MovementDirection{0, 0});
    newEntity.set<Moving, Orientation>({helperObject->getOrientation()});
    newEntity.set<Stationary, Orientation>({helperObject->getOrientation()});

    objects.emplace_back();
    objects.back().setPosition(glm::vec3(x, 0.0f, y));
    addRenderObject(&(eyesModel), &(eyesTexture), &(objects.back()));
    ecsWorld.entity()
      .set(RenderObjectHandle{&(renderObjects.back())})
      .add<Eyes>()
      .child_of(helper)
      .set<Blink, Animation>({0.7f})
      .set<Peer, Animation>({0.7f});
  }
  return newEntity;
}

void World::deleteRenderObject(flecs::entity& e) {
  deleteObject(e.get<RenderObjectHandle>()->renderObject);
}

void World::initSystems() {
  elevationSystem = ecsWorld.system<const Position, Elevation>() // update elevation of game objects
                      .with<GameObject>()
                      .with<Moving>()
                      .kind<GameScene>()
                      .each([&](flecs::entity e, const Position& p, Elevation& el) {
                        flecs::entity walkable = queryWalkable.find([&e, &p, &el](flecs::entity e2, Walkable w, Position& p2, Elevation& el2) {
                          return e.view().id() != e2.view().id() && p.x == p2.x && p.y == p2.y && el.value == el2.value;
                        });
                        if(walkable) {
                          e.set<Elevation>({el.value + 1});
                        } else {
                          walkable = queryWalkable.find([&e, &p, &el](flecs::entity e2, Walkable w, Position& p2, Elevation& el2) {
                            return e.view().id() != e2.view().id() && p.x == p2.x && p.y == p2.y && el.value - 1 == el2.value;
                          });
                          if(!walkable && el.value > 0) {
                            e.set<Elevation>({el.value - 1});
                          } else {
                            e.remove<Moving>();
                          }
                        }
                      });

  ecsWorld.system<CarriedBy, Position, Elevation>() // update position of carried entities
    .kind<GameScene>()
    .each([](flecs::entity e, CarriedBy& c, Position& p, Elevation& el) {
      if(c.id) {
        e.set<Position>(*(c.id.get<Position>()));
        e.set<Elevation>({c.id.get<Elevation>()->value});
      }
    });

  // note: the animation systems' architecture is kept limited in extensibility to avoid unnecessary framework code for the use case of a simple game

  animationSystem = ecsWorld.system<Animation>() // animation system for entities with a single animation
                      .kind<GameScene>()
                      .iter([&](flecs::iter& it, Animation* a) {
                        for(auto i: it) {
                          flecs::entity e = it.entity(i);
                          flecs::entity carriedEntity;

                          float newAlpha;
                          if(e.has<MovementDirection>()) {
                            newAlpha = a->alpha + it.delta_time() * 4.0f;
                          } else if(e.has<Goal>()) {
                            newAlpha = a->alpha + it.delta_time() * 0.5f;
                          } else if(e.has<Key>()) {
                            newAlpha = a->alpha + it.delta_time() * 0.1f;
                          } else if(e.has<Lock>()) {
                            newAlpha = a->alpha + it.delta_time() * 1.0f;
                            if(e.get<Carrying>()) {
                              carriedEntity = e.get<Carrying>()->id;
                              float carriedAlpha = carriedEntity.get<Animation>()->alpha;
                              carriedEntity.set<Animation>({carriedAlpha + a->alpha * it.delta_time() * 3.0f});
                            }
                          } else if(e.has<State>()) {
                            newAlpha = a->alpha + it.delta_time() * 4.0f;
                          } else {
                            newAlpha = a->alpha + it.delta_time() * 1.0f;
                          }

                          if(a->alpha < 1.0f) {
                            e.set<Animation>({newAlpha});
                            if(a->alpha >= 1.0f) {
                              e.set<Animation>({1.0f});
                              e.add<AnimationFinished>();

                              if(e.has<Goal>()) {
                                currentLevel++;
                                world->writeSaveFile(world->saveFile);
                                e.world().add<ActiveScene, IntermissionScene>();
                              } else if(e.has<Key>()) {
                                e.set<Animation>({0.0f});
                              } else if(e.has<Lock>()) {
                                deleteRenderObject(carriedEntity);
                                deleteRenderObject(e);
                                carriedEntity.destruct();
                                e.destruct();
                              }
                            }
                          }
                        }
                      });

  eyeAnimationSystem = ecsWorld.system() // a special animation system for Eyes to support multiple animations
                         .with<Eyes>()
                         .kind<GameScene>()
                         .write<Blink, Animation>() // write components not provided by the system signature so that they are updated for the next systems
                         .write<Peer, Animation>()
                         .iter([](flecs::iter& it) {
                           for(auto i: it) {
                             flecs::entity e = it.entity(i);
                             e.set<Blink, Animation>({std::min(e.get<Blink, Animation>()->alpha + it.delta_time() * 10.0f, 1.0f)});
                             e.set<Peer, Animation>({std::min(e.get<Peer, Animation>()->alpha + it.delta_time() * 1.0f, 1.0f)});
                           }
                         });

  eyeBlinkAnimationSystem = ecsWorld.system() // trigger Eyes blink animation periodically with some randomness
                              .with<Eyes>()
                              .kind<GameScene>()
                              .interval(2.33f)
                              .write<Blink, Animation>()
                              .each([](flecs::entity e) {
                                if(Utility::randomDouble(0.0f, 1.0f) > 0.35f) {
                                  e.set<Blink, Animation>({-(float)Utility::randomDouble(0.0f, 2.0f)});
                                }
                              });

  eyePeerAnimationSystem = ecsWorld.system() // the same for Eyes peer animation
                             .with<Eyes>()
                             .kind<GameScene>()
                             .interval(1.47f)
                             .write<Peer, Animation>()
                             .each([](flecs::entity e) {
                               if(Utility::randomDouble(0.0f, 1.0f) > 0.7f) {
                                 e.set<Peer, Animation>({-(float)Utility::randomDouble(0.0f, 2.0f)});
                               }
                             });

  dynamicUpdaterSystem = ecsWorld.system<const RenderObjectHandle, const Position, const Elevation, const Animation>() // handle updating parameters for rendering based on animation state
                           .kind<GameScene>()
                           .each([](flecs::entity e, const RenderObjectHandle& r, const Position& p, const Elevation& el, const Animation& a) {
                             Object*       object = r.renderObject->object;
                             flecs::entity carrier;
                             if(e.get<CarriedBy>()) {
                               carrier = e.get<CarriedBy>()->id;
                               if(carrier) {
                                 object->setPosition(carrier.get<RenderObjectHandle>()->renderObject->object->getPosition() + glm::vec3(0.0f, 1.0f, 0.0f));
                               }
                             }
                             if(e.has<MovementDirection>() && e.has<Moving, Orientation>()) {
                               int x = e.get<MovementDirection>()->x;
                               int y = e.get<MovementDirection>()->y;
                               if(x != 0 || y != 0) {
                                 glm::vec3 v = glm::vec3(-x * 0.5f, 0.5f, -y * 0.5f);
                                 glm::vec3 vr = glm::rotate(v, a.alpha * Utility::tau * 0.25f, glm::vec3(y, 0.0f, -x));
                                 if(!carrier) object->setPosition(glm::vec3(p.x, el.value, p.y) - v + vr - glm::vec3(x, 0.0f, y));
                                 object->setOrientationFromDirection(glm::vec3(0.0f, 0.0f, -1.0f));
                                 glm::vec3 rotateVector = Utility::tau * 0.25f * glm::vec3(y, 0.0f, -x);
                                 object->rotateGlobal(a.alpha * rotateVector);
                                 Object o;
                                 o.setOrientation(e.get<Stationary, Orientation>()->quaternion);
                                 o.rotateGlobal(a.alpha * rotateVector);
                                 e.set<Moving, Orientation>({o.getOrientation()});
                                 if(e.has<AnimationFinished>()) {
                                   e.remove<AnimationFinished>();
                                   o.setOrientation(e.get<Stationary, Orientation>()->quaternion);
                                   if(e.has<Interrupted, MovementDirection>()) {
                                     int xi = e.get<Interrupted, MovementDirection>()->x;
                                     int yi = e.get<Interrupted, MovementDirection>()->y;
                                     e.remove<Interrupted, MovementDirection>();
                                     rotateVector = Utility::tau * 0.25f * glm::vec3(yi, 0.0f, -xi);
                                     o.rotateGlobal(rotateVector);
                                     e.set<Moving, Orientation>({o.getOrientation()});
                                     e.set<Stationary, Orientation>({o.getOrientation()});
                                   } else {
                                     o.rotateGlobal(rotateVector);
                                     e.set<Stationary, Orientation>({o.getOrientation()});
                                   }
                                 }
                               } else {
                                 if(!carrier) {
                                   object->setPosition(glm::vec3(p.x, el.value, p.y));
                                 }
                               }
                             } else if(e.has<Key>()) {
                               if(!carrier) object->setPosition(glm::vec3(p.x, el.value, p.y));
                               object->setOrientationFromDirection(glm::vec3(0.0f, 0.0f, 1.0f));
                               object->rotate(a.alpha * Utility::tau * glm::vec3(0.0f, 1.0f, 0.0f));
                             } else if(e.has<Lock>() || e.has<Goal>()) {
                               if(!carrier) object->setPosition(glm::vec3(p.x, el.value - a.alpha, p.y));
                             }
                           });

  staticUpdaterSystem = ecsWorld.system<const RenderObjectHandle, const Position, const Elevation>() // handle updating parameters for rendering for static objects
                          .with<Moving>()
                          .without<Animation>()
                          .kind<GameScene>()
                          .each([](flecs::entity e, const RenderObjectHandle& r, const Position& p, const Elevation& el) {
                            Object* object = r.renderObject->object;
                            object->setPosition(glm::vec3(p.x, el.value, p.y));
                          });

  eyeUpdaterSystem = ecsWorld.system<const RenderObjectHandle>() // handle updating parameters for rendering based on animation state: a special case handling of Eyes entities
                       .with<Eyes>()
                       .kind<GameScene>()
                       .each([&](flecs::entity e, const RenderObjectHandle& r) {
                         Object* object = r.renderObject->object;
                         if(e.parent().get<Animation>()) {
                           if(e.parent().get<Animation>()->alpha == 1.0f) {
                             object->setOrientation(e.parent().get<Stationary, Orientation>()->quaternion);
                           } else {
                             object->setOrientation(e.parent().get<Moving, Orientation>()->quaternion);
                           }
                         }
                         object->setPosition(e.parent().get<RenderObjectHandle>()->renderObject->object->getPosition());
                         float alpha = e.get<Blink, Animation>()->alpha;
                         if(alpha < 1.0f) {
                           object->setPosition(camera.getPosition() + glm::vec3(0.0f, 1000.0f, 0.0f));
                         }
                         alpha = e.get<Peer, Animation>()->alpha;
                         float direction = 1.0f;
                         if(alpha < 0.5f) {
                           direction = -1.0f;
                         }
                         if(alpha < 1.0f) {
                           object->rotate(direction * Utility::tau * 0.025f * glm::vec3(0.0f, 1.0f, 0.0f));
                         }
                       });

  menuRenderSystem = ecsWorld.system<const Selection, Position>() // render menu
                       .kind(0)
                       .each([&](flecs::entity e, const Selection& selection, const Position& position) {
                         flecs::entity previousSelection = querySelections.find([&](Selection& selection, State& state) {
                           return (state.value == menuState.get<PreviousState>()->value);
                         });
                         float         alpha = menuState.get<Animation>()->alpha;
                         glm::vec2     pos = glm::vec2(position.x, position.y);
                         glm::vec2     prevPos = glm::vec2(previousSelection.get<Position>()->x, previousSelection.get<Position>()->y);
                         float         c = std::min(exp(-10.0f * alpha), 1.0f);
                         if(e.get<State>()->value == menuState.get<State>()->value) {
                           Utility::renderRectangle(pos + glm::vec2(-30, 25) + c * (prevPos - pos), glm::vec2(20), sf::Color(255, 255, 255, 255), alpha * 180);
                           Utility::renderMenuText(pos + c * glm::vec2(20, 0), selection.text, sf::Color(255, 255, 255, 255));
                           Utility::renderText(pos + glm::vec2(250, 20) + c * glm::vec2(50, 0), selection.secondaryText, sf::Color(255, 255, 255, 255));
                         } else {
                           Utility::renderMenuText(pos, selection.text, sf::Color(255, 255, 255, 200));
                         }
                       });

  intermissionRenderSystem = ecsWorld.system<const Intermission, Animation>() // handle the intermission between levels
                               .kind(0)
                               .iter([&](flecs::iter& it, const Intermission* intr, Animation* a) {
                                 for(auto i: it) {
                                   flecs::entity e = it.entity(i);
                                   if(a->alpha < 2.0) {
                                     e.set<Animation>({a->alpha + it.delta_time() * 0.5f});
                                   }
                                   int x = renderWindow->getSize().x * 0.15f;
                                   if(a->alpha >= 1.0) {
                                     if(world->currentLevel == world->levelFiles.size()) {
                                       Utility::renderText(glm::vec2(x, 160), "Press [confirm] to return to the main menu...", sf::Color(0, 0, 0, (a->alpha - 1.0) * 255));
                                     } else {
                                       Utility::renderText(glm::vec2(x, 160), "Press [confirm] to continue...", sf::Color(0, 0, 0, (a->alpha - 1.0) * 255));
                                     }
                                   }
                                   Utility::renderMenuText(glm::vec2(x - 5, 100), intr->text, sf::Color(0, 0, 0, std::min(a->alpha * 255, 255.0f)));
                                 }
                               });

  messageRenderSystem = ecsWorld.system<const Message, Animation>() // handle messages
                          .kind(0)
                          .iter([&](flecs::iter& it, const Message* msg, Animation* a) {
                            for(auto i: it) {
                              flecs::entity e = it.entity(i);
                              if(a->alpha < 1.0) {
                                e.set<Animation>({a->alpha + it.delta_time() * 0.5f});
                              }
                              Utility::renderText(glm::vec2(5, 30), msg->text, sf::Color(0, 0, 0, std::min(a->alpha * 255, 255.0f)));
                            }
                          });

  dialogueRenderSystem = ecsWorld.system<const Dialogue>() // handle dialogue
                           .kind(0)
                           .iter([&](flecs::iter& it, const Dialogue* dlg) {
                             for(auto i: it) {
                               flecs::entity e = it.entity(i);
                               if(!world->camera.isFollowMode()) {
                                 int wx = world->renderWindow->getSize().x * 0.5f;
                                 int wy = world->renderWindow->getSize().y * 0.5f;
                                 int rx = 350;
                                 int ry = 95;
                                 Utility::renderRectangle(glm::vec2(wx, wy) + glm::vec2(-rx, -ry), glm::vec2(2 * rx, 2 * ry), sf::Color(0, 0, 0, 150));
                                 HelperData* data = &(world->helpers.at(world->levelHelpers.at(world->currentLevel)));
                                 Utility::renderText(glm::vec2(wx, wy) + glm::vec2(-rx, -ry) + glm::vec2(10, 10), data->name, data->colour);
                                 Utility::renderText(glm::vec2(wx, wy) + glm::vec2(-rx, -ry) + glm::vec2(10, 60), dlg->text, sf::Color(255, 255, 255, 255));
                               }
                             }
                           });
}

void World::addRenderObject(Model* model, Texture* texture, Object* object) {
  renderObjects.emplace_back(&modelTexturePairs, model, texture, object);
}

void World::deleteObject(RenderObject* renderObject) {
  for(auto it = objects.begin(); it != objects.end(); ++it) {
    if(&(*it) == renderObject->object) {
      ;
      objects.erase(it);
      break;
    }
  }
  for(auto it = renderObjects.begin(); it != renderObjects.end(); ++it) {
    if(&(*it) == renderObject) {
      ;
      renderObjects.erase(it);
      break;
    }
  }
}

bool World::writeSaveFile(std::string path) {
  unsigned int size = sizeof(int);
  char*        memBlock = new char[size];
  char*        memPointer = memBlock;

  Utility::writeValue(&memPointer, currentLevel);

  bool success = Utility::writeFile(path, memBlock, size);
  delete[] memBlock;
  return success;
}

int World::readSaveFile(std::string path) {
  char* memBlock;
  if(Utility::readFile(path, &memBlock, nullptr)) {
    char* memPointer = memBlock;

    int level;
    Utility::readValue(&memPointer, &level);

    delete[] memBlock;
    return level;
  }
  return -1;
}

bool World::writeLevelFile(std::string path) {
  unsigned int size = sizeof(int) + (objects.size() * 3 * sizeof(int));
  char*        memBlock = new char[size];
  char*        memPointer = memBlock;

  Utility::writeValue(&memPointer, ecsWorld.count<GameObject>());
  queryGameObjects.each([&](flecs::entity e, GameObject o, Position p) {
    Utility::writeValue(&memPointer, o.type);
    Utility::writeValue(&memPointer, p.x);
    Utility::writeValue(&memPointer, p.y);
  });

  const std::string fullPath = levelDirectory + "/" + path;
  bool              success = Utility::writeFile(fullPath, memBlock, size);
  delete[] memBlock;
  return success;
}

bool World::readLevelFile(std::string path) {
  char*             memBlock;
  unsigned int      sizeInBytes;
  const std::string fullPath = levelDirectory + "/" + path;
  if(Utility::readFile(fullPath, &memBlock, &sizeInBytes)) {
    char* memPointer = memBlock;

    int objectCount;
    Utility::readValue(&memPointer, &objectCount);
    for(int i = 0; i < objectCount; i++) {
      int type, x, y;
      Utility::readValue(&memPointer, &type);
      Utility::readValue(&memPointer, &x);
      Utility::readValue(&memPointer, &y);
      addGameObject(type, x, y);
    }

    delete[] memBlock;
    return true;
  }
  return false;
}

bool World::readAssetFile(std::string path) {
  char* memBlock;
  if(Utility::readFile(path, &memBlock, nullptr)) {
    char* memPointer = memBlock;

    if(!Utility::readFontFromMemoryBlock(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    Utility::initTextRendering();
    cubeModel.readModelFromMemoryBlock(&memPointer);
    cubeModel.bufferData();
    if(!cubeTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    eyesModel.readModelFromMemoryBlock(&memPointer);
    eyesModel.bufferData();
    if(!eyesTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    grassModel.readModelFromMemoryBlock(&memPointer);
    grassModel.bufferData();
    if(!grassTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    gridModel.readModelFromMemoryBlock(&memPointer);
    gridModel.bufferData();
    if(!gridTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    goalModel.readModelFromMemoryBlock(&memPointer);
    goalModel.bufferData();
    if(!goalTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    waterModel.readModelFromMemoryBlock(&memPointer);
    waterModel.bufferData();
    if(!waterTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    boulderModel.readModelFromMemoryBlock(&memPointer);
    boulderModel.bufferData();
    if(!boulderTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    keyModel.readModelFromMemoryBlock(&memPointer);
    keyModel.bufferData();
    if(!keyTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    lockModel.readModelFromMemoryBlock(&memPointer);
    lockModel.bufferData();
    if(!lockTexture.loadTextures(&memPointer)) {
      delete[] memBlock;
      return false;
    }
    for(int i = 0; i < helpers.size(); i++) {
      helpers.at(i).model.readModelFromMemoryBlock(&memPointer);
      helpers.at(i).model.bufferData();
      if(!helpers.at(i).texture.loadTextures(&memPointer)) {
        delete[] memBlock;
        return false;
      }
    }

    delete[] memBlock;
    return true;
  }
  return false;
}

World::World() {
}

World::~World() {
  glDeleteVertexArrays(1, &vertexArray);
}

bool World::init(sf::RenderWindow* renderWindow, Input* input) {
  // window, filesystem, input, and rendering

  world = this;
  levelDirectory = "res/levels";
  saveFile = "res/gamesave";

  levelFiles.emplace_back("00");
  levelFiles.emplace_back("01");
  levelFiles.emplace_back("02");
  levelFiles.emplace_back("03");
  levelFiles.emplace_back("04");
  levelNames.emplace_back("Level 1 - Boulder Is Push");
  levelNames.emplace_back("Level 2 - Three Keys");
  levelNames.emplace_back("Level 3 - The Cliff");
  levelNames.emplace_back("Level 4 - Locked Out");
  levelNames.emplace_back("Level 5 - No Return");
  levelNames.emplace_back("You have solved all 5 levels! Thanks for playing!");
  helpDialogue.emplace_back("You only need directional movement to solve the levels.\nUse a gamepad or the arrow keys / WASD / HJKL on a keyboard.\nGo back to the menu and load your progress if you need to restart a level.\n\nAsk the teapots for tips if needed.");
  helpDialogue.emplace_back("Looks like the boulders cannot move on grass.");
  helpDialogue.emplace_back("Perhaps the key can be pushed to a safe area.");
  helpDialogue.emplace_back("Aren't locks basically boulders anyway?");
  helpDialogue.emplace_back("Add hot water to oversteeped tea to fix the bitterness.\nAlso, the keys are metal so they are waterproof in case you didn't know for some reason.");
  levelHelpers.emplace_back(0);
  levelHelpers.emplace_back(1);
  levelHelpers.emplace_back(2);
  levelHelpers.emplace_back(1);
  levelHelpers.emplace_back(2);
  helpers.emplace_back(HelperData{"Tetrahedron", sf::Color(255, 140, 40, 255)});
  helpers.emplace_back(HelperData{"Short Teapot", sf::Color(180, 100, 210, 255)});
  helpers.emplace_back(HelperData{"Tall Teapot", sf::Color(130, 235, 235, 255)});

  currentLevel = 0;
  tileSize = 40;

  this->renderWindow = renderWindow;
  this->input = input;
  Utility::initRenderState(renderWindow, &projectionViewMatrix);

  glGenVertexArrays(1, &vertexArray);

  depthMapResolution = 4096;
  shadowLevelCount = 5;
  zMult = 10.0f;
  zBias = 5.0f;
  fov = 70.0f;
  nearClippingPlane = 0.1f;
  farClippingPlane = 75.0f;
  multisamplingEnabled = true;
  multisamplingSampleCount = 4;

  if(!opaqueShader.setup(shadowLevelCount)) return false;
  if(!shadowMappingShader.setup(depthMapResolution, shadowLevelCount)) return false;
  glm::ivec2 windowSize((int)renderWindow->getSize().x, (int)renderWindow->getSize().y);
  if(!blurShader.setup(windowSize)) return false;
  if(!screenShader.setup(windowSize, multisamplingEnabled, multisamplingSampleCount)) return false;

  maxBlurIterations = 20;

  followCameraTargetOffset = glm::vec3(0.0f, 5.0f, 3.0f);
  followCameraSpeed = 5.0f;
  camera.setupOrbitCamera(glm::vec3(20.0f, 0.0f, 10.0f), 15.0f);

  // game world and scenes

  initQueries();
  initScenes();
  initSystems();

  readAssetFile("res/assetdata");
  ecsWorld.add<ActiveScene, MenuScene>();

  return true;
}

bool World::update(double dt) {
  if(ecsWorld.has<ActiveScene, MenuScene>()) {
    // Menu
    if(input->combineToDigitalValue("up") ||
       input->combineToDigitalValue("up_alt") ||
       input->combineToDigitalValue("up_alt2")) {
      if(menuState.get<State>()->value > 0) {
        menuState.set<Animation>({0.0f});
        menuState.set<PreviousState>({menuState.get<State>()->value});
        menuState.set<State>({menuState.get<State>()->value - 1});
      }
    } else if(!input->keyDown("up") &&
              !input->keyDown("up_alt") &&
              !input->keyDown("up_alt2") &&
              (input->combineToDigitalValue("down") ||
               input->combineToDigitalValue("down_alt") ||
               input->combineToDigitalValue("down_alt2"))) {
      if(menuState.get<State>()->value < 2) {
        menuState.set<Animation>({0.0f});
        menuState.set<PreviousState>({menuState.get<State>()->value});
        menuState.set<State>({menuState.get<State>()->value + 1});
      }
    }
    if(input->combineToDigitalValue("confirm")) {
      if(menuState.get<State>()->value == 0) {
        currentLevel = 0;
        ecsWorld.add<ActiveScene, IntermissionScene>();
      } else if(menuState.get<State>()->value == 1 && !menuState.has<ContinueDisabled>()) {
        currentLevel = readSaveFile(saveFile);
        readLevelFile(levelFiles.at(currentLevel));
        ecsWorld.add<ActiveScene, IntermissionScene>();
      } else if(menuState.get<State>()->value == 2) {
        return false;
      }
    }
    // run some systems manually for the main menu
    elevationSystem.run(dt);
    animationSystem.run(dt);
    eyeAnimationSystem.run(dt);
    eyeBlinkAnimationSystem.run(dt);
    eyePeerAnimationSystem.run(dt);
    dynamicUpdaterSystem.run(dt);
    staticUpdaterSystem.run(dt);
    eyeUpdaterSystem.run(dt);
  } else if(ecsWorld.has<ActiveScene, IntermissionScene>()) {
    if(input->combineToDigitalValue("confirm")) {
      if(currentLevel == levelFiles.size()) {
        currentLevel = 0;
        ecsWorld.add<ActiveScene, MenuScene>();
      } else {
        ecsWorld.add<ActiveScene, GameScene>();
      }
    }
  } else if(ecsWorld.has<ActiveScene, GameScene>()) {
    // Game
    int        x = cube.get<Position>()->x;
    int        y = cube.get<Position>()->y;
    int        elevation = cube.get<Elevation>()->value;
    glm::ivec2 nextTile(x, y);
    glm::ivec2 nextTile2(x, y);
    glm::ivec2 nextTile3(x, y);

    bool tryMoving = false;

    if(input->combineToDigitalValue("up") ||
       input->combineToDigitalValue("up_alt") ||
       input->combineToDigitalValue("up_alt2")) {
      nextTile.y = y - 1;
      nextTile2.y = y - 2;
      nextTile3.y = y - 3;
      tryMoving = true;
    } else if(input->combineToDigitalValue("down") ||
              input->combineToDigitalValue("down_alt") ||
              input->combineToDigitalValue("down_alt2")) {
      nextTile.y = y + 1;
      nextTile2.y = y + 2;
      nextTile3.y = y + 3;
      tryMoving = true;
    } else if(input->combineToDigitalValue("left") ||
              input->combineToDigitalValue("left_alt") ||
              input->combineToDigitalValue("left_alt2")) {
      nextTile.x = x - 1;
      nextTile2.x = x - 2;
      nextTile3.x = x - 3;
      tryMoving = true;
    } else if(input->combineToDigitalValue("right") ||
              input->combineToDigitalValue("right_alt") ||
              input->combineToDigitalValue("right_alt2")) {
      nextTile.x = x + 1;
      nextTile2.x = x + 2;
      nextTile3.x = x + 3;
      tryMoving = true;
    }

    if(tryMoving) {
      int maxElevation = 0;
      queryWalkable.each([&](Walkable w, Position& p, Elevation& e) {
        if(p.x == nextTile.x && p.y == nextTile.y && e.value > maxElevation) {
          maxElevation = e.value;
        }
      });
      flecs::entity nextWalkable = queryWalkable.find([&](Walkable w, Position& p, Elevation& e) {
        return (p.x == nextTile.x && p.y == nextTile.y && maxElevation <= elevation);
      });
      flecs::entity nextBlocking = queryBlocking.find([&](Blocking b, Position& p) {
        return (p.x == nextTile.x && p.y == nextTile.y);
      });
      flecs::entity nextPushable = queryPushable.find([&](Pushable pu, Position& p, Elevation& e) {
        return (p.x == nextTile.x && p.y == nextTile.y && e.value == elevation);
      });
      flecs::entity nextCollectable = queryCollectable.find([&](Collectable c, Position& p, Elevation& e) {
        return (p.x == nextTile.x && p.y == nextTile.y && e.value <= elevation);
      });
      flecs::entity nextLock = queryLock.find([&](Lock l, Position& p) {
        return (p.x == nextTile.x && p.y == nextTile.y);
      });
      flecs::entity nextGoal = queryGoal.find([&](Goal l, Position& p) {
        return (p.x == nextTile.x && p.y == nextTile.y);
      });
      flecs::entity nextHelper = queryHelper.find([&](Helper h, Position& p) {
        return (p.x == nextTile.x && p.y == nextTile.y);
      });
      flecs::entity pushableBlocked = queryBlocking.find([&](Blocking b, Position& p) {
        return (p.x == nextTile2.x && p.y == nextTile2.y);
      });

      auto moveCube = [&]() {
        cube.add<Moving>();
        cube.set<Position>({nextTile.x, nextTile.y});
        if(cube.has<Animation>() && cube.get<Animation>()->alpha < 1.0f) {
          cube.add<AnimationFinished>();
          cube.set<Interrupted, MovementDirection>(*(cube.get<MovementDirection>()));
        }
        cube.set<Animation>({0.0f});
        cube.set<MovementDirection>({nextTile.x - x, nextTile.y - y});
      };

      bool unmovable = cube.has<CarriedBy>();
      if(nextWalkable && !nextBlocking && !unmovable) {
        moveCube();
      }

      flecs::entity carriedEntity = cube.get<Carrying>()->id;
      if(nextCollectable && !carriedEntity && !unmovable) {
        moveCube();
        cube.set<Carrying>({nextCollectable});
        nextCollectable.set<CarriedBy>({cube});
        nextCollectable.remove<Blocking>();
        nextCollectable.remove<Collectable>();
      }

      if(nextLock && carriedEntity) {
        if(carriedEntity.has<Key>()) {
          if(!nextLock.has<Animation>()) {
            cube.set<Carrying>({flecs::entity::null()});
            carriedEntity.set<CarriedBy>({nextLock});
            nextLock.set<Carrying>({carriedEntity});
            nextLock.set<Animation>({0.0f});
          }
        }
      }

      if(nextGoal && !unmovable) {
        cube.set<CarriedBy>({nextGoal});
        nextGoal.set<Carrying>({cube});
        nextGoal.set<Animation>({0.0f});
        blurAnimation.remove<ReverseAnimation>();
        blurAnimation.set<Animation>({0.0f});
        ecsWorld.entity()
          .set(Message{"Saving progress"})
          .set(Animation{0.0f})
          .child_of(ecsWorld.component<SceneRoot>());
      }
      if(nextHelper && camera.isFollowMode()) {
        camera.setFollowMode(false);
        camera.setupOrbitCamera(helperObject->getPosition(), 3.0f);
      } else {
        camera.setFollowMode(true);
      }

      // the logic gets a bit too convoluted here, but shall be good enough for the demo
      flecs::entity pushableCollectable = queryCollectable.find([&](Collectable c, Position& p, Elevation& e) {
        return (p.x == nextTile2.x && p.y == nextTile2.y);
      });
      flecs::entity pushableCollectableBlocked = queryBlocking.find([&](Blocking b, Position& p) {
        return (p.x == nextTile3.x && p.y == nextTile3.y);
      });
      if(nextPushable && (!pushableBlocked || (pushableCollectable && !pushableCollectableBlocked)) && !unmovable) {
        maxElevation = 0;
        queryWalkable.each([&](Walkable w, Position& p, Elevation& e) {
          if(p.x == nextTile2.x && p.y == nextTile2.y && e.value > maxElevation) {
            maxElevation = e.value;
          }
        });
        flecs::entity pushableSupported = querySupporting.find([&](Supporting s, Position& p) {
          return (p.x == nextTile2.x && p.y == nextTile2.y && maxElevation + 1 == nextPushable.get<Elevation>()->value);
        });
        if(pushableSupported) {
          moveCube();
          nextPushable.add<Moving>();
          nextPushable.set<Position>({nextTile2.x, nextTile2.y});

          flecs::entity unlockableKey = queryKey.find([&](Key k, Position& p) {
            return (p.x == nextTile2.x && p.y == nextTile2.y);
          });
          if(nextLock && unlockableKey) {
            if(!nextLock.has<Animation>()) {
              unlockableKey.set<CarriedBy>({nextLock});
              unlockableKey.remove<Collectable>();
              nextLock.set<Carrying>({unlockableKey});
              nextLock.set<Animation>({0.0f});
              nextLock.remove<Pushable>();
            }
          } else if(pushableCollectable) {
            pushableCollectable.add<Moving>();
            pushableCollectable.set<Position>({nextTile3.x, nextTile3.y});
          }
        }
      }
    }
  }

  for(auto it = objects.begin(); it != objects.end(); ++it) {
    it->integrate(dt);
  }
  ecsWorld.progress(dt);

  if(input->keyHit("quit") || input->keyHit("quit_alt")) {
    ecsWorld.add<ActiveScene, MenuScene>();
  }
  return true;
}

void World::updateCameraPosition(double dt, double alpha) {
  camera.updatePosition(dt, alpha);
}

void World::renderGeometry(double alpha) {
  renderWindow->setActive(true);

  glBindVertexArray(vertexArray);
  glDisable(GL_FRAMEBUFFER_SRGB);

  std::vector<Light::DirectionalLight> directionalLights;
  directionalLights.push_back(Light::DirectionalLight{glm::vec3(-0.3f, -1.0f, -0.3f), glm::vec3(1.0f, 0.75f, 0.5f), 2.0f});
  std::vector<Light::PointLight> pointLights;

  glm::mat4              view = camera.getViewMatrix();
  std::vector<glm::vec2> clippingPlanes = shadowMappingShader.calculateClippingPlanes(nearClippingPlane, farClippingPlane, shadowLevelCount);
  std::vector<glm::mat4> projectionMatrices = shadowMappingShader.calculateProjectionMatrices(renderWindow, fov, clippingPlanes);
  std::vector<glm::mat4> projectionViewMatrices;
  for(auto it = projectionMatrices.begin(); it != projectionMatrices.end(); ++it) {
    projectionViewMatrices.push_back((*it) * view);
  }
  std::vector<glm::mat4> projectionViewInverseMatrices;
  for(auto it = projectionViewMatrices.begin(); it != projectionViewMatrices.end(); ++it) {
    projectionViewInverseMatrices.push_back(glm::inverse(*it));
  }

  glm::mat4 projection = glm::perspective(
    glm::radians(fov),
    (float)renderWindow->getSize().x / (float)renderWindow->getSize().y,
    nearClippingPlane,
    farClippingPlane);
  projectionViewMatrix = projection * view;

  glm::vec3 lightDir = glm::normalize(directionalLights.back().direction);
  glm::mat4 depthViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), lightDir, glm::vec3(0, 1, 0));

  std::vector<glm::mat4> depthViewProjections;
  for(auto it = projectionViewInverseMatrices.begin(); it != projectionViewInverseMatrices.end(); ++it) {
    depthViewProjections.push_back(shadowMappingShader.calculateDepthViewProjection(directionalLights.back().direction, *it, zMult, zBias));
  }

  shadowMappingShader.render(&renderObjects, alpha, depthViewProjections);

  for(auto it = clippingPlanes.begin(); it != clippingPlanes.end(); ++it) {
    glm::vec4 vecView(0.0f, 0.0f, it->y, 1.0f);
    glm::vec4 vecClip = projection * vecView;
    it->y = -vecClip.z;
  }

  glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
  for(auto it = depthViewProjections.begin(); it != depthViewProjections.end(); ++it) {
    *it = biasMatrix * (*it);
  }

  unsigned int blurIterations = 0;
  if(blurAnimation) {
    if(blurAnimation.has<ReverseAnimation>()) {
      blurIterations = int(maxBlurIterations * (1.0f - blurAnimation.get<Animation>()->alpha));
    } else {
      blurIterations = int(maxBlurIterations * blurAnimation.get<Animation>()->alpha);
    }
  }
  GLuint framebuffer = screenShader.framebuffer;
  if(blurIterations > 0) framebuffer = blurShader.getStartingFramebuffer();

  opaqueShader.render(renderWindow,
                      framebuffer,
                      &directionalLights,
                      &pointLights,
                      view,
                      shadowMappingShader.depthTextures,
                      &renderObjects,
                      alpha,
                      projectionViewMatrix,
                      depthViewProjections,
                      clippingPlanes);

  if(blurIterations > 0) blurShader.render(renderWindow, screenShader.framebuffer, blurIterations);
  screenShader.render(renderWindow, true);
}

void World::renderUi(int fps) {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  renderWindow->setActive(false);
  renderWindow->pushGLStates();
  renderWindow->resetGLStates();

  // running some systems manually that are not attached to a scene

  Utility::renderText(glm::vec2(5, 5), std::to_string(fps) + " FPS", sf::Color(255, 255, 255, 200));
  if(ecsWorld.has<ActiveScene, IntermissionScene>()) {
    intermissionRenderSystem.run();
  }
  if(ecsWorld.has<ActiveScene, MenuScene>()) {
    int x = renderWindow->getSize().x * 0.15f;
    Utility::renderMenuText(glm::vec2(x, 200), "Cube Is Walk", sf::Color(255, 255, 255, 255));
    Utility::renderMenuText(glm::vec2(x - 20, 200), "___________", sf::Color(255, 255, 255, 255));
    menuRenderSystem.run();
  }
  if(ecsWorld.has<ActiveScene, GameScene>()) {
    dialogueRenderSystem.run();
  }
  messageRenderSystem.run();

  renderWindow->popGLStates();
}
