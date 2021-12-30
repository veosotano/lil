/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Metal shaders used for this sample
*/

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef enum LILVertexInputIndex
{
    LILVertexInputIndexVertices = 0,
    LILVertexInputIndexUniforms = 1,
} LILVertexInputIndex;

typedef struct
{
    float x;
    float y;

    float red;
    float green;
    float blue;
    float alpha;
} LILVertex;

typedef struct
{
    float scale;
    vector_uint2 viewportSize;
} LILUniforms;

// Vertex shader outputs and per-fragment inputs
struct RasterizerData
{
    float4 clipSpacePosition [[position]];
    float3 color;
};

vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]],
             constant LILVertex *vertexArray [[ buffer(LILVertexInputIndexVertices) ]],
             constant LILUniforms &uniforms  [[ buffer(LILVertexInputIndexUniforms) ]])

{
    RasterizerData out;

    LILVertex vtx = vertexArray[vertexID];
    float2 pixelSpacePosition = float2(vtx.x, vtx.y);

    // Scale the vertex by scale factor of the current frame
    pixelSpacePosition *= uniforms.scale;

    float2 viewportSize = float2(uniforms.viewportSize);
    
    pixelSpacePosition -= viewportSize / 2.0;

    // Divide the pixel coordinates by half the size of the viewport to convert from positions in
    // pixel space to positions in clip space
    out.clipSpacePosition.xy = pixelSpacePosition / (viewportSize / 2.0);
    out.clipSpacePosition.z = 0.0;
    out.clipSpacePosition.w = 1.0;

    out.color = float3(vtx.red, vtx.green, vtx.blue);

    return out;
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]])
{
    return float4(in.color, 1.0);
}

