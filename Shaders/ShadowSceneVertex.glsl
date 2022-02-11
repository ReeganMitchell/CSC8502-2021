#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix[6];

uniform vec3 lightPos;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec2 texCoord;
in vec4 tangent;

out Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj[6];
} OUT;

void main(void){
	OUT.colour = colour;
	OUT.texCoord = texCoord;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wNormal, wTangent) * tangent.w;

	vec4 worldPos = (modelMatrix * vec4(position,1));
	OUT.worldPos = worldPos.xyz;
	gl_Position = (projMatrix * viewMatrix) * worldPos;

	vec3 viewDir = normalize(lightPos - worldPos.xyz);
	vec4 pushVal = vec4(OUT.normal, 0) * dot(viewDir, OUT.normal);
	for (int i = 0; i < 6; i++){
		OUT.shadowProj[i] = shadowMatrix[i] * (worldPos + pushVal);
	}
}