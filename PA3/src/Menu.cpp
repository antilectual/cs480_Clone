#include "Menu.h"

Menu::Menu(Window& a, Object& b) : window(a), root(b), options(_options) {
	//Attach ImGUI to our SDL window
	ImGui_ImplSdlGL3_Init(window.getSDL_Window());
	
	//Defaults
	_options.numPlanets = 0;
	_options.planetSelector = 0;
	satelliteList = "";
	
	//Now build our satellite drop-down list
	//We're assuming this will never change after construction of the Menu object
	buildSatelliteList(root, "", _options.numPlanets);
}

void Menu::update() {
	ImGui_ImplSdlGL3_NewFrame(window.getSDL_Window());
	
	//ImGui::ShowTestWindow();
	
	//Start a new window called "Controls"
	//On first seeing this window, make it 500 x 200, but let the user resize it later
	ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("Controls", NULL, ImGuiWindowFlags_ShowBorders);
	
	//Get information of whatever planet we're currently looking at
	Object::Context& ctx = getPlanet(_options.planetSelector)->ctx;
	const Object::Context& originalCtx = getPlanet(_options.planetSelector)->originalCtx;
	
	//Drop down planet list
	ImGui::Combo("Select Satellite", &_options.planetSelector, satelliteList.c_str());
	
	//Time Scale slider and reset
	ImGui::SliderFloat("Time Scale", &ctx.timeScale, -5.0f, 5.0); ImGui::SameLine(ImGui::GetWindowWidth() - 50);
	if(ImGui::Button("Reset##timeScale")) ctx.timeScale = originalCtx.timeScale;
	
	//Orbit speed, reset, and direction
	ImGui::SliderFloat("Orbit Speed", &ctx.moveScale, 0.0, 5.0); ImGui::SameLine(ImGui::GetWindowWidth() - 50);
	if(ImGui::Button("Reset##moveScale")) ctx.moveScale = originalCtx.moveScale, ctx.moveDir = originalCtx.moveDir;
	ImGui::RadioButton("cw##move", &ctx.moveDir, -1); ImGui::SameLine();
	ImGui::RadioButton("ccw##move", &ctx.moveDir, 1);
	
	//Spin speed, reset, and direction
	ImGui::SliderFloat("Spin Speed", &ctx.spinScale, 0.0, 5.0); ImGui::SameLine(ImGui::GetWindowWidth() - 50);
	if(ImGui::Button("Reset##spinScale")) ctx.spinScale = originalCtx.spinScale, ctx.spinDir = originalCtx.spinDir;
	ImGui::RadioButton("cw##spin", &ctx.spinDir, -1); ImGui::SameLine();
	ImGui::RadioButton("ccw##spin", &ctx.spinDir, 1);
	
	//Change the planet we're looking at
	if(ImGui::Button("Focus")) _options.lookingAt = _options.planetSelector;
	
	//End the window
	ImGui::End();
}

void Menu::render() {
	ImGui::Render();
}

void Menu::buildSatelliteList(Object& root, std::string pre, int& numSatellites) {
	//Build a tree-like format where it's clear what orbits what
	//-Sun
	//|-Mercury
	//|-Venus
	//|-Earth
	//||-The Moon
	//|-Mars
	satelliteList += pre;
	satelliteList += '-';
	satelliteList += root.ctx.name;
	satelliteList += '\0';
	
	satelliteMap[numSatellites] = &root;
	
	numSatellites++;
	
	//Now add all the satellites
	for(int i = 0; i < root.getNumChildren(); i++) {
		buildSatelliteList(root[i], pre + '|', numSatellites);
	}
}

Object* Menu::getPlanet(int index) const {
	return satelliteMap.at(index);
}