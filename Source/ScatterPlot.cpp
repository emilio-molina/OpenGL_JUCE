/*
 ==============================================================================
 
 ScatterPlot.cpp
 Created: 27 Jan 2018 4:34:23pm
 Author:  Emilio Molina
 
 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"
#include "ScatterPlot.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace glm;

Matrix3D<float> glmToJuce(glm::mat4 mat) {
	Matrix3D<float> result(glm::value_ptr(mat));
	return result;
}


/* Generate a template for one sphere
 *
 * @param rings Number of rings in mesh
 * @param sectors Number of sectors in mesh
 */
SolidSphereGenerator::SolidSphereGenerator(unsigned int rings,
																					 unsigned int sectors) {
	printf("%f %f\n", M_PI, M_PI_2);
	float const R = 1. / (float) (rings - 1);
	float const S = 1. / (float) (sectors - 1);
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			float y = sin(-M_PI_2 + M_PI * r * R);
			float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);
			templateVertices.push_back(
				{
					{x,     y, z},
					{x,     y, z},
					{s * S, r * R}
				}
			);
		}
	}
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			templateIndices.push_back(r * sectors + s); // 1
			templateIndices.push_back(r * sectors + (s + 1)); // 2
			templateIndices.push_back((r + 1) * sectors + (s + 1)); // 3


			templateIndices.push_back((r + 1) * sectors + (s + 1)); // 3
			templateIndices.push_back((r + 1) * sectors + s); // 4
			templateIndices.push_back(r * sectors + s); // 1
		}
	}
}

/* Generate a new sphere with specific colour and size
 *
 * @param sphereId Id given to this new sphere
 * @param offsetX X position of center
 * @param offsetY Y position of center
 * @param offsetZ Z position of center
 * @param alpha Transparency
 * @param radius Radius of sphere
 * @param vertices Output array where sphere vertices will be appended
 * @param sphereIds Output array where each vertex index is assigned an sphereId
 * @param indices Output array with indices
 */
void SolidSphereGenerator::generateSphere(int sphereId, float offsetX,
																					float offsetY, float offsetZ,
																					float colourR, float colourG,
																					float colourB, float alpha,
																					float radius, Array<Vertex> &vertices,
																					Array<int> &sphereIds, Array<int> &indices) {
	int startIndex = (int) vertices.size();
	for (auto v: templateVertices) {
		vertices.add(
			{
				{v.position[0] * radius + offsetX,
					v.position[1] * radius + offsetY,
					v.position[2] * radius + offsetZ},
				{v.normal[0],   v.normal[1], v.normal[2]},
				{v.texCoord[0], v.texCoord[1]}
			}
		);
		sphereIds.add(sphereId);
	}
	for (auto i: templateIndices) {
		indices.add(i + startIndex);
	}
}


/* ScatterPlot component initializer
 *
 */
ScatterPlot::ScatterPlot() {
	// apparently openGL 3.2 is the highest supported by JUCE
	openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);


	_camera = new Camera();
    shiftPressed = false;
	setSize(800, 600);
	zoomValue = 5.0f;
	_draggingStartX = 0.0f;
	_draggingStartY = 0.0f;
	Random r;

	for (int i = 0; i < 200; i++) {
		spheres.push_back(SphereInfo{i,
																 (r.nextFloat() - 0.5f) * 20, // X
																 (r.nextFloat() - 0.5f) * 20, // Y
																 (r.nextFloat() - 0.5f) * 20, // Z

																 0.2f + r.nextFloat() * 0.4f, // radius

																 0.4f + r.nextFloat() * 0.8f, // red
																 0.4f + r.nextFloat() * 0.8f, // green
																 0.4f + r.nextFloat() * 0.8f, // blue

																 false, false});
	}

	SolidSphereGenerator sphereGenerator(16, 16);

	// Generate a template sphere in position 0.0f and size 1.0f
	sphereGenerator.generateSphere(0,
																 0.f,
																 0.f,
																 0.f,
																 1.0f,
																 1.0f,
																 1.0f,
																 1.0f,
																 1.0f,
																 vertices,
																 sphereId,
																 indices);
}


void ScatterPlot::callbackHover(int sphereId) {
	for (auto &sphere: spheres) {
		if (sphere.sphereId == sphereId) continue;
		sphere.hover = false;
		sphere.shake = false;
		sphere.animation.reset();
	}

	if (sphereId >= 0) {
		spheres[sphereId].hover = true;
		spheres[sphereId].shake = true;
		Logger::writeToLog("intersect: " + String(sphereId));
	}
}

Matrix3D<float> ScatterPlot::getProjectionMatrix() const {
	return glmToJuce(getProjectionMatrixGlm());
}

