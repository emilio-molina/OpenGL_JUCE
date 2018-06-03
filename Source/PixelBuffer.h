/*
  ==============================================================================

    PixelBuffer.h
    Created: 31 May 2018 2:14:35am

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PixelBuffer {
public:
	PixelBuffer(OpenGLContext &c, const int w, const int h) : context(c) {
		width = w;
		height = h;

		dataSize = (size_t) w * (size_t) h * 4;
		c.extensions.glGenBuffers(1, &buffer);
		c.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
		c.extensions.glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);
		c.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		if (glMapBuffer == nullptr) {
			glMapBuffer = (PFNGLMAPBUFFERPROC) OpenGLHelpers::getExtensionFunction
				("glMapBuffer");
			glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) OpenGLHelpers::getExtensionFunction
				("glUnmapBuffer");

			if (glMapBuffer == nullptr) {
				glMapBuffer = (PFNGLMAPBUFFERPROC) OpenGLHelpers::getExtensionFunction
					("glMapBufferARB");
				glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) OpenGLHelpers::getExtensionFunction
					("glUnmapBufferARB");
			}
		}
	}

	~PixelBuffer() {
		context.extensions.glDeleteBuffers(1, &buffer);
	}

	void read() {
		context.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
		glReadPixels(0, 0, width, height, JUCE_RGBA_FORMAT, GL_UNSIGNED_BYTE, 0);
		context.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	GLubyte *map() {
		context.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
		return (GLubyte *) (*glMapBuffer)(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
	}

	void unmap() {
		(*glUnmapBuffer)(GL_PIXEL_PACK_BUFFER);
		context.extensions.glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	size_t getDataSize() const {
		return dataSize;
	}

private:
	static PFNGLMAPBUFFERPROC glMapBuffer;
	static PFNGLUNMAPBUFFERPROC glUnmapBuffer;


	OpenGLContext &context;
	int width;
	int height;
	GLuint buffer;
	size_t dataSize;
};

class AlternatingPixelBuffer {
public:

	AlternatingPixelBuffer(OpenGLContext &c, const int w, const int h) {
		pbo1 = std::shared_ptr<PixelBuffer>(new PixelBuffer(c, w, h));
		pbo2 = std::shared_ptr<PixelBuffer>(new PixelBuffer(c, w, h));
		reading = pbo1.get();
		use = pbo2.get();
	}

	void read() {
		auto tmp = reading;
		reading = use;
		reading->read();
		use = tmp;
	}

	GLubyte *map() {
		return use->map();
	}

	void unmap() {
		use->unmap();
	}

	size_t getDataSize() const {
		return pbo1->getDataSize();
	}

private:
	std::shared_ptr<PixelBuffer> pbo1;
	std::shared_ptr<PixelBuffer> pbo2;
	PixelBuffer *reading;
	PixelBuffer *use;
};