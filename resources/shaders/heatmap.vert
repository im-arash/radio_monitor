#version 440
layout(location = 0) in vec2 corner;      // (u, v), v = 0 is the top (north)
layout(location = 0) out vec2 vUv;

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;
    vec4 rect;      // x, y, w, h as fractions of the item size
    vec4 params;    // x: intensity
} ubuf;

void main()
{
    vUv = corner;
    vec2 p = ubuf.rect.xy + corner * ubuf.rect.zw;
    gl_Position = ubuf.mvp * vec4(p.x * 2.0 - 1.0, 1.0 - p.y * 2.0, 0.0, 1.0);
}