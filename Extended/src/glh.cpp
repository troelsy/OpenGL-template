#include "glh.hpp"

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
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>

#define PNG_DEBUG 3
#define PNG_SIG_BYTES 8
#include <png.h>

using namespace std;
using namespace glm;

// TODO: PLace somewhere else
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

void glh::printShaderError(GLuint shaderObj, GLenum shaderType, string const& msg){
    GLchar errorLog[1024] = { 0 };
    glGetShaderInfoLog(shaderObj, 1024, NULL, errorLog);
    cout << msg << " " << shaderType << ": " << errorLog << endl;
    exit(EXIT_FAILURE);
}

void glh::printProgramError(GLuint program, string const& msg){
    GLchar errorLog[1024] = { 0 };
    glGetProgramInfoLog(program, 1024, NULL, errorLog);
    cout << msg << ": " << errorLog << endl;
    exit(EXIT_FAILURE);
}



void glh::uploadArray(struct arrayObject *obj){
    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * obj->vertexArrayLength,
                 obj->vertexArray, GL_STATIC_DRAW);
}

void glh::drawArray(struct arrayObject *obj){
    glUseProgram(obj->shader);

    glUniform3fv(obj->uniform, 1, glm::value_ptr(obj->colorVec));
    glUniformMatrix4fv(obj->programObject, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(obj->mode, 0, obj->vertexArrayLength);
}

void glh::getUniform(struct arrayObject *obj){
    obj->programObject = glGetUniformLocation(obj->shader, "uModelMatrix");
    obj->uniform = glGetUniformLocation(obj->shader, "uColorVec");
}

void glh::init(){
    glewExperimental = GL_TRUE;

    assert(glfwInit() == true  && "Could not initialize GLFW");

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(width, height,windowName.c_str(), NULL, NULL);
    assert(window != NULL && "Could not initialize window");
    glfwMakeContextCurrent(window);
    assert(glewInit() == GLEW_OK && "Could not initialize GLEW");

    // TODO: Implement keyboard callback
    glfwSetKeyCallback(window, keycallback); // Set GLFW call back function
    glfwSetMouseButtonCallback(window, mousecallback);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glClearDepth(-1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    // Create a Vertex Array Object
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Load and Bind shader
    // Load shaders
    string vs = "";
    string fs = "";
    assert(fileRead("Shaders/Shader.vert", &vs) >= 0);
    assert(fileRead("Shaders/Shader.frag", &fs) >= 0);
    shader = compileShader(vs, fs);
}


void glh::addShader(string const& shaderString, GLenum shaderType, GLuint m_program){
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

GLuint glh::compileShader(string const& vs, string const& fs){
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

GLuint glh::read_png_file(const char * file_name, int * width, int * height){
    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    assert(fp && "Could not open PNG file");

    fread(header, 1, 8, fp);
    assert(!png_sig_cmp(header, 0, PNG_SIG_BYTES) && "File is not a PNG");

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                                 NULL, NULL);
    assert(png_ptr && "Could not create read struct for PNG");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr && "Could not create info struct");

    png_infop end_info = png_create_info_struct(png_ptr);
    assert(end_info && "Could not create end infomation");

    assert(!setjmp(png_jmpbuf(png_ptr)) && "libpng encountered an error");

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, PNG_SIG_BYTES);
    png_read_info(png_ptr, info_ptr);

    int bit_depth;
    int color_type;
    png_uint_32 temp_width;
    png_uint_32 temp_height;
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth,
                 &color_type, NULL, NULL, NULL);

    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }


    png_read_update_info(png_ptr, info_ptr);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data;
    image_data = (png_byte *) malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    assert(image_data != NULL && "Could not allocate memory for PNG image data");

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep * row_pointers = (png_bytep *)malloc(temp_height * sizeof(png_bytep));
    assert(row_pointers != NULL && "Could not allocate memory for PNG row pointers");

    // set the individual row_pointers to point at the correct offsets of image_data
    for (int i = 0; i < temp_height; i++){
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp_width, temp_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);

    return texture;
}


// Default parameters given in header!
glh::glh(int w, int h, string name,GLFWkeyfun kcb,GLFWmousebuttonfun mcb)
{
	width = w;
	height = h;
	windowName = name;

    // Set callbacks
    mousecallback = mcb;
    keycallback = kcb;

	init();
}

void glh::cleanup()
{
    // Clean up
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwDestroyWindow(window);
    glfwTerminate();
}
