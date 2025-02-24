#version 430

in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingSpotLightDir;
in vec2 varyingTexCoord;
in vec3 originalVertex;

out vec4 fragColor;

// 光照和材质结构
struct PositionalLight {
    vec4 ambient;  
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
    float cutoff; // 截止角度的余弦值
    float exponent; // 衰减指数
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform SpotLight spotLight;

// 距离衰减参数
uniform float constantAttenuation = 0.7;
uniform float linearAttenuation = 0.04;
uniform float quadraticAttenuation = 0.032;

// 深度图和光源变换
uniform sampler2D shadowMap;  // 深度图纹理
uniform mat4 lightSpaceMatrix; // 光源空间矩阵

// 计算阴影的函数，使用PCF
float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // 归一化到[0,1]区间

    // 检查是否在深度图范围内
    if(projCoords.z > 1.0) return 0.0;

    float shadow = 0.0;
    float bias = 0.005;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += (projCoords.z - bias > pcfDepth ? 1.0 : 0.0);
        }    
    }
    shadow /= 9.0;

    return shadow;
}

uniform sampler2D textureSampler;

void main(void)
{
    vec3 L = normalize(varyingLightDir);
    vec3 N = normalize(varyingNormal);
    vec3 V = normalize(-varyingVertPos);

    //float a = 1.25; // a 控制凸起的高度
    //float b = 100.0; // b 控制凸起的宽度
    //float x = originalVertex.x;
    //float y = originalVertex.y;
    //float z = originalVertex.z;
    //N.x = varyingNormal.x + a*sin(b*x); // 使用正弦函数扰乱传入法向量
    //N.y = varyingNormal.y + a*sin(b*y);
    //N.z = varyingNormal.z + a*sin(b*z);
    //N = normalize(N);

    vec3 R = normalize(reflect(-L, N));

    float cosTheta = dot(L, N);
    float cosPhi = dot(V, R);

    float distance = length(varyingLightDir);
    float attenuation = 1.0 / (constantAttenuation + linearAttenuation * distance + quadraticAttenuation * distance * distance);

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(varyingVertPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace);

    vec3 textureColor = texture(textureSampler, varyingTexCoord).rgb;

    vec3 ambient = ((globalAmbient) + (light.ambient)).xyz;
    vec3 diffuse = (light.diffuse.xyz) * max(cosTheta, 0.0);
    vec3 specular = (light.specular.xyz) * max(cosPhi, 0.0);

    vec3 lightColor = (ambient + diffuse + specular);

    vec3 spotLightDir = normalize(varyingSpotLightDir); 
    float spotLightIntensity = dot(spotLightDir, normalize(spotLight.direction));
    float spotLightFactor = 0.0;
    if (spotLightIntensity > spotLight.cutoff) {
        spotLightFactor = pow(spotLightIntensity, spotLight.exponent);
        vec3 spotDiffuse = spotLight.diffuse.xyz * max(dot(spotLightDir, N), 0.0);
        vec3 spotSpecular = spotLight.specular.xyz * max(dot(V, reflect(-spotLightDir, N)), 0.0);
        float spotShadow = ShadowCalculation(fragPosLightSpace);
        lightColor += 10.0 * spotLightFactor * (spotDiffuse + spotSpecular) * (1.0 - spotShadow);
        lightColor /= 5;
    }

    fragColor = vec4(0.75 * textureColor + 0.25 * lightColor, 1.0);
}