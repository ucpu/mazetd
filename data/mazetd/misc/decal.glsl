
$include /cage/shader/shaderConventions.h

$include /cage/shader/engine/vertex.glsl

void main()
{
	updateVertex();
}

$include /cage/shader/engine/fragment.glsl

$include /cage/shader/functions/reconstructPosition.glsl

layout(binding = CAGE_SHADER_TEXTURE_DEPTH) uniform sampler2D texDepth;

void main()
{
	vec3 prevPos = reconstructPosition(texDepth, uniViewport.vpInv, uniViewport.viewport);
	if (length(varPosition - prevPos) > 0.2)
		discard;
	updateNormal();
	Material material = loadMaterial();
#ifdef CutOut
	if (material.opacity < 0.5)
		discard;
#endif // CutOut
	outColor = lighting(material);
}
