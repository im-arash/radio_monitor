#version 440
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;
    vec4 color;
    vec2 range;
} ubuf;

void main()
{
    fragColor = ubuf.color;
}