/*
  ==============================================================================

    ShaderBase.h
    Created: 9 May 2018 7:01:04am

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class ShaderBase;

class ShaderUniform {
public:
	enum UniformTypes {
		UT_FLOAT,
		UT_INT,
		UT_VEC3,
		UT_VEC4,
		UT_MAT4
	};

	ShaderUniform(ShaderBase &shader, std::string name,
								OpenGLShaderProgram::Uniform *uniform)
		: shader(shader), uniform(uniform), name(name) {

	}

	ShaderUniform &operator=(float floatValue) {
		ShaderUniform::floatValue = floatValue;
		type = UT_FLOAT;
		return *this;
	}

	ShaderUniform &operator=(int intValue) {
		ShaderUniform::intValue = intValue;
		type = UT_INT;
		return *this;
	}

	ShaderUniform &operator=(const glm::vec3 &vec3Value) {
		ShaderUniform::vec3Value = vec3Value;
		type = UT_VEC3;
		return *this;
	}

	ShaderUniform &operator=(const glm::vec4 &vec4Value) {
		ShaderUniform::vec4Value = vec4Value;
		type = UT_VEC4;
		return *this;
	}

	ShaderUniform &operator=(const glm::mat4 &mat4Value) {
		ShaderUniform::mat4Value = mat4Value;
		type = UT_MAT4;
		return *this;
	}

	void apply() {
		if (uniform != nullptr) {
			switch (type) {
				case UT_FLOAT:
					uniform->set(floatValue);
					break;

				case UT_INT:
					uniform->set(intValue);
					break;

				case UT_VEC3:
					uniform->set(vec3Value.x, vec3Value.y, vec3Value.z);
					break;

				case UT_VEC4:
					uniform->set(vec4Value.x, vec4Value.y, vec4Value.z, vec4Value.w);
					break;

				case UT_MAT4:
					uniform->setMatrix4(glm::value_ptr(mat4Value), 1, GL_FALSE);
					break;
			}
		}
	}

protected:
	friend class ShaderBase;

	float floatValue;
	int intValue;
	glm::vec3 vec3Value;
	glm::vec4 vec4Value;
	glm::mat4 mat4Value;

	std::string name;
	ShaderBase &shader;
	UniformTypes type;

	std::shared_ptr<OpenGLShaderProgram::Uniform> uniform;
};


class ShaderBase {
public:
	OpenGLContext &openGLContext;
	ScopedPointer<OpenGLShaderProgram> shader;
	String status;
	std::vector<std::shared_ptr<ShaderUniform>> uniforms;

	ShaderUniform &addUniform(std::string name) {
		shader->use();
		auto gl_uniform = createUniform(name.c_str());
		auto uniform = std::make_shared<ShaderUniform>(*this, name, gl_uniform);
		uniforms.push_back(uniform);
		return *uniform;
	}

	template<class T>
	void addUniform(std::string name, T value) {
		auto &uniform = addUniform(name);
		uniform = value;
	}

	ShaderUniform &operator[](const char *name) {
		for (auto &uniform: uniforms) {
			if (uniform->name == name) {
				return *uniform;
			}
		}
		throw std::logic_error("invalid uniform name");
	}

	ShaderBase(OpenGLContext &openGLContext) : openGLContext(openGLContext) {
	}


protected:
	void createShader(const char *frag, const char *vert);

	OpenGLShaderProgram::Uniform *createUniform(const char *uniform);

	OpenGLShaderProgram::Attribute *createAttribute(const char *attrib);
};
