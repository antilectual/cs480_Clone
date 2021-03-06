//
// Created by alex on 9/20/17.
//

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "graphics_headers.h"

#ifndef TUTORIAL_MODEL_H
#define TUTORIAL_MODEL_H

//Set up which texture channels to use with which type of texture
#define GL_COLOR_TEXTURE  GL_TEXTURE0
#define GL_ALT_TEXTURE    GL_TEXTURE1
#define GL_NORMAL_TEXTURE GL_TEXTURE2
//And the offset (for sending to the sampler)
#define GL_COLOR_TEXTURE_OFFSET  0
#define GL_ALT_TEXTURE_OFFSET    1
#define GL_NORMAL_TEXTURE_OFFSET 2

class Model {
	public:
		struct Material {
			glm::vec3 ambient = {0.2, 0.2, 0.2};  //Ka
			glm::vec3 diffuse = {0.8, 0.8, 0.8};  //Kd
			glm::vec3 specular = {0.0, 0.0, 0.0}; //Ks
			
			float shininess = 0.0f;               //Ns
		};
		static Model* load(std::string filename);
		
		std::vector<Vertex> _vertices;
		std::vector<unsigned int> _indices;
		
		Material material;
		
	private:
		Model();
		
		std::unordered_map<std::string, Material> materialList;
};

class Texture {
	public:
		static Texture* load(std::string filename);
		
		void initGL();
		void bind(GLenum textureTarget);
		
	private:
		Texture();
		
		bool initialised;
		
		GLuint m_textureObj;
		
		Magick::Image* m_Image;
		Magick::Blob* m_Blob;
};
#endif //TUTORIAL_MODEL_H