glm::mat4 ScatterPlot::getProjectionMatrixGlm() const {
	auto bounds = getLocalBounds();
	auto pers = glm::perspectiveFovRH<float>(45, bounds.getWidth(), bounds.getHeight(), 0.01f,
																					 100.0f);
	return pers;
}


Matrix3D<float> ScatterPlot::getViewMatrix() const {
	return glmToJuce(getViewMatrixGlm());
}

glm::mat4 ScatterPlot::getViewMatrixGlm() const {
	return _camera->getWorldToViewMatrix();
}


void ScatterPlot::render() {
	auxRender1();
	auxRender2();
	auxRender3();
}

ScatterPlot::~ScatterPlot() {
	shutdownOpenGL();
}

void ScatterPlot::paint(Graphics &g) {
}

void ScatterPlot::resized() {
}

/** @brief OpenGL initialization function called only once
 */
void ScatterPlot::initialise() {
    setWantsKeyboardFocus(true);
	auto local = getLocalBounds();

	selectShader = new SelectShader(openGLContext);

	std::cout << "w:" << local.getWidth() << " h:" << local.getHeight() << std::endl;


	selectShader->selectFrameBuffer = std::unique_ptr<FrameBuffer>(
		new FrameBuffer(openGLContext, local.getWidth(), local.getHeight(), true, false));



	//selectShader->selectFrameBuffer initialise(openGLContext, local.getWidth(), local.getHeight());


	std::cout << "Compiling main shader...";
	mainShader = new MainShader(openGLContext);
	std::cout << "Done" << std::endl;

	mainShader->output = std::unique_ptr<FrameBuffer>(
		new FrameBuffer(openGLContext, local.getWidth(), local.getHeight(), true, false));

	pp = new PostProcessing(openGLContext, local.getWidth(), local.getHeight());

	auto bright = pp->create({mainShader->output},
													 BinaryData::brightPassFrag_glsl);

	auto hBlur = pp->create({bright->output},
													BinaryData::blurPassFrag_glsl);
	hBlur->screenShader->addUniform<float>("resolution", local.getWidth());
	hBlur->screenShader->addUniform<float>("radius", 4.0f);
	hBlur->screenShader->addUniform<glm::vec3>("dir", glm::vec3(1.0f, 0.0f, 0.0f));

	auto vBlur = pp->create({hBlur->output},
													BinaryData::blurPassFrag_glsl);
	vBlur->screenShader->addUniform<float>("resolution", local.getHeight());
	vBlur->screenShader->addUniform<float>("radius", 4.0f);
	vBlur->screenShader->addUniform<glm::vec3>("dir", glm::vec3(0.0f, 1.0f, 0.0f));


	auto hBlur2 = pp->create({vBlur->output},
													 BinaryData::blurPassFrag_glsl);
	hBlur2->screenShader->addUniform<float>("resolution", local.getWidth());
	hBlur2->screenShader->addUniform<float>("radius", 8.0f);
	hBlur2->screenShader->addUniform<glm::vec3>("dir", glm::vec3(1.0f, 0.0f, 0.0f));

	auto vBlur2 = pp->create({hBlur2->output},
													 BinaryData::blurPassFrag_glsl);
	vBlur2->screenShader->addUniform<float>("resolution", local.getHeight());
	vBlur2->screenShader->addUniform<float>("radius", 8.0f);
	vBlur2->screenShader->addUniform<glm::vec3>("dir", glm::vec3(0.0f, 1.0f, 0.0f));


	auto final = pp->create({mainShader->output, vBlur2->output},
													BinaryData::finalPassFrag_glsl, true);


	//ppManager->create(mainShader->mainFrameBuffer, nullptr, true);



	// We only need to define this data once ----
	// TODO: Explain

	openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER,
																				vertices.size() * sizeof(Vertex),
																				vertices.getRawDataPointer(), GL_DYNAMIC_DRAW);


	openGLContext.extensions.glGenBuffers(1, &indexBuffer);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
																				indices.size() * sizeof(juce::uint32),
																				indices.getRawDataPointer(), GL_DYNAMIC_DRAW);
}

/** @brief Shutdown OpenGL
 */
void ScatterPlot::shutdown() {

}

void ScatterPlot::postProcessRender() {
	pp->render();

	/*screenShader->begin();

	screenShader->screenPass(mainShader->mainFrameBuffer->getTextureID());

	screenShader->end();*/
}

/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender1() {
	// Stuff to be done before defining your triangles
	jassert (OpenGLHelpers::isContextActive());
	const float desktopScale = (float) openGLContext.getRenderingScale();
	OpenGLHelpers::clear(Colour::greyLevel(0.05f));
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	glViewport(0, 0, roundToInt(desktopScale * getWidth()), roundToInt(desktopScale * getHeight()));

}


