#include "shader.h"

std::unordered_map<std::string, Shader*> Shader::loadedShaders;

Shader::Shader() {
	m_shaderProg = 0;
}

Shader::~Shader() {
	for (std::vector<GLuint>::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
		glDeleteShader(*it);
	}
	
	if (m_shaderProg != 0) {
		glDeleteProgram(m_shaderProg);
		m_shaderProg = 0;
	}
}

bool Shader::Initialize() {
	if(!initialised) {
		m_shaderProg = glCreateProgram();
		
		if (m_shaderProg == 0) {
			std::cerr << "Error creating shader program\n";
			return false;
		}
		
		if (!AddShader(GL_VERTEX_SHADER, vertexShader)) return false;
		if (!AddShader(GL_FRAGMENT_SHADER, fragmentShader)) return false;
		
		initialised = true;
		
		return Finalize();
	}
	
	return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool Shader::AddShader(GLenum ShaderType, const std::string &s) {
	GLuint ShaderObj = glCreateShader(ShaderType);
	
	if (ShaderObj == 0) {
		std::cerr << "Error creating shader type " << ShaderType << std::endl;
		return false;
	}
	
	// Save the shader object - will be deleted in the destructor
	m_shaderObjList.push_back(ShaderObj);
	
	const GLchar *p[1];
	p[0] = s.c_str();
	GLint Lengths[1] = {(GLint) s.size()};
	
	glShaderSource(ShaderObj, 1, p, Lengths);
	
	glCompileShader(ShaderObj);
	
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error with " << s << std::endl;
		std::cerr << "Error compiling: " << InfoLog << std::endl;
		return false;
	}
	
	glAttachShader(m_shaderProg, ShaderObj);
	
	return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Shader::Finalize() {
	GLint Success = 0;
	GLchar ErrorLog[1024] = {0};
	
	glLinkProgram(m_shaderProg);
	
	glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), nullptr, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		return false;
	}
	
	glValidateProgram(m_shaderProg);
	glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), nullptr, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		return false;
	}
	
	// Delete the intermediate shader objects that have been added to the program
	for (auto it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++) {
		glDeleteShader(*it);
	}
	
	m_shaderObjList.clear();
	
	return true;
}


void Shader::Enable() {
	glUseProgram(m_shaderProg);
}


GLint Shader::GetUniformLocation(const char *pUniformName) {
	GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);
	
	return Location;
}

Shader *Shader::load(std::string vertexLocation, std::string fragmentLocation) {
	std::string key = vertexLocation + ", " + fragmentLocation;
	
	if(loadedShaders.find(key) != loadedShaders.end()) {
		return loadedShaders[key];
	}
	
	auto * newShader = new Shader();
	
	std::ifstream vertexFile(vertexLocation);
	if (!vertexFile.is_open()) {
		std::cerr << "Could not find vertex shader file: " << vertexLocation << std::endl;
		return nullptr;
	}
	newShader->vertexShader = std::string(std::istreambuf_iterator<char>(vertexFile), {});
	
	std::ifstream fragFile(fragmentLocation);
	if (!fragFile.is_open()) {
		std::cerr << "Could not find fragment shader file: " << fragmentLocation << std::endl;
		return nullptr;
	}
	newShader->fragmentShader = std::string(std::istreambuf_iterator<char>(fragFile), {});
	
	
	newShader->initialised = false;
	loadedShaders[key] = newShader;
	
	return newShader;
}

bool Shader::uniform1fv(const char* uniform, GLsizei size, const GLfloat* value) {
	GLint location = GetUniformLocation(uniform);
	if(location == INVALID_UNIFORM_LOCATION) return false;
	
	glUniform1fv(location, size, value);
	return true;
}

bool Shader::uniform3fv(const char* uniform, GLsizei size, const GLfloat* value) {
	GLint location = GetUniformLocation(uniform);
	if(location == INVALID_UNIFORM_LOCATION) return false;
	
	glUniform3fv(location, size, value);
	return true;
}

bool Shader::uniform1i(const char* uniform, GLint value) {
	GLint location = GetUniformLocation(uniform);
	if(location == INVALID_UNIFORM_LOCATION) return false;
	
	glUniform1i(location, value);
	return true;
}

bool Shader::uniformMatrix4fv(const char* uniform, GLsizei size, GLboolean transpose, const GLfloat* value) {
	GLint location = GetUniformLocation(uniform);
	if(location == INVALID_UNIFORM_LOCATION) return false;
	
	glUniformMatrix4fv(location, size, transpose, value);
	return true;
}