#include "glh.hpp"

void keyboardCallback(GLFWwindow* window, int key, int scancode,
		int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char **argv)
{
	glh my_gl(1000,800,"OpenGL Simulator",keyboardCallback);

	// Create dots
	arrayObject dot = {GL_POINTS,
		my_gl.shader,
		glm::vec3(1.0f, 1.0f, 0.0f),
		(GLuint)NULL,
		{0.0f,0.0f,0.0f,10.0f,-10.0f,0.0f,20.0f,-20.0f,0.0f,
			30.0f,-30.0f,0.0f,40.0f,-40.0f,0.0f,50.0f,-50.0f,0.0f,
			60.0f,-60.0f,0.0f},
		21};

	// Create triangle
	arrayObject tri = {GL_TRIANGLES,
		my_gl.shader,
		glm::vec3(0.0f, 1.0f, 1.0f),
		(GLuint)NULL,
		{50.0f,0.0f,0.0f,150.0f,100.0f,0.0f,0.0f,150.0f,0.0f},
		9};

	// Create line
	arrayObject line = {GL_LINES,
		my_gl.shader,
		glm::vec3(1.0f, 0.0f, 1.0f),
		(GLuint)NULL,
		{150.0f,50.0f,0.0f,250.0f,350.0f,0.0f},
		4};

	glGenBuffers(1, &dot.vertexBuffer);
	glGenBuffers(1, &tri.vertexBuffer);
	glGenBuffers(1, &line.vertexBuffer);

	my_gl.uploadArray(&dot);
	my_gl.uploadArray(&tri);
	my_gl.uploadArray(&line);

	my_gl.getUniform(&dot);
	my_gl.getUniform(&tri);
	my_gl.getUniform(&line);


	while(!glfwWindowShouldClose(my_gl.window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnableVertexAttribArray(0);

		my_gl.drawArray(&dot);
		my_gl.drawArray(&tri);
		my_gl.drawArray(&line);

		glDisableVertexAttribArray(0);
		glfwSwapBuffers(my_gl.window);
	}

	glDeleteBuffers(1, &dot.vertexBuffer);
	glDeleteBuffers(1, &tri.vertexBuffer);
	glDeleteBuffers(1, &line.vertexBuffer);

	glDeleteProgram(dot.shader);
	glDeleteProgram(tri.shader);
	glDeleteProgram(line.shader);


	my_gl.cleanup();

	return 0;
}
