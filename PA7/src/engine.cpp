
#include "engine.h"

Engine::Engine(const Context &ctx, Object *sun) : m_cube(sun), windowWidth(m_WINDOW_WIDTH), windowHeight(m_WINDOW_HEIGHT) {
	m_WINDOW_NAME = ctx.name;
	m_WINDOW_WIDTH = ctx.width;
	m_WINDOW_HEIGHT = ctx.height;
	m_FULLSCREEN = ctx.fullscreen;
	
	m_light = ctx.lightStrength;
	
	mouseDown = false;
}

Engine::~Engine() {
	delete m_window;
	delete m_graphics;
	m_window = NULL;
	m_graphics = NULL;
}

bool Engine::Initialize() {
	// Start a window
	m_window = new Window();
	if (!m_window->Initialize(m_WINDOW_NAME, &m_WINDOW_WIDTH, &m_WINDOW_HEIGHT)) {
		printf("The window failed to initialize.\n");
		return false;
	}
	
	//Start the menu and connect it to the window
	m_menu = new Menu(*m_window, *m_cube);
	
	// Start the graphics
	m_graphics = new Graphics(m_cube, m_light, *m_menu, m_WINDOW_WIDTH, m_WINDOW_HEIGHT);
	if (!m_graphics->Initialize(m_WINDOW_WIDTH, m_WINDOW_HEIGHT)) {
		printf("The graphics failed to initialize.\n");
		return false;
	}
	
	Object::menu = m_menu;
	
	// Set the time
	m_currentTimeMillis = GetCurrentTimeMillis();
	
	// No errors
	return true;
}

void Engine::Run() {
	m_running = true;
	
	while (m_running) {
		// Update the DT
		m_DT = getDT();
		
		while (SDL_PollEvent(&m_event) != 0) {
			eventHandler();
			ImGui_ImplSdlGL3_ProcessEvent(&m_event);
		}
		// Check the keyboard input
		Keyboard(m_DT);
		
		//Make certain whatever we're looking at is at origin
		Object::globalOffset = &m_menu->getPlanet(m_menu->options.lookingAt)->position;

		// Update planet positions
		m_graphics->Update(m_DT);
		
		// Update menu options and labels
		m_menu->update(m_DT, m_WINDOW_WIDTH, m_WINDOW_HEIGHT);
		
		if(m_menu->options.switchedPlanetView) m_graphics->cameraMode = CAMERA_MODE_FOLLOW;
		
		//Render everything
		m_graphics->Render();
		m_menu->render();
		
		// Swap to the Window
		m_window->Swap();
	}
	
	ImGui_ImplSdlGL3_Shutdown();
}

