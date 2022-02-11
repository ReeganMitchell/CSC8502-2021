#version 330 core

uniform sampler2D diffuseTex1;
uniform sampler2D diffuseTex2;
uniform sampler2D diffuseTex3;
uniform sampler2D diffuseTex4;

uniform sampler2D bumpTex1;
uniform sampler2D bumpTex2;
uniform sampler2D bumpTex3;
uniform sampler2D bumpTex4;

uniform sampler2D mask;

uniform sampler2D shadowTex1;
uniform sampler2D shadowTex2;
uniform sampler2D shadowTex3;
uniform sampler2D shadowTex4;
uniform sampler2D shadowTex5;
uniform sampler2D shadowTex6;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj[6];
} IN;

out vec4 fragColour;

void main(void) {
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	mat3 TBN = mat3(normalize(IN.tangent),normalize(IN.binormal),normalize(IN.normal));

	vec4 maskTexel = texture(mask,IN.texCoord /64);
	vec4 texel1 = texture(diffuseTex1,IN.texCoord);
	texel1 *= maskTexel.r;
	texel1.a = 1.0;
	vec4 texel2 = texture(diffuseTex2,IN.texCoord);
	texel2 *= maskTexel.g;
	texel2.a = 1.0;
	vec4 texel3 = texture(diffuseTex3,IN.texCoord);
	texel3 *= maskTexel.b;
	texel3.a = 1.0;
	vec4 texel4 = texture(diffuseTex4,IN.texCoord);
	texel4 *= 1 - maskTexel.a;
	texel4.a = 1.0;

	vec4 diffuse = texel1 + texel2 + texel3 + texel4;

	vec3 bumpNormal1 = texture(bumpTex1,IN.texCoord).rgb;
	bumpNormal1 *= maskTexel.r;
	vec3 bumpNormal2 = texture(bumpTex2,IN.texCoord).rgb;
	bumpNormal2 *= maskTexel.g;
	vec3 bumpNormal3 = texture(bumpTex3,IN.texCoord).rgb;
	bumpNormal3 *= maskTexel.b;
	vec3 bumpNormal4 = texture(bumpTex4,IN.texCoord).rgb;
	bumpNormal4 *= 1 - maskTexel.a;

	vec3 normal = bumpNormal1 + bumpNormal2 + bumpNormal3 + bumpNormal4;
	normal = normalize(TBN * normalize(normal * 2.0 - 1.0));

	float lambert = max(dot(incident, normal), 0.0f);
	float distance = length(lightPos - IN.worldPos);
	float attenuation = 1.0 - clamp(distance / lightRadius, 0.0, 1.0);

	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0);

	float shadow = 1.0;

	vec3 shadowNDC = IN.shadowProj[0].xyz / IN.shadowProj[0].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex1, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	shadowNDC = IN.shadowProj[1].xyz / IN.shadowProj[1].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex2, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	shadowNDC = IN.shadowProj[2].xyz / IN.shadowProj[2].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex3, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	shadowNDC = IN.shadowProj[3].xyz / IN.shadowProj[3].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex4, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	shadowNDC = IN.shadowProj[4].xyz / IN.shadowProj[4].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex5, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	shadowNDC = IN.shadowProj[5].xyz / IN.shadowProj[5].w;
	if (abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f ){
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex6, biasCoord.xy).x;
		if (shadowZ < biasCoord.z) {
			shadow = 0.0f;
		}
	}

	vec3 surface = (diffuse.rgb * lightColour.rgb);
	fragColour.rgb = surface * attenuation * lambert;
	fragColour.rgb += (lightColour.rgb * attenuation * specFactor) * 0.33;
	fragColour.rgb *= shadow;
	fragColour.rgb += surface * 0.15f;

	fragColour.a = diffuse.a;
}