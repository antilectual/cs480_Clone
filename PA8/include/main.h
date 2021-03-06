#include <iostream>
#include <vector>
#include <unordered_map>
#include <btBulletDynamicsCommon.h>
#include "imgui.h"
#include "json.h"
#include "engine.h"
#include "physics_model.h"
#include "physics_world.h"
#include "gameworldctx.h"

using json = nlohmann::json;

#define PROGRAM_NAME "Tutorial"

GameWorld::ctx *gameCtx = new GameWorld::ctx;

//Take all the information in the config file, and stuff it into where it needs to go
int processConfig(int argc, char **argv, json& config, Engine::Context &ctx);
//Load an object's data
int loadObjectContext(json &config, Object::Context &ctx, Shader* defaultShader, PhysicsWorld *physWorld);
int loadLightContext(json &config, Graphics::LightContext &ctx);
//Display help menu
void helpMenu();