#include <iostream>
#include <unordered_map>

#include "imgui.h"
#include "json.h"

#include "engine.h"
#include "model.h"

using json = nlohmann::json;

#define PROGRAM_NAME "Tutorial"

//Take all the information in the config file, and stuff it into where it needs to go
int processConfig(int argc, char** argv, json& config, Engine::Context&, Object*& sun);
//Start loading planets from the config file
int loadPlanets(json& planetConfig, Object& sun, float spaceScale, float timeScale, Shader* defaultShader);
//Load an individual planet's data
int loadPlanetContext(json& config, Object::Context& ctx, float spaceScale, float timeScale, float scaleMultiplier, Shader* defaultShader);
//Display help menu
void helpMenu();