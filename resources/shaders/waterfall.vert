#version 440
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 0) out vec2 vUv;

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;
    vec4 params;   // x: minDb, y: maxDb, z: newest row center (0..1), w: (H-1)/H
} ubuf;

void main()
{
    vUv = uv;
    gl_Position = ubuf.mvp * vec4(pos, 0.0, 1.0);
}