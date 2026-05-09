#version 330 core
in vec2 TexCoord;
out vec4 FragColour;

uniform sampler2D tex;
uniform float colours[256 * 3];

void main() {
    int colour_index = int(round(texture(tex,TexCoord).r * 256.0));

    FragColour = vec4(
            colours[3 * colour_index + 0],
            colours[3 * colour_index + 1],
            colours[3 * colour_index + 2],
            1.0);
}
