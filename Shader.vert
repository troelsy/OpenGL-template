#version 330 core

layout(location = 0) in vec3 vertPosition;
uniform mat4 uModelMatrix;

void main() {
    gl_Position = uModelMatrix * vec4(vertPosition.xyz, 1.0);
}
