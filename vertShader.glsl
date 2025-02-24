#version 430

layout (location = 0) in vec3 vertPos;
layout (location = 2) in vec3 vertNormal;
layout (location = 1) in vec2 vertTexCoord;

out vec3 varyingNormal;
out vec3 varyingLightDir;
out vec3 varyingVertPos;
out vec3 varyingSpotLightDir;
out vec2 varyingTexCoord;
out vec3 originalVertex;

struct PositionalLight
{	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};

struct SpotLight {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
	vec3 direction;
	float cutoff; // ��ֹ�Ƕȵ�����ֵ
	float exponent; // ˥��ָ��
};

struct Material
{	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform SpotLight spotLight;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;

void main(void)
{	
	originalVertex = vertPos;
	varyingVertPos = (mv_matrix * vec4(vertPos,1.0)).xyz;
	varyingLightDir = light.position - varyingVertPos;
	varyingSpotLightDir = spotLight.position - varyingVertPos;
	varyingNormal = (norm_matrix * vec4(vertNormal,1.0)).xyz;
	varyingTexCoord = vertTexCoord;

	gl_Position = proj_matrix * mv_matrix * vec4(vertPos,1.0);
}
