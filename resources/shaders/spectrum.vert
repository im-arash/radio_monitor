#version 440
layout(location = 0) in vec2 pos;   // x: 0..1, y: power in dBm

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;       // clip-space correction (backend Y flip)
    vec4 color;
    vec2 range;     // minDb, maxDb
} ubuf;

void main()
{
    float y = (pos.y - ubuf.range.x) / (ubuf.range.y - ubuf.range.x);
    gl_Position = ubuf.mvp * vec4(pos.x * 2.0 - 1.0, y * 2.0 - 1.0, 0.0, 1.0);
}