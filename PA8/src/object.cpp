#include <Menu.h>
#include "object.h"

glm::mat4* Object::viewMatrix;
glm::mat4* Object::projectionMatrix;
Menu* Object::menu;
int Object::idCounter = 1;

Object::Object(const Context &a) : ctx(a), originalCtx(a), position(_position) {
	//Default value - just in case anything tries to read it
	//The planet doesn't actually start here - this will be updated in Update()
	_position = {a.xLoc, a.yLoc, a.zLoc};
	modelMat = glm::translate(modelMat, position);
	ctx.id = idCounter;
	idCounter++;
}

Object::~Object() {}

void Object::Init_GL(std::unordered_map<std::string, std::string> const * dictionary) {
	//Initialise shaders
	ctx.shader->Initialize(dictionary);
	
	//Initialise models
	if(ctx.model != nullptr)
	{
		ctx.model->initGL();
	}

	//Initialise textures
	if(ctx.texture != nullptr) {
		ctx.texture->initGL();
	}
	if(ctx.altTexture != nullptr) {
		ctx.altTexture->initGL();
	}
	if(ctx.normalMap != nullptr) {
		ctx.normalMap->initGL();
	}
	if(ctx.specularMap != nullptr) {
		ctx.specularMap->initGL();
	}
}

void Object::Update(float dt) {
	if(ctx.hasPhysics)
	{
		btTransform transformObject;
		ctx.physicsBody->getMotionState()->getWorldTransform(transformObject);

		//16 element matrix
		float mat[16];
		transformObject.getOpenGLMatrix(mat);
		modelMat = glm::make_mat4(mat);
		modelMat = glm::scale(modelMat, glm::vec3(ctx.scale, ctx.scale, ctx.scale));
		
		_position = glm::vec3(modelMat * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}
}

const glm::mat4& Object::GetModel() const {
	return modelMat;
}

void Object::Render() const {
	//Send our shaders the MVP matrices
	glm::mat4 modelViewMatrix = *viewMatrix * modelMat;
	ctx.shader->uniformMatrix4fv("modelMatrix", 1, GL_FALSE, glm::value_ptr(modelMat));
	ctx.shader->uniformMatrix4fv("viewMatrix", 1, GL_FALSE, glm::value_ptr(*viewMatrix));
	ctx.shader->uniformMatrix4fv("projectionMatrix", 1, GL_FALSE, glm::value_ptr(*projectionMatrix));
	ctx.shader->uniformMatrix4fv("modelViewMatrix", 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
	
	//Send the material information
	ctx.shader->uniform3fv("MaterialAmbientColor", 1, &ctx.model->material.ambient.r);
	ctx.shader->uniform3fv("MaterialDiffuseColor", 1, &ctx.model->material.diffuse.r);
	ctx.shader->uniform3fv("MaterialSpecularColor", 1, &ctx.model->material.specular.r);
	ctx.shader->uniform1fv("shininess", 1, &ctx.model->material.shininess);
	
	//If we have a texture, use it
	if(ctx.texture != nullptr) {
		ctx.texture->bind(GL_COLOR_TEXTURE);
		ctx.shader->uniform1i("gSampler", GL_COLOR_TEXTURE_OFFSET);
	}
	if(ctx.altTexture != nullptr) {
		ctx.altTexture->bind(GL_ALT_TEXTURE);
		ctx.shader->uniform1i("gAltSampler", GL_ALT_TEXTURE_OFFSET);
	}
	if(ctx.normalMap != nullptr) {
		ctx.normalMap->bind(GL_NORMAL_TEXTURE);
		ctx.shader->uniform1i("gNormalSampler", GL_NORMAL_TEXTURE_OFFSET);
	}
	if(ctx.specularMap != nullptr) {
		ctx.specularMap->bind(GL_SPECULAR_TEXTURE);
		ctx.shader->uniform1i("gSpecularSampler", GL_SPECULAR_TEXTURE_OFFSET);
	}

	//Now draw our planet
	ctx.model->drawModel();

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Object::RenderID(Shader* shader) const {
	//Send our shaders the MVP matrices
	glm::mat4 MVPMatrix = *projectionMatrix * *viewMatrix * modelMat;
	shader->uniformMatrix4fv("MVP", 1, GL_FALSE, glm::value_ptr(MVPMatrix));
	float modifiedID = ctx.id;
	shader->uniform1fv("id", 1, &modifiedID);
	
	//Now draw our planet
	ctx.model->drawModel();
}