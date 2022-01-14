#version 410 core


in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

in vec4 fPos;

out vec4 fColor;

uniform vec3 viewPos;
//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform vec3 pointLightPositions[2];
uniform vec3 pointLightColors[2];
uniform float pointLightConsts[2];
uniform float pointLightLinears[2];
uniform float pointLightQuads[2];

float ambientStrength = 0.2f;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//fog
uniform float fogDensity;

vec3 ambient;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
void computeDirectionalLight()
{		
    //ambient
	ambient += ambientStrength * lightColor;

	//diffuse
	vec3 norm = normalize(fNormal);
	vec3 lightDir_normalized = normalize(-lightDir); //directional light
	diffuse += max(dot(norm, lightDir_normalized), 0.0f) * lightColor;

	//spec
	vec3 viewDir_normalized = normalize(-fPos.xyz);
	vec3 reflection = reflect(-lightDir_normalized, norm);
	float spec = pow(max(dot(viewDir_normalized, reflection), 0.0f), shininess);
	specular += specularStrength * spec * lightColor;
}


void computePointLight(vec3 lightPosition, vec3 lightColor, float constant, float lin, float quad) {
	float distance = length(lightPosition - fPos.xyz);
	float att = 1.0f / (constant + lin * distance + quad * distance * distance);

	//ambient
	ambient += att * ambientStrength * lightColor;

	//diffuse
	vec3 norm = normalize(fNormal);
	vec3 lightDir_normalized = normalize(lightPosition - fPos.xyz); //point light
	diffuse += att * max(dot(norm, lightDir_normalized), 0.0f) * lightColor;

	//spec
	vec3 viewDir_normalized = normalize(viewPos - fPos.xyz);
	vec3 reflection = reflect(-lightDir_normalized, norm);
	float spec = pow(max(dot(viewDir_normalized, reflection), 0.0f), 32);
	specular += att * specularStrength * spec * lightColor;
}

float computeShadow(){

	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	if(normalizedCoords.z > 1){
		return 0.0f;
	}

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	// Check whether current frag pos is in shadow
// Check whether current frag pos is in shadow
	float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;

}

float computeFog()
{
    float fragmentDistance = length(fPos);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);

}

void main() 
{
	ambient = vec3(0.0f, 0.0f, 0.0f);
	diffuse = vec3(0.0f, 0.0f, 0.0f);
	specular = vec3(0.0f, 0.0f, 0.0f);

	computeDirectionalLight();

	computePointLight(pointLightPositions[0], pointLightColors[0], pointLightConsts[0], pointLightLinears[0], pointLightQuads[0]);
	computePointLight(pointLightPositions[1], pointLightColors[1], pointLightConsts[1], pointLightLinears[1], pointLightQuads[1]);
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	//modulate with shadow
	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)* specular , 1.0f);

	float fogFactor = computeFog();
    	vec4 fogColor = vec4(0.9f, 0.85f, 0.45f, 1.0f);
    	fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
