#ifndef _GLH_HPP
#define _GLH_HPP

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #include <GL/glew.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace glm;

struct arrayObject {
    GLuint mode;
    GLuint shader;
    glm::vec3 colorVec;
    GLuint vertexBuffer;
    GLfloat vertexArray[1024]; // Should be deleted after upload
    GLuint vertexArrayLength;
    GLuint programObject;
    GLuint uniform;
};


// GLHelper
class glh {
	public:
		glh(GLint, GLint, string);
		void keyboardCallback(GLFWwindow* window, int key, int scancode,
				int action, int mods);

		void uploadArray(struct arrayObject*);
		void drawArray(struct arrayObject*);	
		void getUniform(struct arrayObject *obj);
		void printShaderError(GLuint shaderObj, GLenum shaderType, string const& msg);
		void printProgramError(GLuint program, string const& msg);

		void addShader(string const& shaderString, GLenum shaderType, GLuint m_program);
		GLuint compileShader(string const& vs, string const& fs);

		int run();


	private:
		void init();
		void cleanup();

	GLint width;
	GLint height;
	string windowName;
	GLFWwindow *window;
	GLuint VertexArrayID;

	// Static Model-View-Projection matrix
	float matrix[16] = {
    1.0f / ((float)width/2.0f), 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f / ((float)height/2.0f), 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f};
	glm::mat4 MVP = glm::make_mat4(matrix);


};


#endif