/*
  ==============================================================================

    PostProcessing.h
    Created: 13 May 2018 2:50:45pm

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "FrameBuffer.h"
#include "ShaderBase.h"

/**
 * Vertex for 2d rendering on screen
 */
struct Vertex2D {
	float position[3];  // position of the vertex
	float color[4];    //color of the vertex
	float texCoord[2];  // coordinate of texture
};

class TextureInput {
public:
	int index = 0;
	std::shared_ptr<FrameBuffer> input;
	std::shared_ptr<OpenGLShaderProgram::Uniform> textureUniform;
	std::string name;
};


/**
 * Rendering 2d to the screen space
 * Used for combining default scene with bloom filter
 */
class ScreenShader : public ShaderBase {
public:
	glm::mat4 projection;
	int width;
	int height;

	std::vector<TextureInput> inputs;

	std::shared_ptr<FrameBuffer> output;

	void addInput(std::shared_ptr<FrameBuffer> inputFrame) {
		shader->use();

		std::stringstream name;
		name << "texture" << inputs.size();

		TextureInput input;
		input.name = name.str();
		input.textureUniform = std::shared_ptr<OpenGLShaderProgram::Uniform>
			(createUniform(input.name.c_str()));
		input.index = static_cast<int>(inputs.size());
		input.input = inputFrame;
		inputs.push_back(input);
	}

	void resize(int width, int height) {
		float w = (float) width;
		float h = (float) height;

		projection = glm::orthoLH(0.0f, w, 0.0f, h, -1.0f, 1.0f);
		this->width = width;
		this->height = height;


		vertices.clear();
		vertices.add(Vertex2D{0, 0, 0, 1, 1, 1, 1, 0, 0});
		vertices.add(Vertex2D{w, 0, 0, 1, 1, 1, 1, 1.0f, 0});
		vertices.add(Vertex2D{w, h, 0, 1, 1, 1, 1, 1.0f, 1.0f});
		vertices.add(Vertex2D{0.0f, h, 0, 1, 1, 1, 1, 0.0f, 1.0f});

		openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER,
																					vertices.size() * sizeof(Vertex2D),
																					vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);


	}

	ScreenShader(OpenGLContext &openGLContext,
							 int width, int height,
							 FrameBuffer *output = nullptr,
							 const char *frag = nullptr,
							 const char *vert = nullptr) : ShaderBase(openGLContext), output(output) {
		if (vert == nullptr)
			vert = BinaryData::screenVert_glsl;

		if (frag == nullptr)
			frag = BinaryData::screenFrag_glsl;

		createShader(frag, vert);

		resize(width, height);

		shader->use();

		projectionMatrix = createUniform("projectionMatrix");
		modelMatrix = createUniform("modelMatrix");

		vertex = createAttribute("vertex");
		color = createAttribute("color");
		textureCoord = createAttribute("textureCoord");
	}

	void begin() {
		if (output == nullptr) {
			openGLContext.extensions.glBindFramebuffer(GL_FRAMEBUFFER, openGLContext.getFrameBufferID());
		} else {
			output->bind();
		}

		openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		OpenGLHelpers::clear(Colour());

		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, width, height);*/

		Vertex2D *k = 0;

		shader->use();
		if (projectionMatrix != nullptr)
			projectionMatrix->setMatrix4(glm::value_ptr(projection), 1, false);

		if (vertex != nullptr) {
			openGLContext.extensions.glVertexAttribPointer(vertex->attributeID, 3, GL_FLOAT,
																										 GL_FALSE, sizeof(Vertex2D), &k->position);
			openGLContext.extensions.glEnableVertexAttribArray(vertex->attributeID);
		}

		if (color != nullptr) {
			openGLContext.extensions.glVertexAttribPointer(color->attributeID, 4, GL_FLOAT,
																										 GL_FALSE, sizeof(Vertex2D), &k->color);
			openGLContext.extensions.glEnableVertexAttribArray(color->attributeID);
		}


		if (textureCoord != nullptr) {
			openGLContext.extensions.glVertexAttribPointer(textureCoord->attributeID, 2,
																										 GL_FLOAT,
																										 GL_FALSE, sizeof(Vertex2D), &k->texCoord);

			openGLContext.extensions.glEnableVertexAttribArray(textureCoord->attributeID);
		}

		for (auto &uniform: uniforms) {
			uniform->apply();
		}
	}

	void end() {
		openGLContext.extensions.glBindFramebuffer(GL_FRAMEBUFFER, openGLContext.getFrameBufferID());
	}

	void pass() {
		glm::mat4 identity = glm::mat4();

		if (modelMatrix != nullptr)
			modelMatrix->setMatrix4(glm::value_ptr(identity), 1, false);


		for (auto &input : inputs) {
			if (input.textureUniform != nullptr)
				input.textureUniform->set(input.index);

			glActiveTexture(GL_TEXTURE0 + input.index);
			glBindTexture(GL_TEXTURE_2D, input.input->getTextureID());
		}

		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size());
	}

	void render() {
		begin();
		pass();
		end();
	}

	ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, modelMatrix;
	ScopedPointer<OpenGLShaderProgram::Attribute> vertex, color, textureCoord;
private:
	Array<Vertex2D> vertices;
	GLuint vertexBuffer;
};

class PostProcessingEffect {
public:
	std::vector<std::shared_ptr<FrameBuffer>> inputs;
	std::shared_ptr<FrameBuffer> output;
	std::unique_ptr<ScreenShader> screenShader;
};

class PostProcessing {
public:
	int width, height;
	OpenGLContext &openGLContext;
	std::vector<std::shared_ptr<PostProcessingEffect>> effects;

	PostProcessing(OpenGLContext &openGLContext, int width, int height) :
		openGLContext(openGLContext), width(width), height(height) {
	}

	std::shared_ptr<PostProcessingEffect> create(std::vector<std::shared_ptr<FrameBuffer>> inputs,
																							 const char *frag, bool toScreen = false) {
		std::shared_ptr<PostProcessingEffect> result = std::make_shared<PostProcessingEffect>();
		result->inputs = inputs;
		if (!toScreen) {
			result->output = std::make_shared<FrameBuffer>(openGLContext, width, height, false, false);
		}
		result->screenShader = std::unique_ptr<ScreenShader>(
			new ScreenShader(openGLContext, width, height, result->output.get(), frag));
		for (auto &input: inputs) {
			result->screenShader->addInput(input);
		}

		effects.push_back(result);
		return result;
	}

	void render() {
		for (auto &fx: effects) {
			fx->screenShader->render();
		}
	}
};