glm::vec3 rgbToFloat(uint32_t rgb) {
	float r = (float) ((rgb & (uint32_t) 0xFF)) / 255.0f;
	float g = (float) (((rgb >> 8) & (uint32_t) 0xFF)) / 255.0f;
	float b = (float) (((rgb >> 16) & (uint32_t) 0xFF)) / 255.0f;
	return glm::vec3(r, g, b);
}

void ScatterPlot::selectRender() {

	selectShader->begin();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	selectShader->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);
	selectShader->viewMatrix->setMatrix4(getViewMatrix().mat, 1, false);

	openGLContext.extensions.glVertexAttribPointer(selectShader->vertex->attributeID, 3, GL_FLOAT,
																								 GL_FALSE,
																								 sizeof(Vertex), 0);
	openGLContext.extensions.glEnableVertexAttribArray(selectShader->vertex->attributeID);

	for (auto &sphere : spheres) {
		glm::mat4 sphereMat = glm::translate(glm::mat4(), glm::vec3(sphere.x, sphere.y, sphere.z));

		auto color = rgbToFloat(selectShader->getColorByIndex(sphere.sphereId));

		glm::mat4 scaled = glm::scale(sphereMat, glm::vec3(sphere.radius));

		selectShader->modelMatrix->setMatrix4(glm::value_ptr(scaled), 1, false);
		selectShader->color->set(color.x, color.y, color.z, 1.0f);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	selectShader->end();
}

void ScatterPlot::mainRender() {
	mainShader->shader->use();

	mainShader->output->bind();
	glDisable(GL_DEPTH_TEST);

	OpenGLHelpers::clear(Colour::greyLevel(0.05f));
	//OpenGLHelpers::clear(Colour());


	mainShader->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

	mainShader->viewMatrix->setMatrix4(glm::value_ptr(_camera->getWorldToViewMatrix()), 1,
																		 false);

	if (mainShader->eyePosition != nullptr) {
		mainShader->eyePosition->set(
			_camera->position.x,
			_camera->position.y,
			_camera->position.z,
			1.0f);
		/*std::cout << "cam pos:" << _camera->position.x << "," << _camera->position.y << "," <<
							_camera->position.z << std::endl;*/
	}


	if (mainShader->lightPosition != nullptr)
		mainShader->lightPosition->set(-15.0f, 10.0f, 15.0f, 0.0f);

	if (mainShader->lightColor != nullptr)
		mainShader->lightColor->set(1.0f, 1.0f, 1.0f, 1.0f);


	Vertex *k = 0;

	// Now prepare this information to be drawn
	if (mainShader->vertex != nullptr) {
		openGLContext.extensions.glVertexAttribPointer(mainShader->vertex->attributeID, 3, GL_FLOAT,
																									 GL_FALSE, sizeof(Vertex), &k->position);
		openGLContext.extensions.glEnableVertexAttribArray(mainShader->vertex->attributeID);
	}

	if (mainShader->normal != nullptr) {
		openGLContext.extensions.glVertexAttribPointer(mainShader->normal->attributeID, 3, GL_FLOAT,
																									 GL_FALSE, sizeof(Vertex), &k->normal);
		openGLContext.extensions.glEnableVertexAttribArray(mainShader->normal->attributeID);
	}


	if (mainShader->textureCoord != nullptr) {
		openGLContext.extensions.glVertexAttribPointer(mainShader->textureCoord->attributeID, 2,
																									 GL_FLOAT,
																									 GL_FALSE, sizeof(Vertex), &k->texCoord);

		openGLContext.extensions.glEnableVertexAttribArray(mainShader->textureCoord->attributeID);
	}


	for (auto &sphere: spheres) {

		glm::vec3 spherePos(sphere.x, sphere.y, sphere.z);
		glm::mat4 sphereMat = glm::translate(glm::mat4(), spherePos);


		if (sphere.shake) {
			float shakeSpeed = 0.30f; // Lower number is faster, higher number is slower (time)

			glm::vec3 randomPos(sphere.x + (r.nextFloat() - 0.5f) * (sphere.radius / 2.0f),
													sphere.y + (r.nextFloat() - 0.5f) * (sphere.radius / 2.0f),
													sphere.z + (r.nextFloat() - 0.5f) * (sphere.radius / 2.0f));

			if (!sphere.animation.enabled) {
				sphere.animation.animate(spherePos, randomPos, shakeSpeed);
			} else if (sphere.animation.finished) {
				sphere.animation.reanimate(randomPos, shakeSpeed);
			}
		}

		if (sphere.animation.enabled) {
			sphereMat = glm::translate(glm::mat4(), sphere.animation.animate());
		}

		glm::mat4 scaled = glm::scale(sphereMat, glm::vec3(sphere.radius));

		if (mainShader->normalMatrix != nullptr) {
			auto viewMatrix = _camera->getWorldToViewMatrix();
			auto modelMatrix = scaled;
			auto modelViewMatrix = viewMatrix * modelMatrix;
			auto normalMatrix = glm::inverse(modelViewMatrix);
			mainShader->normalMatrix->setMatrix4(glm::value_ptr(normalMatrix), 1, true);

		}


		mainShader->modelMatrix->setMatrix4(glm::value_ptr(scaled), 1, false);
		if (mainShader->color != nullptr) {
			if (sphere.hover) {
				mainShader->color->set(1.0f, 1.0f, 1.0f, 1.0f);
			} else {
				//mainShader->color->set(1.0f, 0.0f, 0.0f, 1.0f);
				mainShader->color->set(sphere.r, sphere.g, sphere.b, 1.0f);
			}
		}




		/*if (mainShader->color != nullptr)
			mainShader->color->set(1.0f, 1.0f, 1.0f, 1.0f);*/

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}

	mainShader->output->unbind();
}

