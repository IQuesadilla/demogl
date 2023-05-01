#version 330 core

out vec3 fragColor;

uniform float tScale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 colormask;
uniform int startID;

void main(){
    int cID = gl_VertexID + int( startID / tScale );
    float Mt = cID * tScale;
    vec3 aPos = Mf(Mt);
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    float nMt = (cID + 1) * tScale;
    vec3 tangent = Mf(nMt) - aPos;
    vec3 unitTan = tangent / distance(tangent,vec3(0));

    fragColor = abs(unitTan * colormask * tScale);
}
