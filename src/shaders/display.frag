#version 430

layout(std430, binding = 0) buffer PixelBuffer {
    uint data[];
};

uniform uvec2 bufSize;      // Width of the buffer in pixels
uniform uint scale;         // Pixel scale factor
uniform uvec2 position;    // Position on screen

out vec4 fragColor;

void main() {
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);

    // Translate fragment coordinate to buffer space
    uvec2 bufCoord = (fragCoord + position) / scale;

    // If outside the scaled buffer area, output transparent
    if (bufCoord.x < 0 || bufCoord.y < 0 || bufCoord.x >= bufSize.x || bufCoord.y >= bufSize.y) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0); 
        return;
    }

    // Compute bit index
    uint index = bufCoord.y * bufSize.x + bufCoord.x;
    uint wordIndex = index / 32;       // 32 bits per uint
    uint bitIndex  = index % 32;       // which bit in the uint

    uint word = data[wordIndex];
    uint bit = (word >> bitIndex) & 1u;

    fragColor = (bit == 1u) ? vec4(1.0) : vec4(vec3(0.0),1.0);
}
