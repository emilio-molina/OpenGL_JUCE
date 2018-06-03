/*
 ==============================================================================
 
 ScatterPlot.h
 Created: 27 Jan 2018 4:34:23pm
 Author:  Emilio Molina
 
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <algorithm>

#include <chrono>
#include "Camera.hpp"
#include "ShaderBase.h"
#include "FrameBuffer.h"
#include "PostProcessing.h"
#include "PixelBuffer.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif


/* Vertex struct for OpenGL buffers
 *
 */
struct Vertex {
	float position[3];  // position of the vertex
	float normal[3];    // normal vector of the vertex
	float texCoord[2];  // coordinate of texture
};


/* Class to generate vertices of a sphere.
 * taken from:
 * https://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c/5989676#5989676
 */
class SolidSphereGenerator {
public:
	std::vector<Vertex> templateVertices;
	std::vector<int> templateIndices;

	/* Create a single-sphere template with specific number of rings and sectors
	 *
	 */
	SolidSphereGenerator(unsigned int rings,
											 unsigned int sectors);

	/* Append to vertices, sphereIds, and indices all needed information to
	 * create a new sphere.
	 */
	void generateSphere(int sphereId, float offsetX, float offsetY, float offsetZ,
											float colourR, float colourG, float colourB, float alpha,
											float radius, Array<Vertex> &vertices, Array<int> &sphereIds,
											Array<int> &indices);
};

struct Move {
	bool enabled = false;
	bool finished = false;
	glm::vec3 from;
	glm::vec3 to;
	float start;
	float duration = 1.0f;

	float ease(float p) {
		float f = (p - 1);
		return f * f * f + 1;
	}

	float getTime() {
		return (float) Time::getMillisecondCounter() / 1000.0f;
	}

	void reset() {
		finished = false;
		enabled = false;
	}

	void animate(glm::vec3 from, glm::vec3 to, float duration) {
		this->from = from;
		this->to = to;
		this->duration = duration;
		enabled = true;
		finished = false;
		start = getTime();
	}

	void reanimate(glm::vec3 to, float duration) {
		animate(this->to, to, duration);
	}

	static inline float lerp(float a0, float a1, float w) {
		return (1.0f - w) * a0 + w * a1;
	}

	glm::vec3 animate() {
		float time = getTime() - start;
		float w = ease(time / duration);
		if (w > duration) {
			finished = true;
			return to;
		}
		glm::vec3 result(
			lerp(from.x, to.x, w),
			lerp(from.y, to.y, w),
			lerp(from.z, to.z, w)
		);
		return result;
	}
};

struct SphereInfo {
	int sphereId;
	float x;
	float y;
	float z;
	float radius;
	float r;
	float g;
	float b;
	bool hover;
	bool shake;

	Move animation;
};


class SelectShader : public ShaderBase {
public:
	std::vector<PixelARGB> pixels;
	std::shared_ptr<FrameBuffer> selectFrameBuffer;
	uint32_t maxIndexes = 262144;

	uint32_t getColorByIndex(int index) {
		if (index > maxIndexes) {
			throw std::logic_error("Index should't be larger than maxIndex");
		}
		uint32_t stepColor = (uint32_t) 0xFFFFFF / (uint32_t) (maxIndexes);
		return stepColor + (stepColor * index);
	}

	int getIndexByColor(uint32_t color) {
		if (color == 0)
			return -1;

		uint32_t stepColor = (uint32_t) 0xFFFFFF / (uint32_t) (maxIndexes);
		uint32_t index = (color / stepColor) - 1;
		if (index > maxIndexes) {
			throw std::logic_error("Index should't be larger than maxIndex");
		}
		return index;
	}

	void begin() {
		shader->use();
		selectFrameBuffer->makeCurrentAndClear();
	}

	void captureOld() {
		auto start = Time::getMillisecondCounter();
		selectFrameBuffer->readPixels(pixels.data(), Rectangle<int>(0, 0, selectFrameBuffer->getWidth(),
																																selectFrameBuffer->getHeight()));
		auto took = Time::getMillisecondCounter() - start;
		//std::cout << "Old technique took:" << took << "ms" << std::endl;
	}

