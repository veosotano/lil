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
	
	float textureX;
	float textureY;

} LILVertex;

typedef struct
{
	float scale;
	unsigned int targetSizeX;
	unsigned int targetSizeY;
	unsigned int targetPosX;
	unsigned int targetPosY;
	unsigned int viewportSizeX;
	unsigned int viewportSizeY;
	float time;
} LILUniforms;

// Vertex shader outputs and per-fragment inputs
struct RasterizerData
{
	float4 position [[position]];
	float4 color;
	float2 texture;
	float2 targetSize;
	float2 targetPos;
	float2 viewportSize;
	float time;
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

	out.targetSize = float2(uniforms.targetSizeX, uniforms.targetSizeY) * uniforms.scale;
	out.targetPos = float2(uniforms.targetPosX, uniforms.targetPosY) * uniforms.scale;
	out.viewportSize = float2(uniforms.viewportSizeX, uniforms.viewportSizeY);
	out.time = uniforms.time;

	pixelSpacePosition -= out.viewportSize / 2.0;

	// Divide the pixel coordinates by half the size of the viewport to convert from positions in
	// pixel space to positions in clip space
	out.position.xy = pixelSpacePosition / (out.viewportSize / 2.0);
	out.position.z = 0.0;
	out.position.w = 1.0;

	out.color = float4(vtx.red, vtx.green, vtx.blue, vtx.alpha);
	
	out.texture = float2(vtx.textureX, vtx.textureY);

	return out;
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]])
{
	return in.color;
}

fragment float4
textureShader(  RasterizerData in [[stage_in]],
				texture2d<float> texture [[texture(0)]] )
{
	constexpr sampler theSampler(coord::normalized);
	float4 color = texture.sample(theSampler, in.texture);
	return float4(color.r, color.g, color.b, color.a);
}

fragment float4
customFragmentShader(RasterizerData in [[stage_in]])
{
	float2 pos = in.position.xy;
	pos.x -= in.targetPos.x;
	pos.y -= (in.viewportSize.y - in.targetSize.y) - in.targetPos.y;

	float2 uv = pos / in.viewportSize;
	uv *= (in.viewportSize / in.targetSize);
	uv.y = 1 - uv.y;

	uv -= 0.5;
	uv.x *= in.targetSize.x / in.targetSize.y;
	return float4(float3(in.color.r, in.color.g, in.color.b), 1.0);
}