void Engine::Keyboard(unsigned dt) {
	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	float cameraSpeed = glm::length(m_graphics->lookAt - m_graphics->eyePos) * dt / 500;
	
	if(SDL_GetModState() & KMOD_SHIFT) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go Down
		glm::vec3 moveDir = {0.0, -1.0, 0.0};
		moveDir *= cameraSpeed;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	if(keyState[SDL_SCANCODE_SPACE]) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go Up
		glm::vec3 moveDir = {0.0, 1.0, 0.0};
		moveDir *= cameraSpeed;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	if(keyState[SDL_SCANCODE_W]) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go forward
		glm::vec3 moveDir = glm::normalize(m_graphics->lookAt - m_graphics->eyePos);
		moveDir *= cameraSpeed;
		moveDir.y = 0;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	if(keyState[SDL_SCANCODE_A]) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go left
		glm::vec3 moveDir = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), m_graphics->lookAt - m_graphics->eyePos));
		moveDir *= cameraSpeed;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	if(keyState[SDL_SCANCODE_S]) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go back
		glm::vec3 moveDir = glm::normalize(m_graphics->eyePos - m_graphics->lookAt);
		moveDir.y = 0;
		moveDir *= cameraSpeed;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	if(keyState[SDL_SCANCODE_D]) {
		m_graphics->cameraMode = CAMERA_MODE_FREE;
		//Go right
		glm::vec3 moveDir = glm::normalize(glm::cross(m_graphics->lookAt - m_graphics->eyePos, glm::vec3(0.0, 1.0, 0.0)));
		moveDir *= cameraSpeed;
		m_graphics->lookAt += moveDir;
		m_graphics->eyePos += moveDir;
	}
	//Rotations
	if(keyState[SDL_SCANCODE_LEFT]) {
		float step = 0.05 * dt;
		m_menu->setRotation(m_menu->options.rotation + step);
		if(m_graphics->cameraMode == CAMERA_MODE_FREE) {
			step *= M_PI / -180;
			glm::vec3 toLookAt = m_graphics->lookAt - m_graphics->eyePos;
			m_graphics->lookAt.x = toLookAt.x * cos(step) - toLookAt.z * sin(step) + m_graphics->eyePos.x;
			m_graphics->lookAt.z = toLookAt.x * sin(step) + toLookAt.z * cos(step) + m_graphics->eyePos.z;
		}
	}
	if(keyState[SDL_SCANCODE_RIGHT]) {
		float step = 0.05 * dt;
		m_menu->setRotation(m_menu->options.rotation - step);
		if(m_graphics->cameraMode == CAMERA_MODE_FREE) {
			step *= M_PI / 180;
			glm::vec3 toLookAt = m_graphics->lookAt - m_graphics->eyePos;
			m_graphics->lookAt.x = toLookAt.x * cos(step) - toLookAt.z * sin(step) + m_graphics->eyePos.x;
			m_graphics->lookAt.z = toLookAt.x * sin(step) + toLookAt.z * cos(step) + m_graphics->eyePos.z;
		}
	}
}

void Engine::eventHandler() {
	//Quit program
	if (m_event.type == SDL_QUIT
	    || m_event.type == SDL_KEYDOWN && m_event.key.keysym.sym == SDLK_ESCAPE) {
		m_running = false;
	}
	
	//Mouse - click and drag rotation
	else if (m_event.type == SDL_MOUSEBUTTONDOWN && !ImGui::GetIO().WantCaptureMouse) {
		switch (m_event.button.button) {
			case SDL_BUTTON_LEFT:
				mouseDown = true;
				break;
		}
	} else if (m_event.type == SDL_MOUSEBUTTONUP) {
		switch (m_event.button.button) {
			case SDL_BUTTON_LEFT:
				mouseDown = false;
				break;
		}
	} else if (m_event.type == SDL_MOUSEMOTION && mouseDown) {
		float scale = 360.0f / m_WINDOW_WIDTH;
		
		m_menu->setRotation(m_menu->options.rotation + m_event.motion.xrel * scale);
	}
	
	else if (m_event.type == SDL_MOUSEWHEEL && !ImGui::GetIO().WantCaptureMouse) {
		float step = 0.05 * m_menu->options.zoom;
		//Scroll down
		if(m_event.wheel.y > 0) {
			step *= -1;
		}
		
		m_menu->setZoom(m_menu->options.zoom + step);
	}
	
		//Window resize
	else if(m_event.type == SDL_WINDOWEVENT) {
		switch(m_event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				m_WINDOW_WIDTH = m_event.window.data1;
				m_WINDOW_HEIGHT = m_event.window.data2;
				
				//Update our projection matrix in case the aspect ratio is different now
				m_graphics->getProjection() = glm::perspective( 45.0f,
				                                                float(windowWidth)/float(windowHeight),
				                                                0.001f,
				                                                FAR_FRUSTRUM);
				
				//Tell OpenGL how large our window is now
				//SUPER IMPORTANT
				glViewport(0, 0, windowWidth, windowHeight);
				break;
		}
	}
}

unsigned int Engine::getDT() {
	long long TimeNowMillis = GetCurrentTimeMillis();
	assert(TimeNowMillis >= m_currentTimeMillis);
	unsigned int DeltaTimeMillis = (unsigned int) (TimeNowMillis - m_currentTimeMillis);
	m_currentTimeMillis = TimeNowMillis;
	return DeltaTimeMillis;
}

long long Engine::GetCurrentTimeMillis() {
	timeval t;
	gettimeofday(&t, NULL);
	long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
	return ret;
}
