#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "Menu.h"

class Graphics {
	public:
		Graphics(Object *sun);
		
		~Graphics();
		
		//Initialise OpenGL
		bool Initialize(int width, int height, std::string vertexShader, std::string fragmentShader);
		//Update physics and models
		void Update(unsigned int dt);
		//Render models
		void Render(const Menu &);
		
		//Return pointer to main Object (sun)
		Object *getCube();
	
	private:
		std::string ErrorString(GLenum error);
		
		Camera *m_camera;
		Shader *m_shader;
		
		//Where our matrices are in video memory
		GLint m_projectionMatrix;
		GLint m_viewMatrix;
		GLint m_modelMatrix;
		
		//The sun
		Object *m_cube;
};

#endif /* GRAPHICS_H */
