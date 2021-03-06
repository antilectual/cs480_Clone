//
// Created by alex on 9/20/17.
//
#ifndef MODEL
#define MODEL

#include "model.h"

Model* Model::load(std::string filename) {
	static std::unordered_map<std::string, Model*> loadedModels;
	
	//If we already loaded this model before, don't do it again
	if(loadedModels.find(filename) != loadedModels.end()) {
		return loadedModels[filename];
	}

	Model* newModel = new Model();
	
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(filename, aiProcessPreset_TargetRealtime_Fast);
	if(scene == nullptr) {
		std::cerr << "Could not load model from " << filename << std::endl;
		return nullptr;
	}
	
	
	for(int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		Mesh newMesh;
		
		loadVertices(mesh, &newMesh);
		loadIndices(mesh, &newMesh);
		newMesh.material = loadMaterials(scene, i);
		
		newModel->meshes.push_back(newMesh);
	}
	
	newModel->initialised = false;
	
	//Now save this model for later in case we need to use it again
	loadedModels[filename] = newModel;
	
	return newModel;
}

Model::Model(){}

void Model::initGL() {
	if(!initialised) {
		for(auto& i : meshes) {
			glGenBuffers(1, &i.VB);
			glBindBuffer(GL_ARRAY_BUFFER, i.VB);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * i._vertices.size(), &i._vertices[0], GL_STATIC_DRAW);
			
			glGenBuffers(1, &i.IB);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.IB);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * i._indices.size(), &i._indices[0], GL_STATIC_DRAW);
		}
		
		initialised = true;
	}
}

void Model::drawModel(Shader* shader) {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	
	for(auto& i : meshes) {
		if(shader != nullptr) {
			//Send the material information
			shader->uniform3fv("MaterialAmbientColor", 1, &i.material.ambient.r);
			shader->uniform3fv("MaterialDiffuseColor", 1, &i.material.diffuse.r);
			shader->uniform3fv("MaterialSpecularColor", 1, &i.material.specular.r);
			shader->uniform1fv("shininess", 1, &i.material.shininess);
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, i.VB);
		//Now send vertices, uvs, and normals
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void *) offsetof(Vertex, tangent));
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void *) offsetof(Vertex, bitangent));
		
		//Send all our face information
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.IB);
		
		//Now draw everything
		glDrawElements(GL_TRIANGLES, i._indices.size(), GL_UNSIGNED_INT, 0);
	}
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}

void Model::loadVertices(aiMesh *mesh, Mesh *newModel)
{
	//Add all our vertices for openGL
	aiVector3D uv = {0.0, 0.0, 0.0};
	aiVector3D tangent = {0.0, 0.0, 0.0};
	aiVector3D bitangent = {0.0, 0.0, 0.0};
	aiVector3D normal = {0.0, 0.0, 0.0};
	
	for(unsigned vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) 
	{
		aiVector3D& vertex = mesh->mVertices[vertexIndex];
		
		if(mesh->HasNormals()) {
			normal = mesh->mNormals[vertexIndex];
		}
		
		if(mesh->HasTextureCoords(0)) {
			uv = mesh->mTextureCoords[0][vertexIndex];
		}
		
		if(mesh->HasTangentsAndBitangents()) {
			tangent = mesh->mTangents[vertexIndex];
			bitangent = mesh->mBitangents[vertexIndex];
		}
		
		Vertex newVert = {{vertex.x, vertex.y, vertex.z},
		                  {uv.x, 1-uv.y}, // To account for OpenGL starting from the top left instead of the bottom left
		                  {normal.x, normal.y, normal.z},
		                  {tangent.x, tangent.y, tangent.z},
		                  {bitangent.x, bitangent.y, bitangent.z}};
		
		newModel->_vertices.push_back(newVert);
	}
}

void Model::loadIndices(aiMesh *mesh, Mesh *newModel)
{
	//Now our indices
	for(unsigned faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) 
	{
		aiFace& face = mesh->mFaces[faceIndex];
		
		for(unsigned vertexIndex = 0; vertexIndex < 3; vertexIndex++) 
		{
			newModel->_indices.push_back(face.mIndices[vertexIndex]);
		}
	}
}


Material Model::loadMaterials(const aiScene *scene, int meshIndex)
{
	//Now material stuff
	aiMaterial* material = scene->mMaterials[scene->mMeshes[meshIndex]->mMaterialIndex];
	aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
	aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
	aiColor3D specularColor(0.0f, 0.0f, 0.0f);
	
	material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
	material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
	
	Material modelMaterial;
	modelMaterial.ambient = {ambientColor.r, ambientColor.g, ambientColor.b};
	modelMaterial.diffuse = {diffuseColor.r, diffuseColor.g, diffuseColor.b};
	modelMaterial.specular = {specularColor.r, specularColor.g, specularColor.b};
	
	material->Get(AI_MATKEY_SHININESS, modelMaterial.shininess);
	
	return modelMaterial;
}

Texture* Texture::load(std::string filename) {
	static std::unordered_map<std::string, Texture*> loadedTextures;
	
	//If we already have the texture loaded, don't load it again
	if(loadedTextures.find(filename) != loadedTextures.end()) {
		return loadedTextures[filename];
	}
	Texture* newTex = new Texture();
	
	//Load our texture with ImageMagick
	try {
		newTex->m_Image = new Magick::Image(filename);
		newTex->m_Blob = new Magick::Blob();
		newTex->m_Image->write(newTex->m_Blob, "RGBA");
	} catch(Magick::Error& err) {
		std::cout << "Could not load texture \"" << filename <<"\": " << err.what() << std::endl;
		return nullptr;
	}
	
	newTex->initialised = false;
	
	//Now add it onto our list
	loadedTextures[filename] = newTex;
	
	return newTex;
}

void Texture::initGL() {
	if(!initialised) {
		//set up the texture with OpenGL
		glGenTextures(1, &m_textureObj);
		glBindTexture(GL_TEXTURE_2D, m_textureObj);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Image->columns(), m_Image->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Blob->data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		delete m_Image;
		delete m_Blob;
		m_Image = nullptr;
		m_Blob = nullptr;
		
		initialised = true;
	}
}

void Texture::bind(GLenum textureTarget) {
	glActiveTexture(textureTarget);
	
	glBindTexture(GL_TEXTURE_2D, m_textureObj);
}

Texture::Texture(){}

#endif /* model */