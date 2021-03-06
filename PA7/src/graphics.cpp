#include "graphics.h"

Graphics::Graphics(Object* sun, float lightStrength, Menu& menu, const int& w, const int& h) : windowWidth(w), windowHeight(h), m_cube(sun), lightPower(lightStrength), m_menu(menu) {
	Object::viewMatrix = &view;
	Object::projectionMatrix = &projection;
	Object::globalOffset = &m_cube->position;
	
	cameraMode = CAMERA_MODE_FOLLOW;
	
	//Load skybox stuff
	skybox = Model::load("models/Skybox.obj");
	skyShader = Shader::load("shaders/vert_sky", "shaders/frag_sky");
	skyTexture = Texture::load("textures/8k_stars_milky_way.jpg");
	
	shadowShader = Shader::load("shaders/vert_shadow", "shaders/frag_shadow");
}

Graphics::~Graphics() {

}

bool Graphics::Initialize(int width, int height) {
	// Used for the linux OS
#if !defined(__APPLE__) && !defined(MACOSX)
	// cout << glewGetString(GLEW_VERSION) << endl;
	glewExperimental = GL_TRUE;
	
	auto status = glewInit();
	
	// This is here to grab the error that comes from glew init.
	// This error is an GL_INVALID_ENUM that has no effects on the performance
	glGetError();
	
	//Check for error
	if (status != GLEW_OK) {
		std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
		return false;
	}
#endif
	
	// For OpenGL 3
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	m_cube->Init_GL();
	skybox->initGL();
	skyShader->Initialize();
	skyTexture->initGL();
	
	shadowShader->Initialize();
	
	glGenFramebuffers(1, &depthBuffer);
	
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	
	for (uint i = 0 ; i < 6 ; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 5120, 5120, 0, GL_RGB, GL_FLOAT, NULL);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, shadowMap, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap, 0);
	
	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 5120, 5120);
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
	
	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
	                               float(width)/float(height), //Aspect Ratio, so Circles stay Circular
	                               0.001f, //Distance to the near plane, normally a small value like this
	                               FAR_FRUSTRUM); //Distance to the far plane,
	
	return true;
}

void Graphics::Update(unsigned int dt) {
	glm::vec3 offsetChange = *Object::globalOffset;
	
	// Update the object
	m_cube->Update(dt, m_menu.options.scale, m_menu.options.drawOrbits);
	
	//Calculate what our offset changed by, in case we need to move the camera
	offsetChange -= *Object::globalOffset;
	
	//Calculate where our camera should be and update the View matrix
	calculateCamera(offsetChange);
}

void Graphics::Render() {
	if(m_menu.options.drawShadows && m_menu.options.scale == CLOSE_SCALE) renderShadowMap();
	
	//Switch to rendering on the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	float modifiedLight = pow(lightPower, m_menu.options.scale);
	
	//Render our background
	//Done first so that transparent objects (rings) work properly
	renderSkybox();
	
	//Decide if we should render orbits (and if so - what kinds)
	unsigned orbits;
	if(m_menu.options.drawOrbits) {
		if(m_menu.options.drawMoonOrbits) {
			orbits = DRAW_ALL_ORBITS;
		} else {
			orbits = DRAW_PLANET_ORBITS;
		}
	} else if(m_menu.options.drawMoonOrbits) {
		orbits = DRAW_MOON_ORBITS;
	} else {
		orbits = DRAW_NO_ORBITS;
	}
	
	if(m_menu.options.drawShadows) {
		glActiveTexture(GL_SHADOW_TEXTURE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
	}
	
	//Render planets
	m_cube->Render(modifiedLight, orbits, shadowMap);
	
	// Get any errors from OpenGL
	auto error = glGetError();
	if (error != GL_NO_ERROR) {
		std::string val = ErrorString(error);
		std::cout << "Error initializing OpenGL! " << error << ", " << val << std::endl;
	}
}

void Graphics::renderSkybox() {
	skyShader->Enable();
	
	//Make certain we can see the inside faces, because we will always be inside the skybox
	GLint OldCullFaceMode;
	glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
	GLint OldDepthFuncMode;
	glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
	
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	
	//Make certain the skybox is centered on the camera
	//We don't have to worry about making it big enough to cover everything
	//the shader will make certain it's always in the background
	glm::mat4 modelMat(1.0);
	modelMat = glm::translate(modelMat, eyePos);
	modelMat = glm::scale(modelMat, glm::vec3(20, 20, 20));
	
	glm::mat4 MVPMatrix = projection * view * modelMat;
	skyShader->uniformMatrix4fv("MVPMatrix", 1, GL_FALSE, glm::value_ptr(MVPMatrix));
	
	skyTexture->bind(GL_COLOR_TEXTURE);
	skyShader->uniform1i("gSampler", GL_COLOR_TEXTURE_OFFSET);
	
	skybox->drawModel();
	
	glCullFace(OldCullFaceMode);
	glDepthFunc(OldDepthFuncMode);
}

void Graphics::renderShadowMap() {
	shadowShader->Enable();
	
	glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer);
	glViewport(0,0,5120,5120);
	
	glCullFace(GL_FRONT);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	//glm::mat4 oldView = view;
	//glm::mat4 oldProj = projection;
	
	glm::vec3 offset = *Object::globalOffset;
	offset *= -1;
	
	lprojection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 100.0f);
	shadowShader->uniform3fv("lightW3", 1, &offset.x);
	
	glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	for (int i = 0; i < 4; i++) {
		lshadowTrans[i] = lprojection * glm::lookAt(offset, offset + gCameraDirections[i].Target, gCameraDirections[i].up);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gCameraDirections[i].CubemapFace, shadowMap, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		shadowShader->uniformMatrix4fv("VP", 1, GL_FALSE, glm::value_ptr(lshadowTrans[i]));
		for(int j = 0; j < m_cube->getNumChildren(); j++) {
			(*m_cube)[j].renderShadow(shadowShader);
		}
	}
	
	//view = oldView;
	//projection = oldProj;
	glViewport(0, 0, windowWidth, windowHeight);
}

