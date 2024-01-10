#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

uniform float alpha;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

    // Output color = color of the texture at the specified UV
    vec4 pixel = texture( myTextureSampler, UV );
    color.rgb = pixel.rgb;
    color.a = alpha * pixel.a;
}