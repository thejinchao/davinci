#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>

#include <GLFW/glfw3.h>

#include "dc_sample01.h"
#include "dc_sample02.h"
#include "dc_sample03.h"

//#define RENDER_ONCE

//-------------------------------------------------------------------------------------
int32_t g_screenWidth = 512;
int32_t g_screenHeight = 256;
uint32_t g_canvasTexture;
Sample02 g_dvScene;

//-------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		g_dvScene.dumpToPNGFile("screen.png", "depth.png");
	}
}

//-------------------------------------------------------------------------------------
double calcFPS(GLFWwindow* window, const char* theWindowTitle, double theTimeInterval = 1.0)
{
	// Static values which only get initialised the first time the function runs
	static double t0Value = glfwGetTime(); // Set the initial time to now
	static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
	static double fps = 0.0;           // Set the initial FPS value to 0.0

									   // Get the current time in seconds since the program started (non-static, so executed every time)
	double currentTime = glfwGetTime();

	// Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
	// Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
	if (theTimeInterval < 0.1)
	{
		theTimeInterval = 0.1;
	}
	if (theTimeInterval > 10.0)
	{
		theTimeInterval = 10.0;
	}

	// Calculate and display the FPS every specified time interval
	if ((currentTime - t0Value) > theTimeInterval)
	{
		// Calculate the FPS as the number of frames divided by the interval in seconds
		fps = (double)fpsFrameCount / (currentTime - t0Value);

		char pszTitleString[256] = { 0 };
		snprintf(pszTitleString, 256, "%s  | FPS: %.1f", theWindowTitle, fps);
		glfwSetWindowTitle(window, pszTitleString);

		// Reset the FPS frame counter and set the initial time to be now
		fpsFrameCount = 0;
		t0Value = glfwGetTime();
	}
	else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
	{
		fpsFrameCount++;
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}

//-------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	// glfw: initialize and configure
	if (GLFW_TRUE != glfwInit()) {
		return -1;
	}

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(g_screenWidth, g_screenHeight, "Davinci", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// Enable vsync
	glfwSwapInterval(1);

	// Set callback functions
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int width, int height) {
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		g_screenWidth = width;
		g_screenHeight = height;
		glViewport(0, 0, width, height);
	});

	// load and create a texture 
	glGenTextures(1, &g_canvasTexture);
	glBindTexture(GL_TEXTURE_2D, g_canvasTexture);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//gl param
	glEnable(GL_TEXTURE_2D);

	//render scene
	if (!g_dvScene.init()) {
		return -1;
	}

	//update scene(once)
#ifdef RENDER_ONCE
	g_dvScene.render(g_screenWidth, g_screenHeight);
	g_dvScene.dumpToGLTexture(g_canvasTexture);
	g_dvScene.dumpToPNGFile("screen.png", "depth.png");
#endif

	// render loop
	while (!glfwWindowShouldClose(window)) {
		//process input
		processInput(window);

#ifndef RENDER_ONCE
		g_dvScene.render(g_screenWidth, g_screenHeight);
		g_dvScene.dumpToGLTexture(g_canvasTexture);
#endif
		// render
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex3f(-1, -1, 0);
			glTexCoord2f(1.0, 0.0); glVertex3f(+1, -1, 0);
			glTexCoord2f(1.0, 1.0); glVertex3f(+1, +1, 0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-1, +1, 0);
		glEnd();

		// swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();

		//fps
		calcFPS(window, "Davinci");
	}

	//terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;

}
