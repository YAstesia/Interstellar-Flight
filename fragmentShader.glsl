#version 430 core

in vec3 FragPos;      // �Ӷ�����ɫ�����ݵ�Ƭ��λ��
in vec3 Normal;       // �Ӷ�����ɫ�����ݵķ���
out vec4 FragColor;   // �����ɫ

uniform vec3 lightPosition;  // ��Դλ��
uniform vec3 lightColor;     // ��Դ��ɫ
uniform vec3 viewPosition;   // �����λ��
uniform vec3 objectColor;    // ������ɫ

void main() {
    // ������
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // ������
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ���淴��
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // ������ɫ
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
