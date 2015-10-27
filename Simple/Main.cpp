// https://github.com/troelsy/OpenGL-template

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
#include <assert.h>
using namespace std;

GLint width = 1024;
GLint height = 576;
GLFWwindow* window;
GLuint VertexArrayID;

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

float matrix[16] = {
    1.0f / ((float)width/2.0f), 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f / ((float)height/2.0f), 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

// Static Model-View-Projection matrix
glm::mat4 MVP = glm::make_mat4(matrix);

static void keyboardCallback(GLFWwindow* window, int key, int scancode,
                             int action, int mods){
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void uploadArray(struct arrayObject *obj){
    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * obj->vertexArrayLength,
                 obj->vertexArray, GL_STATIC_DRAW);
}

static void drawArray(struct arrayObject *obj){
    glUseProgram(obj->shader);

    glUniform3fv(obj->uniform, 1, glm::value_ptr(obj->colorVec));
    glUniformMatrix4fv(obj->programObject, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(obj->mode, 0, obj->vertexArrayLength);
}

static void getUniform(struct arrayObject *obj){
    obj->programObject = glGetUniformLocation(obj->shader, "uModelMatrix");
    obj->uniform = glGetUniformLocation(obj->shader, "uColorVec");
}

static void init(){
    glewExperimental = GL_TRUE;

    assert(glfwInit() == true  && "Could not initialize GLFW");

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(width, height,"title", NULL, NULL);
    assert(window != NULL && "Could not initialize window");
    glfwMakeContextCurrent(window);
    assert(glewInit() == GLEW_OK && "Could not initialize GLEW");

    glfwSetKeyCallback(window, keyboardCallback); // Set GLFW call back function

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glClearDepth(-1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // Create a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
}

static int fileRead(string const& filename, string* result){
    ifstream ifs(filename.data());
    if(ifs.is_open()) {
        *result = string((istreambuf_iterator<char>(ifs)),
                              (istreambuf_iterator<char>()));
        ifs.close();
        return 0;
    }
    return -1;
}

static void printShaderError(GLuint shaderObj, GLenum shaderType, string const& msg){
    GLchar errorLog[1024] = { 0 };
    glGetShaderInfoLog(shaderObj, 1024, NULL, errorLog);
    cout << msg << " " << shaderType << ": " << errorLog << endl;
    exit(EXIT_FAILURE);
}

static void printProgramError(GLuint program, string const& msg){
    GLchar errorLog[1024] = { 0 };
    glGetProgramInfoLog(program, 1024, NULL, errorLog);
    cout << msg << ": " << errorLog << endl;
    exit(EXIT_FAILURE);
}

static void addShader(string const& shaderString, GLenum shaderType, GLuint m_program){
    GLuint shaderObj = glCreateShader(shaderType);

    if (shaderObj == 0){
        cout << "Fatal: Error creating shader type ";
        if (shaderType == GL_VERTEX_SHADER) cout << "GL_VERTEX_SHADER" << endl;
        else if (shaderType == GL_FRAGMENT_SHADER) cout << "GL_FRAGMENT_SHADER" << endl;
        exit(EXIT_FAILURE);
    }

    GLchar const* str = shaderString.data();
    GLint length = shaderString.size();
    glShaderSource(shaderObj, 1, &str, &length);
    glCompileShader(shaderObj);

    GLint success;
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
    if (success == 0)
        printShaderError(shaderObj, shaderType, "Fatal: Error compiling shader type");

    glAttachShader(m_program, shaderObj);
}

GLuint compileShader(string const& vs, string const& fs){
    GLuint program = glCreateProgram();
    assert(program != 0 && "Fatal: Error creating shader program.");

    addShader(vs, GL_VERTEX_SHADER, program);
    addShader(fs, GL_FRAGMENT_SHADER, program);

    GLint success;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(success == 0) {
        printProgramError(program, "Fatal: Error linking shader program");
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if(success == 0) {
        printProgramError(program, "Fatal: Invalid shader program");
    }

    return program;
}

int main(int argc, char *argv[]){
    init(); // Set OpenGL settings

    // Load shaders
    string vs = "";
    string fs = "";
    assert(fileRead("Shader.vert", &vs) >= 0);
    assert(fileRead("Shader.frag", &fs) >= 0);

    // Create dots
    arrayObject dots = {
        GL_POINTS,
        compileShader(vs, fs),
        glm::vec3(1.0f, 1.0f, 0.0f),
        (GLuint)NULL,
        { 0.0f,   0.0f, 0.0f,
         10.0f, -10.0f, 0.0f,
         20.0f, -20.0f, 0.0f,
         30.0f, -30.0f, 0.0f,
         40.0f, -40.0f, 0.0f,
         50.0f, -50.0f, 0.0f,
         60.0f, -60.0f, 0.0f},
        7*3};

    glGenBuffers(1, &dots.vertexBuffer);

    // Upload data array buffer to GPU
    uploadArray(&dots);

    // Get the uniform location of uploaded programs. You -must- refresh all
    // uniforms, if you upload a new array.
    getUniform(&dots);

    // main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(0);

        drawArray(&dots);

        glDisableVertexAttribArray(0);
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VertexArrayID);

    glDeleteBuffers(1, &dots.vertexBuffer);

    glDeleteProgram(dots.shader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