/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender2() {
	// ************************************************

	// TODO: Explain
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);


	mainRender();
	selectRender();
	postProcessRender();
}


/** @brief Needed code for render function
 *
 */
void ScatterPlot::auxRender3() {

	// Reset the element buffers so child Components draw correctly
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//openGLContext.swapBuffers();

	//openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
	//openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
	repaint();
}


void ScatterPlot::mouseDown(const MouseEvent &e) {
    _draggingStartX = e.getPosition().x;
    _draggingStartY = e.getPosition().y;
    _camera->startDragging();
}

glm::vec2 ScatterPlot::computeDraggingDeltaPosition(const MouseEvent &e) {
    float sX = (2.0f * _draggingStartX) / (float)getWidth() - 1.0f;
    float sY = 1.0f - (2.0f * _draggingStartY) / (float)getHeight();
    
    float xPoint = (2.0f * e.getPosition().x) / (float)getWidth() - 1.0f;
    float yPoint = 1.0f - (2.0f * e.getPosition().y) / (float)getHeight();
    
    float dx, dy;
    dx = (float)(sX - xPoint) * 10.f;
    dy = (float)(sY - yPoint) * 20.f;
    return glm::vec2(dx, dy);
}

void ScatterPlot::mouseDrag(const MouseEvent &e) {

    glm::vec2 deltaPosition = computeDraggingDeltaPosition(e);
    float dx = deltaPosition.x;
    float dy = deltaPosition.y;
	if (e.mods.isLeftButtonDown() && !shiftPressed) {
		_camera->rotate(dx, dy);
	}
	if (e.mods.isLeftButtonDown() && shiftPressed) {
		_camera->pan(dx, dy);
	}
	if (e.mods.isRightButtonDown()) {
		_camera->zoomByDragging(dy);
	}
}

void ScatterPlot::mouseWheelMove(const MouseEvent &e, const MouseWheelDetails &d) {
    //std::cout << d.deltaY << std::endl;
    //_camera->zoom(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
	// _camera->zoom(glm::vec2(e.getPosition().x, e.getPosition().y), getHeight(), getWidth());
	//zoomValue += d.deltaY;
	//zoomValue = jmin(jmax(zoomValue, 0.1f), 30.0f);
    _camera->zoomByWheel(d.deltaY);
}

void ScatterPlot::mouseMove(const MouseEvent &e) {
	//float x = ((float) e.getPosition().getX() / getWidth() - 0.5f) * 2;
	//float y = ((float) e.getPosition().getY() / getHeight() - 0.5f) * -2;

	auto local = getLocalBounds();
	auto x = e.getPosition().getX();
	auto y = local.getHeight() - e.getPosition().getY();


	if (selectShader == nullptr) {
		return;
	}

	int pixelIndex = (y * selectShader->selectFrameBuffer->getWidth()) + x;

	if (pixelIndex > selectShader->pixels.size()) {
		return;
	}

	// TODO: check out of bounds
	PixelARGB &readPixel = selectShader->pixels[pixelIndex];

	uint32_t color = readPixel.getRed() | readPixel.getGreen() << 8 | readPixel.getBlue() << 16;
	//std::cout << std::hex << color << std::endl;
	int index = selectShader->getIndexByColor(color);

	callbackHover(index);
}

void ScatterPlot::modifierKeysChanged (const ModifierKeys &modifiers) {
    shiftPressed = modifiers.isShiftDown();
}
