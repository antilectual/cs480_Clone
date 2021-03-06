#ifndef CAMERA_H
#define CAMERA_H

#include "graphics_headers.h"
#include "Menu.h"

#define NEAR_FRUSTRUM 0.1f
#define FAR_FRUSTRUM 50.0f
#define FOV 45.0f

#define CAMERA_MODE_FOLLOW 1
#define CAMERA_MODE_FREE   2

struct CameraDirection {
	GLenum CubemapFace;
	glm::vec3 Target;
	glm::vec3 up;
};

class Camera {
	public:
		Camera(Menu& menu);
		
		~Camera();
		
		bool Initialize(int w, int h);
		
		glm::mat4& GetProjection();
		
		glm::mat4& GetView();
		
		//Calculate where the camera should be, and the corresponding View matrix
		void calculateCamera(unsigned dt);
		
		Menu& m_menu;
		
		CameraDirection* getCameraDirection();
		
		//Keeps track of where the camera is and what it's looking at
		glm::vec3 lookAt = glm::vec3(0.0, 0.0, 0.0);
		glm::vec3 eyePos;
		
		float tempZoom = 1;
		glm::vec2 screenShake = glm::vec2(0.0, 0.0);
		
		void moveTowards(glm::vec3 towards, unsigned time);
	
	private:
		
		CameraDirection gCameraDirections[4] =
				{
						{GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec3(0.0f, 1.0f, 0.0f)},
						{GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
						{GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)},
						{GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}
				};
		
		glm::mat4 projection;
		glm::mat4 view;
		
		glm::vec3 movingTowards = glm::vec3(0.0, 0.0, 0.0);
		unsigned movingTime = 0;
};

#endif /* CAMERA_H */
