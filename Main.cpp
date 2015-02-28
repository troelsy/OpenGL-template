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
#include <png.h>

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

struct texturePolygon {
    GLuint texture;
    GLuint shader;
    GLint textureID;
    GLuint matrixID;
    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLfloat vertexBufferArray[1024];
    GLfloat uvBufferArray[1024];
    GLuint arrayLength;
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

static void uploadArray(struct texturePolygon *obj){
    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * obj->arrayLength, obj->vertexBufferArray, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, obj->uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * obj->arrayLength, obj->uvBufferArray, GL_STATIC_DRAW);
}

static void drawArray(struct arrayObject *obj){
    glUseProgram(obj->shader);

    glUniform3fv(obj->uniform, 1, glm::value_ptr(obj->colorVec));
    glUniformMatrix4fv(obj->programObject, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(obj->mode, 0, obj->vertexArrayLength);
}

static void drawArrayTexture(struct texturePolygon *obj){
    glUseProgram(obj->shader);
    glUniformMatrix4fv(obj->matrixID, 1, GL_FALSE, &MVP[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    glUniform1i(obj->textureID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, obj->uvBuffer);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

    glDrawArrays(GL_TRIANGLES, 0, 12*3);
}

static void getUniform(struct arrayObject *obj){
    obj->programObject = glGetUniformLocation(obj->shader, "uModelMatrix");
    obj->uniform = glGetUniformLocation(obj->shader, "uColorVec");
}

static void getUniform(struct texturePolygon *obj){
    obj->textureID = glGetUniformLocation(obj->shader, "myTextureSampler");
    obj->matrixID = glGetUniformLocation(obj->shader, "MVP");
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


void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

//int x, y;



#define PNG_SIG_BYTES 8

GLuint read_png_file(const char * file_name, int * width, int * height){
    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        //return 0;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        //return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        //return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        //return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        //return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        //return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data;
    image_data = (png_byte *) malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        //return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_bytep * row_pointers = (png_bytep *)malloc(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        //return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    int i;
    for (i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp_width, temp_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    return texture;
}


int main(int argc, char *argv[]){
    init(); // Set OpenGL settings

    // Load shaders
    string vs = "";
    string fs = "";
    assert(fileRead("colorVertex.vert", &vs) >= 0);
    assert(fileRead("colorFragment.frag", &fs) >= 0);

    // Create dots
    arrayObject dot = {GL_POINTS,
                       compileShader(vs, fs),
                       glm::vec3(1.0f, 1.0f, 0.0f),
                       (GLuint)NULL,
                       {0.0f,0.0f,0.0f,10.0f,-10.0f,0.0f,20.0f,-20.0f,0.0f,
                        30.0f,-30.0f,0.0f,40.0f,-40.0f,0.0f,50.0f,-50.0f,0.0f,
                        60.0f,-60.0f,0.0f},
                       21};

    // Create triangle
    arrayObject tri = {GL_TRIANGLES,
                       compileShader(vs, fs),
                       glm::vec3(0.0f, 1.0f, 1.0f),
                       (GLuint)NULL,
                       {50.0f,0.0f,0.0f,150.0f,100.0f,0.0f,0.0f,150.0f,0.0f},
                       9};

    // Create line
    arrayObject line = {GL_LINES,
                        compileShader(vs, fs),
                        glm::vec3(1.0f, 0.0f, 1.0f),
                        (GLuint)NULL,
                        {150.0f,50.0f,0.0f,250.0f,350.0f,0.0f},
                        4};

    string vs2 = "";
    string fs2 = "";
    assert(fileRead("textureVertex.vert", &vs2) >= 0);
    assert(fileRead("textureFragment.frag", &fs2) >= 0);

    int *w = 0;
    int *h = 0;
    texturePolygon tex = {
        read_png_file("test.png", w, h),
        compileShader(vs2, fs2),
        (GLint)NULL,
        (GLint)NULL,
        (GLint)NULL,
        (GLint)NULL,
        {100.0f, 0.0f, 0.0f,
         0.0f, 100.0f, 0.0f,
         100.0f, 100.0f, 0.0f,
         0.0f, 0.0f, 0.0f,
         0.0f, 100.0f, 0.0f,
         100.0f, 0.0f, 0.0f
        },
        {1.0f, 0.0f,
         0.0f, 1.0f,
         1.0f, 1.0f,
         0.0f, 0.0f,
         0.0f, 1.0f,
         1.0f, 0.0f
        },
        6};


    glGenBuffers(1, &tex.vertexBuffer);
    glGenBuffers(1, &tex.uvBuffer);

    glGenBuffers(1, &dot.vertexBuffer);
    glGenBuffers(1, &tri.vertexBuffer);
    glGenBuffers(1, &line.vertexBuffer);

    // Upload data array buffer to GPU
    uploadArray(&tex);

    uploadArray(&dot);
    uploadArray(&tri);
    uploadArray(&line);

    // Get the uniform location of uploaded programs. You -must- refresh all
    // uniforms, if you upload a new array.
    getUniform(&tex);

    getUniform(&dot);
    getUniform(&tri);
    getUniform(&line);

    // main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        drawArrayTexture(&tex);
/*
        glUseProgram(tex.shader);
        glUniformMatrix4fv(tex.matrixID, 1, GL_FALSE, &MVP[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.texture);
        glUniform1i(tex.textureID, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 12*3);
*/


        drawArray(&dot);
        drawArray(&tri);
        drawArray(&line);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glfwSwapBuffers(window);
    }

    // Clean up
    glDeleteVertexArrays(1, &VertexArrayID);

    glDeleteBuffers(1, &dot.vertexBuffer);
    glDeleteBuffers(1, &tri.vertexBuffer);
    glDeleteBuffers(1, &line.vertexBuffer);

    glDeleteProgram(dot.shader);
    glDeleteProgram(tri.shader);
    glDeleteProgram(line.shader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