	void captureNew() {
		auto start2 = Time::getMillisecondCounter();


		apbo->read();
		auto bytes = apbo->map();
		if (bytes != nullptr) {
			memcpy(pixels.data(), bytes, apbo->getDataSize());
			apbo->unmap();
			auto took2 = Time::getMillisecondCounter() - start2;
			//std::cout << "New technique took:" << took2 << "ms" << std::endl;
		} else {
			std::cout << "Got null ptr" << std::endl;
		}
	}

	void end() {
		int pixelCount = selectFrameBuffer->getWidth() * selectFrameBuffer->getHeight();
		pixels.resize(pixelCount);

		captureNew();

		selectFrameBuffer->unbind();
	}


	SelectShader(OpenGLContext &openGLContext, int width, int height) : ShaderBase(openGLContext) {

		apbo = std::shared_ptr<AlternatingPixelBuffer>
			(new AlternatingPixelBuffer(openGLContext, width, height));

		selectFrameBuffer = std::shared_ptr<FrameBuffer>
			(new FrameBuffer(openGLContext, width, height, true, false));


		createShader(BinaryData::selectFragment_glsl, BinaryData::selectVertex_glsl);

		projectionMatrix = createUniform("projectionMatrix");
		viewMatrix = createUniform("viewMatrix");
		modelMatrix = createUniform("modelMatrix");
		color = createUniform("color");

		vertex = createAttribute("vertex");
	}

	std::shared_ptr<AlternatingPixelBuffer> apbo;

	ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, modelMatrix, color;
	ScopedPointer<OpenGLShaderProgram::Attribute> vertex;
};

class MainShader : public ShaderBase {
public:
	std::shared_ptr<FrameBuffer> output; // will be used by bloom later

	MainShader(OpenGLContext &openGLContext) : ShaderBase(openGLContext) {
		createShader(BinaryData::fragmentShader_glsl, BinaryData::vertexShader_glsl);

		shader->use();

		projectionMatrix = createUniform("projectionMatrix");
		viewMatrix = createUniform("viewMatrix");
		modelMatrix = createUniform("modelMatrix");
		normalMatrix = createUniform("normalMatrix");
		color = createUniform("color");
		lightPosition = createUniform("lightPosition");
		lightColor = createUniform("lightColor");
		eyePosition = createUniform("eyePosition");

		vertex = createAttribute("vertex");
		normal = createAttribute("normal");
		textureCoord = createAttribute("textureCoord");
	}

	ScopedPointer<OpenGLShaderProgram::Uniform>
		projectionMatrix, viewMatrix, modelMatrix, color, lightPosition, lightColor, eyePosition,
		normalMatrix;
	ScopedPointer<OpenGLShaderProgram::Attribute> vertex, normal, textureCoord;
};


/* Component for scatterplot of some thousands of spheres
 *
 */
class ScatterPlot : public OpenGLAppComponent {
public:

	OpenGLFrameBuffer fb;
	Camera *_camera;

	std::vector<SphereInfo> spheres;

	ScatterPlot();

	~ScatterPlot();

	void initialise() override;

	void shutdown() override;

	void render() override;

	void selectRender();

	void mainRender();

	void postProcessRender();

	void auxRender1();

	void auxRender2();

	void auxRender3();

	void paint(Graphics &) override;

	void resized() override;

	void mouseDown(const MouseEvent &e) override;

	void mouseMove(const MouseEvent &e) override;

	void mouseDrag(const MouseEvent &e) override;

	void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &d) override;

	Matrix3D<float> getProjectionMatrix() const;

	Matrix3D<float> getViewMatrix() const;

	glm::mat4 getProjectionMatrixGlm() const;

	glm::mat4 getViewMatrixGlm() const;

	void createLambertShader();

	void createHoverShader();

	void callbackHover(int sphereId);  // TODO
private:


	ScopedPointer<SelectShader> selectShader;
	ScopedPointer<MainShader> mainShader;
	ScopedPointer<PostProcessing> pp;

	Random r;
	Draggable3DOrientation draggableOrientation;
	float draggingX;
	float draggingY;
	bool initialized;
	// OpenGL variables:
	Array<Vertex> vertices;
	Array<int> indices;
	Array<int> sphereId;
	GLuint vertexBuffer, indexBuffer;
	float zoomValue;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScatterPlot)
};
