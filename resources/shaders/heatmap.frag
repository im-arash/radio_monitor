#version 440
layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Buf {
    mat4 mvp;
    vec4 rect;
    vec4 params;
} ubuf;

layout(binding = 1) uniform sampler2D tex;

vec3 ramp(float x)
{
    const vec3 c0 = vec3(0.00, 0.35, 1.00);   // blue
    const vec3 c1 = vec3(0.00, 0.85, 0.85);   // cyan
    const vec3 c2 = vec3(0.40, 0.90, 0.20);   // green
    const vec3 c3 = vec3(1.00, 0.90, 0.00);   // yellow
    const vec3 c4 = vec3(1.00, 0.10, 0.05);   // red
    if (x < 0.25) return mix(c0, c1, x / 0.25);
    if (x < 0.50) return mix(c1, c2, (x - 0.25) / 0.25);
    if (x < 0.75) return mix(c2, c3, (x - 0.50) / 0.25);
    return mix(c3, c4, (x - 0.75) / 0.25);
}

void main()
{
    float v = texture(tex, vUv).r;
    float a = smoothstep(0.02, 0.15, v) * ubuf.params.x;
    fragColor = vec4(ramp(v) * a, a);          // premultiplied alpha
}