#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texIn;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 normalDir;
out vec2 texCoord;
out vec3 fragPos;

void main() {
    fragPos = vec3(M * vec4(vertex, 1.0));
    normalDir = normalize(mat3(transpose(inverse(M))) * normal);
    texCoord = texIn;
    gl_Position = P * V * M * vec4(vertex, 1.0);
}
