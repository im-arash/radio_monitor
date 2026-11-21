#version 440
layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;
    vec4 params;
} ubuf;

layout(binding = 1) uniform sampler2D tex;

vec3 ramp(float x)
{
    const vec3 c0 = vec3(0.02, 0.02, 0.12);
    const vec3 c1 = vec3(0.00, 0.25, 0.75);
    const vec3 c2 = vec3(0.00, 0.80, 0.50);
    const vec3 c3 = vec3(0.95, 0.90, 0.10);
    const vec3 c4 = vec3(1.00, 0.15, 0.05);
    if (x < 0.25) return mix(c0, c1, x / 0.25);
    if (x < 0.50) return mix(c1, c2, (x - 0.25) / 0.25);
    if (x < 0.75) return mix(c2, c3, (x - 0.50) / 0.25);
    return mix(c3, c4, (x - 0.75) / 0.25);
}

void main()
{
    // v = 0 (top) -> newest row, v = 1 (bottom) -> oldest row
    float y = ubuf.params.z - vUv.y * ubuf.params.w;
    float t = texture(tex, vec2(vUv.x, y)).r;          // 0..1 maps to -160..0 dBm
    float db = t * 160.0 - 160.0;
    float n = clamp((db - ubuf.params.x) / (ubuf.params.y - ubuf.params.x), 0.0, 1.0);
    fragColor = vec4(ramp(n), 1.0);
}