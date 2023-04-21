#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 fragmentColor;

void main(){

    gl_Position.xyz = aPos;
    gl_Position.w = 1.0;

    // The color of each vertex will be interpolated
    // to produce the color of each fragment
    fragmentColor = aColor;
}