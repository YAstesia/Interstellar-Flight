#version 430 core

in vec3 FragPos;      // 从顶点着色器传递的片段位置
in vec3 Normal;       // 从顶点着色器传递的法线
out vec4 FragColor;   // 输出颜色

uniform vec3 lightPosition;  // 光源位置
uniform vec3 lightColor;     // 光源颜色
uniform vec3 viewPosition;   // 摄像机位置
uniform vec3 objectColor;    // 物体颜色

void main() {
    // 环境光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // 最终颜色
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
