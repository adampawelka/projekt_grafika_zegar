#version 330

in vec3 normalDir;
in vec3 fragPos;

uniform vec4 color;     // bazowy kolor
uniform vec4 lp;        // pozycja światła

out vec4 outColor;

void main() {
    vec3 norm = normalize(normalDir);
    vec3 lightDir = normalize(lp.xyz - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = 0.4 * color.rgb;     // stałe oświetlenie otoczenia
    vec3 diffuse = 1.2 * diff * color.rgb; // mocne światło kierunkowe

    outColor = vec4(ambient + diffuse, color.a);
}
