#include "glh.hpp"

void keyboardCallback(GLFWwindow* window, int key, int scancode,
		int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int main(int argc, char **argv)
{
	glh my_gl(500,100, "Super swag");//keyboardCallback);

	while(!glfwWindowShouldClose(my_gl.window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnableVertexAttribArray(0);

		glDisableVertexAttribArray(0);
		glfwSwapBuffers(my_gl.window);
	}

	my_gl.cleanup();



	return 0;
}
