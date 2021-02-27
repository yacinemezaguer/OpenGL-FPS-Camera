#version 330 core

out vec4 FragColor;
uniform vec4 color;

in vec3 ourColor;

uniform sampler2D ourTexture;

void main(){
	FragColor = (0.5*vec4(ourColor, 1.0) + 0.5*color);
}