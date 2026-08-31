#version 430 core

out vec4 FragColor;

uniform sampler2D Output;
in vec2 TexCoords;

void main () {
    vec3 OutputData = texture(Output, TexCoords).rgb;
    vec3 Result;

    Result = OutputData;
    FragColor = vec4(Result, 1.0);
}