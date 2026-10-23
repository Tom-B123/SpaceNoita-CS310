#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

layout(location = 2) in vec2  instance_pos;  // world-space centre
layout(location = 3) in vec2  instance_scale;// half extents
layout(location = 4) in float instance_angle;   // radians
layout(location = 5) in vec3  instance_colour;

uniform vec2 uWorldSize;

out vec2 TexCoord;
out vec3 Colour;

void main() {
    // rotate the unit quad corner
    float c = cos(instance_angle), s = sin(instance_angle);
    vec2 rotated = vec2(aPos.x * c - aPos.y * s,
                        aPos.x * s + aPos.y * c);
    vec2 world = instance_pos + rotated * (instance_scale);

    // world -> NDC (y flipped so +y is up on screen if you want)
    vec2 ndc = (world / uWorldSize) * 2.0;
    ndc.y = -ndc.y;

    gl_Position = vec4(ndc, 0.0, 1.0);
    TexCoord = aTexCoord;
    Colour = instance_colour;
}
