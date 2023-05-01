#version 330 core

out vec3 fragColor;

uniform float tScale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int startID;

void main(){
    int cID = gl_VertexID + int( (startID*4) / tScale );
    float Mt = floor( float(cID) / 4.0 ) * tScale;
    vec3 aPos = Mf(Mt);

    float nMt = floor( float(cID + 1) / 4.0 ) * tScale;
    vec3 nPos = Mf(nMt);

    switch ( abs( int( mod( cID, 4.0 ) ) ) )
    {
    case 0:
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        break;
    case 1:
        gl_Position = projection * view * model * vec4(aPos.x, 0.0, aPos.z, 1.0);
        break;
    case 2:
        gl_Position = projection * view * model * vec4(nPos, 1.0);
        break;
    case 3:
        gl_Position = projection * view * model * vec4(nPos.x, 0.0, nPos.z, 1.0);
        break;
    }

    fragColor = vec3(0.0);    
}