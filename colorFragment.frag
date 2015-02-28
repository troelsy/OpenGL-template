#version 330 core

layout(location = 0) out vec4 colourOut;
uniform vec3 uColorVec;
void main() {
    colourOut = vec4(uColorVec, 1.0);
}