void Graphics::calculateCamera(glm::vec3 offsetChange) {
	
	if(cameraMode == CAMERA_MODE_FOLLOW) {
		//What we're looking at
		glm::vec3 lookVec;
		//What should be in the background (whatever we're orbiting)
		glm::vec3 backgroundVec;
		Object* lookingAt = m_menu.getPlanet(m_menu.options.lookingAt);
		Object* parent = lookingAt->parent;
		
		//Keep track of how large whatever we're looking at is
		//float scale = sqrt(lookingAt->ctx.scale) * 250;
		//float scale = lookingAt->ctx.scale * 15 * lookingAt->ctx.scaleMultiplier;
		float scale = lookingAt->ctx.scaleMultiplier / pow(lookingAt->ctx.scaleMultiplier, m_menu.options.scale) *
		              pow(lookingAt->ctx.scale, m_menu.options.scale) * 15;
		
		//Find the coordinates of whatever the thing we're looking at is and whatever it is orbiting
		lookVec = lookingAt->position - *Object::globalOffset;
		
		//If we're orbiting something, put that something in the background of the camera
		if (parent != NULL) {
			backgroundVec = parent->position - *Object::globalOffset;
		} else {
			backgroundVec = lookVec + glm::vec3(0.0, 0.0, -5.0) - *Object::globalOffset;
		}
		
		//Now do some math to find where to place the camera
		//First find the direction pointing from what we're orbiting to what we're looking at
		backgroundVec = glm::normalize(lookVec - backgroundVec);
		
		glm::vec3 crossVec = glm::normalize(
				glm::cross(glm::vec3(backgroundVec.x, backgroundVec.y, backgroundVec.z), glm::vec3(0.0, 1.0, 0.0)));
		
		float angle = m_menu.options.rotation * M_PI / 180;
		backgroundVec = cos(angle) * backgroundVec + sin(angle) * glm::vec3(crossVec.x, crossVec.y, crossVec.z);
		//Then scale it depending on how large what we're looking at is
		//We don't want to be to far away from a small object or too close to a large object
		backgroundVec *= scale * m_menu.options.zoom;
		//Then add it back to the location of whatever we were looking at to angle the camera in front of what we're looking at AND what it's orbiting
		backgroundVec += lookVec;
		
		eyePos = glm::vec3(backgroundVec.x, backgroundVec.y + 0.5 * scale * m_menu.options.zoom * m_menu.options.zoom,
		                   backgroundVec.z);
		lookAt = lookVec;
	} else if(cameraMode == CAMERA_MODE_FREE) {
		eyePos += offsetChange;
		lookAt += offsetChange;
	}
	
	//Also let's try and look down from above what we're looking at
	view = glm::lookAt( eyePos, //Eye Position
                        lookAt, //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up
}

std::string Graphics::ErrorString(GLenum error) {
	if (error == GL_INVALID_ENUM) {
		return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
	} else if (error == GL_INVALID_VALUE) {
		return "GL_INVALID_VALUE: A numeric argument is out of range.";
	} else if (error == GL_INVALID_OPERATION) {
		return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
	} else if (error == GL_INVALID_FRAMEBUFFER_OPERATION) {
		return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
	} else if (error == GL_OUT_OF_MEMORY) {
		return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
	} else {
		return "None";
	}
}

Object *Graphics::getCube() {
	return m_cube;
}

glm::mat4& Graphics::getProjection() {
	return projection;
}