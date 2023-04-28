#version 330 core

out vec3 fragColor;

uniform float tScale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 colormask;

void main(){
    float Mt = gl_VertexID * tScale;
    vec3 aPos = Mf(Mt);
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    float nMt = (gl_VertexID + 1) * tScale;
    vec3 tangent = Mf(nMt) - aPos;

    fragColor = abs(tangent * colormask);
}
