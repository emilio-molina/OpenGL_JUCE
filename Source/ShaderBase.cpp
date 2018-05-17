/*
  ==============================================================================

    ShaderBase.cpp
    Created: 9 May 2018 7:01:04am

  ==============================================================================
*/

#include "ShaderBase.h"
#include "../JuceLibraryCode/JuceHeader.h"

void ShaderBase::createShader(const char *frag, const char *vert) {
	shader = new OpenGLShaderProgram(openGLContext);
	shader->addShader(frag, GL_FRAGMENT_SHADER);
	shader->addShader(vert, GL_VERTEX_SHADER);
	if (!shader->link()) {
		status = shader->getLastError();
		std::cout << status << std::endl;
	}
}

OpenGLShaderProgram::Uniform *ShaderBase::createUniform(const char *uniform) {
	auto result = openGLContext.extensions.glGetUniformLocation(this->shader->getProgramID(),
																															uniform);
	if (result < 0) {
		std::cerr << "Coulnd't get uniform location for '" << uniform << "':" << result << std::endl;
		return nullptr;
	}

	return new OpenGLShaderProgram::Uniform(*shader, uniform);
}

OpenGLShaderProgram::Attribute *ShaderBase::createAttribute(const char *attrib) {
	auto result = openGLContext.extensions.glGetAttribLocation(this->shader->getProgramID(), attrib);
	if (result < 0) {
		std::cerr << "Coulnd't get attribute location for '" << attrib << "':" << result << std::endl;
		return nullptr;
	}
	return new OpenGLShaderProgram::Attribute(*shader, attrib);
}