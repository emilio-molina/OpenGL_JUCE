/*
  ==============================================================================

    FrameBuffer.h
    Created: 12 May 2018 5:25:13pm
		Description: JUCE's framebuffer object wasnt supporting depth so derived this
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


class FrameBuffer {
public:
	FrameBuffer(OpenGLContext &c, const int w, const int h,
							const bool wantsDepthBuffer, const bool wantsStencilBuffer)
		: context(c), width(w), height(h),
			textureID(0), frameBufferID(0), depthOrStencilBuffer(0),
			hasDepthBuffer(false), hasStencilBuffer(false) {
		// Framebuffer objects can only be created when the current thread has an active OpenGL
		// context. You'll need to create this object in one of the OpenGLContext's callbacks.
		jassert (OpenGLHelpers::isContextActive());

#if JUCE_WINDOWS || JUCE_LINUX
		if (context.extensions.glGenFramebuffers == nullptr)
			return;
#endif

		context.extensions.glGenFramebuffers(1, &frameBufferID);
		bind();

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		context.extensions.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
																							textureID, 0);

		if (wantsDepthBuffer || wantsStencilBuffer) {
			context.extensions.glGenRenderbuffers(1, &depthOrStencilBuffer);
			context.extensions.glBindRenderbuffer(GL_RENDERBUFFER, depthOrStencilBuffer);
			jassert (context.extensions.glIsRenderbuffer(depthOrStencilBuffer));

			context.extensions.glRenderbufferStorage(GL_RENDERBUFFER,
																							 (wantsDepthBuffer && wantsStencilBuffer)
																							 ? GL_DEPTH24_STENCIL8
																							 #if JUCE_OPENGL_ES
																							 : GL_DEPTH_COMPONENT16,
																							 #else
																							 : GL_DEPTH_COMPONENT,
#endif
																							 width, height);

			GLint params = 0;
			context.extensions.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_DEPTH_SIZE,
																											&params);
			context.extensions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
																									 GL_RENDERBUFFER, depthOrStencilBuffer);

			if (wantsStencilBuffer)
				context.extensions.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
																										 GL_RENDERBUFFER, depthOrStencilBuffer);

			hasDepthBuffer = wantsDepthBuffer;
			hasStencilBuffer = wantsStencilBuffer;
		}

		unbind();
	}

	~FrameBuffer() {
		if (OpenGLHelpers::isContextActive()) {
			if (textureID != 0)
				glDeleteTextures(1, &textureID);

			if (depthOrStencilBuffer != 0)
				context.extensions.glDeleteRenderbuffers(1, &depthOrStencilBuffer);

			if (frameBufferID != 0)
				context.extensions.glDeleteFramebuffers(1, &frameBufferID);

		}
	}

	bool createdOk() const {
		return frameBufferID != 0 && textureID != 0;
	}

	void bind() {
		context.extensions.glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	}

	void unbind() {
		context.extensions.glBindFramebuffer(GL_FRAMEBUFFER, context.getFrameBufferID());
	}

	OpenGLContext &context;
	const int width, height;
	GLuint textureID, frameBufferID, depthOrStencilBuffer;
	bool hasDepthBuffer, hasStencilBuffer;

	int getWidth() const noexcept { return this->width; }

	int getHeight() const noexcept { return this->height; }

	GLuint getTextureID() const noexcept { return this->textureID; }


	GLuint getFrameBufferID() const noexcept {
		return frameBufferID;
	}

	GLuint getCurrentFrameBufferTarget() noexcept {
		GLint fb;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
		return (GLuint) fb;
	}


	void clear(Colour colour) {
		bind();
		OpenGLHelpers::clear(colour);
		unbind();
	}

	void makeCurrentAndClear() {
		bind();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void readPixels(PixelARGB *target, const Rectangle<int> &area) {
		bind();

		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glReadPixels(area.getX(), area.getY(), area.getWidth(), area.getHeight(),
								 JUCE_RGBA_FORMAT, GL_UNSIGNED_BYTE, target);
	}


private:
	bool checkStatus() noexcept {
		const GLenum status = context.extensions.glCheckFramebufferStatus(GL_FRAMEBUFFER);

		return status == GL_NO_ERROR
					 || status == GL_FRAMEBUFFER_COMPLETE;
	}
};
