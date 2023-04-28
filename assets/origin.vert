#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragColor;

void main()
{
    int dir = int( floor( gl_VertexID / 2 ) );
    switch(dir)
    {
    case 0:
        fragColor = vec3(1.0,0.0,0.0);
        break;
    case 1:
        fragColor = vec3(0.0,1.0,0.0);
        break;
    case 2:
        fragColor = vec3(0.0,0.0,1.0);
        break;
    };

    gl_Position = projection * view * vec4(aPos, 1.0);
};