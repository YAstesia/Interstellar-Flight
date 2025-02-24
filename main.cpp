#define GLM_ENABLE_EXPERIMENTAL // ����ʵ������չ
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL2\soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stack>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp> // glm::value_ptr
#include <glm\gtc\matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm\gtx\string_cast.hpp>
#include "Sphere.h"
#include "ImportedModel.h"
#include "Utils.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cmath>
#include <set>
#pragma comment(lib, "winmm.lib")

using namespace std;

#define numVAOs 5
#define numVBOs 15

float cameraX, cameraY, cameraZ, cameraYaw, cameraPitch; // ���λ��
float sphLocX, sphLocY, sphLocZ; // ����λ��
bool isThirdPerson = false; // �����ӽ��л�
GLuint renderingProgram; // ��Ⱦ����
GLuint skyboxRenderingProgram;
GLuint vao[numVAOs]; // VAO����
GLuint vbo[numVBOs]; // VBO����
GLuint earthTexture; // ����ID
GLuint sunTexture;
GLuint moonTexture;
GLuint jupiterTexture;
GLuint marsTexture;
GLuint neptuneTexture;
GLuint uranusTexture;
GLuint mercuryTexture;
GLuint venusTexture;
GLuint saturnTexture;
GLuint shuttleTexture;
GLuint rockTexture;
GLuint skyboxTexture, earthSkyboxTexture;
GLuint basketballTexture;
GLuint backgroundTexture;
GLuint backgroundVAO, backgroundVBO; // Ϊ��������������VAO��VBO
ImportedModel SpaceCraftModel("shuttle.obj");
ImportedModel CameraModel("shuttle.obj");
ImportedModel RockModel("rock.obj");
string currentStar;
string detailStar;
float currentDis;
string detail;
bool showProximityAlert = false;
int score = 10000;
set<string> triggeredStars; // ��¼�Ѵ���������
set<string> allStars = {
    "Saturn", "Earth", "Mars", "Venus", "Mercury", "Jupiter", "Sun"
};
vector<string> space = {
    "Space.png","Space1.png","Space2.png","Space3.png","Space4.png"
};
string skySkyBox = "skyBox.jpg";
int spaceNum = 0;

vector<vector<string>>renderingProgramName = { {"./BallvertShader.glsl", "./BallfragShader.glsl"},{"./vertShader.glsl", "./fragShader.glsl"} };
int renderingProgramNum = 0;

vector<GLuint>RenderingPrograms;
// ��Դ��λ��
glm::vec3 lightLoc = glm::vec3(0.0f, 0.0f, 0.0f);
float amt = 0.0f; // ���ڿ��ƹ�Դ��ת�ĽǶ�
// �۹������
glm::vec3 spotLightPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, 1.0f, 0.0f);
float spotLightCutoff = 0.95f;
float spotLightExponent = 2.0f;

// ��ʼλ��Ϊ (0.0f, 0.0f, 10.5f)
glm::mat4 accumulatedTransform;

bool landed = false;//�����ڵ�����
float rotAmt = 0.0f; // ��ת�Ƕ�
float roatSpeedBasic = 0.2f;
bool crazy = 0;

float toRadians(float degrees)
{
    return (degrees * 2.0f * 3.14159f) / 360.0f;
}

GLfloat vertices[] = {
    // positions          // texture coords
    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,  // top left
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,  // bottom left
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // bottom right

    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,  // top left
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // bottom right
     1.0f,  1.0f, 0.0f,   1.0f, 1.0f   // top right
};

// ��������������ʾ
GLuint mvLoc, projLoc, nLoc; // ���ȱ���λ��
GLint spotLightAmbLoc, spotLightDiffLoc, spotLightSpecLoc, spotLightPosLoc, spotLightDirectionLoc, spotLightCutoffLoc, spotLightExponentLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;// ��ɫ���й��պͲ������Ե�λ��
int width, height; // ���ڿ�Ⱥ͸߶�
float aspect; // ��߱�
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat; // ͶӰ������ͼ����ģ�;���ģ����ͼ������ת��ģ����ͼ�������ת����
glm::vec3 currentLightPos, currentSpotLightPos, currentSpotLightDirection, transformed; // ��ǰ��Դλ�úͱ任��Ĺ�Դλ��
float lightPos[3]; // ���ڴ��ݹ�Դλ�õ�����

// ���Դ����
float globalAmbient[4] = { 0.01f, 0.01f, 0.01f, 1.0f }; // ȫ�ֻ�����
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // ��Դ�Ļ�����
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // ��Դ���������
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // ��Դ�ľ��淴���

// ����۹�����ԵĽṹ��
struct SpotLight {
    float ambient[4];      // ��������ɫ
    float diffuse[4];      // ���������ɫ
    float specular[4];     // ���淴�����ɫ
    float direction[3];    // �۹�ƹ��߷���
};
// ��ʼ���۹������
vector<SpotLight> spotLights = {
    // ��ɫ�۹��
    {
        { 0.1f, 0.0f, 0.0f, 1.0f }, // ��͵ĺ�ɫ������
        { 1.0f, 0.0f, 0.0f, 1.0f }, // ǿ�ҵĺ�ɫ�������
        { 1.0f, 0.0f, 0.0f, 1.0f }, // �����ĺ�ɫ���淴���
        { 0.0f, -1.0f, 0.0f }       // ���µķ���
    },
    // ��ɫ�۹��
    {
        { 0.05f, 0.05f, 0.2f, 1.0f }, // ��͵���ɫ������
        { 0.0f, 0.0f, 1.0f, 1.0f },   // ǿ�ҵ���ɫ�������
        { 0.0f, 0.0f, 1.0f, 1.0f },   // ��������ɫ���淴���
        { 0.0f, -1.0f, 0.0f }         // ���µķ���
    },
    // ��ɫ�۹��
    {
        { 0.1f, 0.1f, 0.0f, 1.0f }, // ��͵Ļ�ɫ������
        { 1.0f, 1.0f, 0.0f, 1.0f }, // ǿ�ҵĻ�ɫ�������
        { 1.0f, 1.0f, 0.0f, 1.0f }, // �����Ļ�ɫ���淴���
        { 0.0f, -1.0f, 0.0f }       // ���µķ���
    },
    // ��ɫ�۹��
    {
        { 0.1f, 0.1f, 0.1f, 1.0f }, // ��͵İ�ɫ������
        { 1.0f, 1.0f, 1.0f, 1.0f }, // ǿ�ҵİ�ɫ�������
        { 1.0f, 1.0f, 1.0f, 1.0f }, // �����İ�ɫ���淴���
        { 0.0f, -1.0f, 0.0f }       // ���µķ���
    },
    // �޺��ɫ�۹��
    {
        { 0.1f, 0.0f, 0.1f, 1.0f }, // ��͵ķ�ɫ������
        { 1.0f, 0.0f, 0.5f, 1.0f }, // ǿ�ҵ��޺��ɫ�������
        { 1.0f, 0.0f, 0.5f, 1.0f }, // �������޺��ɫ���淴���
        { 0.0f, -1.0f, 0.0f }       // ���µķ���
    }
};

int numOfSpotLight = 0;

// �۹�ƹ�Դ����
float dirLightAmbient[4] = { 0.1f, 0.0f, 0.0f, 1.0f }; // ��Դ�Ļ�����
float dirLightDiffuse[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // ��Դ���������
float dirLightSpecular[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // ��Դ�ľ��淴���
float dirLightDirection[3] = { 0.0f,-1.0f,0.0f };//����

// �������ԣ���ɫ��
float* matAmb = Utils::goldAmbient(); // ���ʵĻ�����
float* matDif = Utils::goldDiffuse(); // ���ʵ��������
float* matSpe = Utils::goldSpecular(); // ���ʵľ��淴���
float matShi = Utils::goldShininess(); // ���ʵĸ߹�ָ��

Sphere SphereSun = Sphere(48, 0.25f); // �����������ϸ�ּ���Ϊ48
//Sphere SphereEarth= Sphere(48, 0.1f); // �����������ϸ�ּ���Ϊ48

float calculateDistance(glm::vec3 cameraPos, glm::vec3 planetPos) {
    if (isThirdPerson) {
        cameraPos.y -= 1.0f;
        cameraPos.z -= 2.8f;
    }
    return glm::length(cameraPos - planetPos); // �������������֮��ľ���
}

bool hasLight = true;
bool hasSpotLight = false;
// ���ù�Դ�Ͳ�������
void installLights(glm::mat4 vMatrix) {
    float zeroVec4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float zeroVec3[3] = { 0.0f, 0.0f, 0.0f };
    if (hasLight)
    {
        // �����Դ����ͼ�ռ��е�λ��
        glm::vec3 transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
        lightPos[0] = transformed.x;
        lightPos[1] = transformed.y;
        lightPos[2] = transformed.z;

        // ��ȡ��ɫ���б�����λ��
        globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
        ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
        diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
        specLoc = glGetUniformLocation(renderingProgram, "light.specular");
        posLoc = glGetUniformLocation(renderingProgram, "light.position");
        mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
        mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
        mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
        mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

        // ������ɫ���еĹ��պͲ�������
        glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
        glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
        glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
        glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
        glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
        glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, matShi);
    }
    else
    {
        // ���û�����ù��գ������������
        glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, zeroVec4); // ȫ�ֻ�����
        glProgramUniform4fv(renderingProgram, ambLoc, 1, zeroVec4);       // ������
        glProgramUniform4fv(renderingProgram, diffLoc, 1, zeroVec4);      // �������
        glProgramUniform4fv(renderingProgram, specLoc, 1, zeroVec4);      // ���淴���
        glProgramUniform3fv(renderingProgram, posLoc, 1, zeroVec3);       // ��Դλ��
    }

    if (hasSpotLight)
    {
        // ����۹������ͼ�ռ��е�λ�úͷ���
        glm::vec3 spotTransformedPos = glm::vec3(vMatrix * glm::vec4(currentSpotLightPos, 1.0)); // �۹��λ��
        glm::vec3 spotTransformedDir = glm::vec3(vMatrix * glm::vec4(currentSpotLightDirection, 0.0)); // �۹�Ʒ���

        // ��ȡ�۹�����Ե� uniform ����λ��
        spotLightAmbLoc = glGetUniformLocation(renderingProgram, "spotLight.ambient");
        spotLightDiffLoc = glGetUniformLocation(renderingProgram, "spotLight.diffuse");
        spotLightSpecLoc = glGetUniformLocation(renderingProgram, "spotLight.specular");
        spotLightPosLoc = glGetUniformLocation(renderingProgram, "spotLight.position");
        spotLightDirectionLoc = glGetUniformLocation(renderingProgram, "spotLight.direction");
        spotLightCutoffLoc = glGetUniformLocation(renderingProgram, "spotLight.cutoff");
        spotLightExponentLoc = glGetUniformLocation(renderingProgram, "spotLight.exponent");

        // ���þ۹������
        glProgramUniform4fv(renderingProgram, spotLightAmbLoc, 1, spotLights[numOfSpotLight].ambient);
        glProgramUniform4fv(renderingProgram, spotLightDiffLoc, 1, spotLights[numOfSpotLight].diffuse);
        glProgramUniform4fv(renderingProgram, spotLightSpecLoc, 1, spotLights[numOfSpotLight].specular);
        glProgramUniform3fv(renderingProgram, spotLightPosLoc, 1, glm::value_ptr(spotTransformedPos));
        glProgramUniform3fv(renderingProgram, spotLightDirectionLoc, 1, glm::value_ptr(spotTransformedDir));
        glProgramUniform1f(renderingProgram, spotLightCutoffLoc, spotLightCutoff);
        glProgramUniform1f(renderingProgram, spotLightExponentLoc, spotLightExponent);
    }
    else
    {
        // ���û�����þ۹�ƣ�����۹������
        glProgramUniform4fv(renderingProgram, spotLightAmbLoc, 1, zeroVec4); // �۹�ƻ�����
        glProgramUniform4fv(renderingProgram, spotLightDiffLoc, 1, zeroVec4); // �۹���������
        glProgramUniform4fv(renderingProgram, spotLightSpecLoc, 1, zeroVec4); // �۹�ƾ��淴���
        glProgramUniform3fv(renderingProgram, spotLightPosLoc, 1, zeroVec3);  // �۹��λ��
        glProgramUniform3fv(renderingProgram, spotLightDirectionLoc, 1, zeroVec3); // �۹�Ʒ���
        glProgramUniform1f(renderingProgram, spotLightCutoffLoc, 0.0f); // �۹�ƽ�ֹ��
        glProgramUniform1f(renderingProgram, spotLightExponentLoc, 0.0f); // �۹�ƹ�˥��ָ��
    }
}
void setupVerticesForSphere(Sphere& sphere, GLuint vaoIndex) {
    std::vector<int> ind = sphere.getIndices(); // ��ȡ����
    std::vector<glm::vec3> vert = sphere.getVertices(); // ��ȡ����
    std::vector<glm::vec2> tex = sphere.getTexCoords(); // ��ȡ��������
    std::vector<glm::vec3> norm = sphere.getNormals(); // ��ȡ����

    std::vector<float> pvalues; // ��������
    std::vector<float> tvalues; // ������������
    std::vector<float> nvalues; // ��������

    int numIndices = sphere.getNumIndices(); // ��ȡ��������
    for (int i = 0; i < numIndices; i++) {
        pvalues.push_back((vert[ind[i]]).x);
        pvalues.push_back((vert[ind[i]]).y);
        pvalues.push_back((vert[ind[i]]).z);
        tvalues.push_back((tex[ind[i]]).s);
        tvalues.push_back((tex[ind[i]]).t);
        nvalues.push_back((norm[ind[i]]).x);
        nvalues.push_back((norm[ind[i]]).y);
        nvalues.push_back((norm[ind[i]]).z);
    }

    glBindVertexArray(vao[vaoIndex]); // ��VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3]); // �󶨶���VBO
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW); // �ϴ���������

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 1]); // ����������VBO
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW); // �ϴ�������������

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 2]); // �󶨷���VBO
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW); // �ϴ���������
}
void setupVerticesForModel(ImportedModel& model, GLuint vaoIndex) {
    std::vector<glm::vec3> vert = model.getVertices();
    std::vector<glm::vec2> tex = model.getTextureCoords();
    std::vector<glm::vec3> norm = model.getNormals();

    std::vector<float> pvalues;
    std::vector<float> tvalues;
    std::vector<float> nvalues;

    for (int i = 0; i < model.getNumVertices(); i++) {
        pvalues.push_back((vert[i]).x);
        pvalues.push_back((vert[i]).y);
        pvalues.push_back((vert[i]).z);
        tvalues.push_back((tex[i]).s);
        tvalues.push_back((tex[i]).t);
        nvalues.push_back((norm[i]).x);
        nvalues.push_back((norm[i]).y);
        nvalues.push_back((norm[i]).z);
    }

    glBindVertexArray(vao[vaoIndex]); // ��VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3]); // �󶨶���VBO
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW); // �ϴ���������

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 1]); // ����������VBO
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW); // �ϴ�������������

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 2]); // �󶨷���VBO
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW); // �ϴ���������

}

// ���ö�������
void setupVertices(void) {
    glGenVertexArrays(numVAOs, vao); // ����VAO
    glGenBuffers(numVBOs, vbo); // ����VBO
    float cubeVertices[108] = {
 -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
 -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
 -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
 -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
 -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
 -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f
    };
    // ��պе���������������
    float cubeTextureCoord[72] = {
    1.00f, 0.6666f, 1.00f, 0.3333f, 0.75f, 0.3333f, // �������½�
    0.75f, 0.3333f, 0.75f, 0.6666f, 1.00f, 0.6666f, // �������Ͻ�
    0.75f, 0.3333f, 0.50f, 0.3333f, 0.75f, 0.6666f, // �������½�
    0.50f, 0.3333f, 0.50f, 0.6666f, 0.75f, 0.6666f, // �������Ͻ�
    0.50f, 0.3333f, 0.25f, 0.3333f, 0.50f, 0.6666f, // �������½�
    0.25f, 0.3333f, 0.25f, 0.6666f, 0.50f, 0.6666f, // �������Ͻ�
    0.25f, 0.3333f, 0.00f, 0.3333f, 0.25f, 0.6666f, // �������½�
    0.00f, 0.3333f, 0.00f, 0.6666f, 0.25f, 0.6666f, // �������Ͻ�
    0.25f, 0.3333f, 0.50f, 0.3333f, 0.50f, 0.00f, // �������½�
    0.50f, 0.00f, 0.25f, 0.00f, 0.25f, 0.3333f, // �������Ͻ�
    0.25f, 1.00f, 0.50f, 1.00f, 0.50f, 0.6666f, // �������½�
    0.50f, 0.6666f, 0.25f, 0.6666f, 0.25f, 1.00f // �������Ͻ�
    };
    // �󶨲���䶥��VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // �󶨲������������VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTextureCoord), cubeTextureCoord, GL_STATIC_DRAW);
    setupVerticesForSphere(SphereSun, 0);
    setupVerticesForModel(SpaceCraftModel, 1);
    setupVerticesForModel(RockModel, 2);
    setupVerticesForModel(CameraModel, 3);

    //setupVerticesForSphere(SphereEarth, 1 );
}


// ��ʼ������
void init(GLFWwindow* window) {
    for (int i = 0; i < renderingProgramName.size(); ++i) {
        RenderingPrograms.push_back(Utils::createShaderProgram(renderingProgramName[i][0].c_str(), renderingProgramName[i][1].c_str()));
    }
    renderingProgram = RenderingPrograms[renderingProgramNum];
    skyboxRenderingProgram = Utils::createShaderProgram("./skyboxVertShader.glsl", "./skyboxFragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("./BlinnPhongShaders/vertShader.glsl", "./BlinnPhongShaders/fragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("./GouraudShaders/vertShader.glsl", "./GouraudShaders/fragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl"); // ������������ɫ������
    cameraX = 0.0f; cameraY = 0.0f; cameraZ = 300.0f; // �������λ��
    accumulatedTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraX, cameraY, cameraZ + 0.25));
    cameraPitch = 0.0f, cameraYaw = -90.0f; // ����Ƕ�
    sphLocX = 0.0f; sphLocY = 0.0f; sphLocZ = -1.0f; // ��������λ��
    srand(static_cast<unsigned int>(time(0)));
    glfwGetFramebufferSize(window, &width, &height); // ��ȡ���ڳߴ�
    aspect = (float)width / (float)height; // �����߱�
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // ����͸��ͶӰ����

    setupVertices(); // ���ö�������
    earthTexture = Utils::loadTexture("earth.jpg");
    sunTexture = Utils::loadTexture("2k_sun.jpg");
    moonTexture = Utils::loadTexture("2k_moon.jpg");
    jupiterTexture = Utils::loadTexture("2k_jupiter.jpg");
    marsTexture = Utils::loadTexture("2k_mars.jpg");
    neptuneTexture = Utils::loadTexture("2k_neptune.jpg");
    uranusTexture = Utils::loadTexture("2k_uranus.jpg");
    mercuryTexture = Utils::loadTexture("mercury.jpg");
    venusTexture = Utils::loadTexture("venus.jpg");
    saturnTexture = Utils::loadTexture("2k_saturn.jpg");
    shuttleTexture = Utils::loadTexture("spstob_1.jpg");
    rockTexture = Utils::loadTexture("rock.png");
    basketballTexture = Utils::loadTexture("basketball.jpg");
    skyboxTexture = Utils::loadTexture(space[spaceNum].c_str());
    earthSkyboxTexture = Utils::loadTexture(skySkyBox.c_str());

    glEnable(GL_DEPTH_TEST); // ������Ȳ���
    //glDepthFunc(GL_LESS); // ������Ȳ��Ժ���
}

stack<glm::mat4>mvStack;

//// ��Ⱦģ�͵ĺ���
//void renderModelWithTransform(ImportedModel& model) {
//
//}

//void translateModelAlongX(float dx) {
//    glm::vec3 translation(dx, 0.0f, 0.0f);
//    accumulatedTransform = glm::translate(accumulatedTransform, translation);
//}

//void translateModelAlongY(float dy) {
//    glm::vec3 translation(0.0f, dy, 0.0f);
//    accumulatedTransform = glm::translate(accumulatedTransform, translation);
//}

//void translateModelAlongZ(float dz) {
//    glm::vec3 translation(0.0f, 0.0f, dz);
//    accumulatedTransform = glm::translate(accumulatedTransform, translation);
//}

vector<float> generateRandomAngle(int size, int max, int min) {
    srand(1);
    std::vector<float> randomNumbers(size);
    for (int i = 0; i < size; ++i) {
        randomNumbers[i] = static_cast<float>(rand()) / RAND_MAX * (max - min) + min;
    }
    return randomNumbers;
}


// ��ʾ����
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT); // �����Ȼ�����
    glClear(GL_COLOR_BUFFER_BIT); // �����ɫ������

    glUseProgram(renderingProgram);

    float threshold = 60.0f;
    float minDistance = std::numeric_limits<float>::max();
    string closestPlanet = "None";
    bool foundPlanet = false;
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix"); // ��ȡģ����ͼ������ȱ���λ��
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix"); // ��ȡͶӰ������ȱ���λ��
    nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

    // ���Ƹ����ǣ���ֹ������
    if (cameraPitch > 89.0f)
        cameraPitch = 89.0f;
    if (cameraPitch < -89.0f)
        cameraPitch = -89.0f;
    //if (cameraYaw > -1.0f)
    //    cameraYaw = -1.0f;
    //if (cameraYaw < -179.0f)
    //    cameraYaw = -179.0f;

    // ���� yaw �� pitch ���������ǰ������
    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front = glm::normalize(front);
    // ����������ҷ����Ϸ�����
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, front));
    up = glm::cross(front, right); // ȷ�� 'up' �������� 'front' �� 'right'
    // ������ͼ���� (View Matrix) ʹ�� glm::lookAt
    vMat = glm::lookAt(
        glm::vec3(cameraX, cameraY, cameraZ), // ���λ��
        glm::vec3(cameraX, cameraY, cameraZ) + front, // ���Ŀ���
        up // "��" ����
    );
    // ������ͼ���󣬽����λ���Ƶ�ԭ��
    //vMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));

     // �л�����պ�ר�õ���ɫ������
    glUseProgram(skyboxRenderingProgram);

    // ��ȡ��պ���ɫ���е�ͳһ����λ��
    GLint skyboxMvLoc = glGetUniformLocation(skyboxRenderingProgram, "mv_matrix");
    GLint skyboxProjLoc = glGetUniformLocation(skyboxRenderingProgram, "proj_matrix");

    // ����MODEL-VIEW���󣬽���պз����������λ��
    glm::mat4 mMat = glm::translate(glm::mat4(1.0f), glm::vec3(cameraX, cameraY, cameraZ));
    glm::mat4 mvMat = vMat * mMat;

    // ���ݾ������ɫ��
    glUniformMatrix4fv(skyboxMvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��

    // ���ð�������Ļ�����
    glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // ���ð�����������Ļ�����
    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // �������պ�����
    glActiveTexture(GL_TEXTURE0);
    if (!landed)
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    else
        glBindTexture(GL_TEXTURE_2D, earthSkyboxTexture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
    glEnableVertexAttribArray(2);// ���÷�������

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW); // ���������˳����˳ʱ��ģ������Ǵ��ڲ��鿴�����ʹ����ʱ�����˳��GL_CCW
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 36); // ��û����Ȳ��Ե�����»�����պ�
    glEnable(GL_DEPTH_TEST);
    // �ָ�֮ǰ����ɫ������
    glUseProgram(renderingProgram);

    mvStack.push(vMat);

    glm::vec3 cameraPos(cameraX, cameraY, cameraZ);
    glm::vec3 shipPosition;
    // ����ɴ���λ�ã����λ�� + ǰ������ * 1.5 - �Ϸ����� * 0.5
    if (isThirdPerson) {
        shipPosition = glm::vec3(cameraX, cameraY, cameraZ) + front * 1.5f - up * 0.5f;
        spotLightPos = shipPosition + 0.25f;
    }
    else {
        shipPosition = glm::vec3(cameraX, cameraY, cameraZ);
        spotLightPos = shipPosition;
    }
    // ���¹�Դλ��
    currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);

    currentSpotLightPos = glm::vec3(spotLightPos.x, spotLightPos.y, spotLightPos.z);

    // ������ת�Ƕ�
    //amt += 0.5f;
    // ������ת����ʹ��Դ�� z ����ת
    //rMat = glm::rotate(glm::mat4(1.0f), toRadians(amt), glm::vec3(0.0f, 0.0f, 1.0f));
    // Ӧ����ת���󣬸��¹�Դλ��
    //currentSpotLightPos = glm::vec3(rMat * glm::vec4(currentSpotLightPos, 1.0f));

    // ����۹�ƹ�Դ�ķ���������ʹ��ʼ��ָ��ԭ�� (0, 0, 0)
    glm::vec3 targetPosition = spotLightPos + front;
    currentSpotLightDirection = glm::normalize(currentSpotLightPos - targetPosition);
    // ���ù�Դ�Ͳ�������
    installLights(vMat);
    //��Դ
    //��������
    float* whiteAmb = Utils::whiteAmbient();
    float* whiteDif = Utils::whiteDiffuse();
    float* whiteSpe = Utils::whiteSpecular();
    float whiteShi = Utils::whiteShininess();
    glProgramUniform4fv(renderingProgram, mambLoc, 1, whiteAmb);
    glProgramUniform4fv(renderingProgram, mdiffLoc, 1, whiteDif);
    glProgramUniform4fv(renderingProgram, mspecLoc, 1, whiteSpe);
    glProgramUniform1f(renderingProgram, mshiLoc, whiteShi);
    //���Դ
    mMat = glm::translate(glm::mat4(1.0f), currentLightPos);
    mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
    mvMat = vMat * mMat;
    invTrMat = glm::transpose(glm::inverse(mvMat));

    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��

    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glEnable(GL_CULL_FACE); // ���ñ����޳�
    glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����
    glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices());

    // copper material
    float* copperAmb = Utils::copperAmbient();
    float* copperDif = Utils::copperDiffuse();
    float* copperSpe = Utils::copperSpecular();
    float copperShi = Utils::copperShininess();
    glProgramUniform4fv(renderingProgram, mambLoc, 1, copperAmb);
    glProgramUniform4fv(renderingProgram, mdiffLoc, 1, copperDif);
    glProgramUniform4fv(renderingProgram, mspecLoc, 1, copperSpe);
    glProgramUniform1f(renderingProgram, mshiLoc, copperShi);
    // ���Ʒɴ�
    if (isThirdPerson) {


        float scale = 0.25f; // ����ģ�ʹ�С

        // ����ǰ�ľ���ѹ���ջ
        mvStack.push(mvStack.top());

        mvStack.top() *= glm::translate(glm::mat4(1.0f), shipPosition);
        mvStack.top() = glm::rotate(mvStack.top(), glm::acos(glm::dot(glm::vec3(0, 0, -1), front)), glm::cross(glm::vec3(0, 0, -1), front));
        mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));

        // ������
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shuttleTexture);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[5]); // �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���÷�������ָ��
        glEnableVertexAttribArray(2); // ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        // �����µ�ģ�;��󴫵ݸ���ɫ��
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation11 = glGetUniformLocation(renderingProgram, "shuttleTexture");
        glUniform1i(textureLocation11, 0); // ������Ԫ0�󶨵�������

        // �󶨶��㻺����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // ������Ȳ��Ժͱ����޳�
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        // ����ģ��
        glDrawArrays(GL_TRIANGLES, 0, CameraModel.getNumVertices());

        // �ָ������ջ
        mvStack.pop();
    }

    if (!landed) {
        mvStack.top() = glm::scale(mvStack.top(), glm::vec3(100, 100, 100));

        // ̫��
        float* goldAmb = Utils::goldAmbient();
        float* goldDif = Utils::goldDiffuse();
        float* goldSpe = Utils::goldSpecular();
        float goldShi = Utils::goldShininess();
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, goldAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, goldDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, goldSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, goldShi);

        float sunSpeed = 10.0f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // ̫��λ��
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * sunSpeed, glm::vec3(0.0f, 1.0f, 0.0f)); // ̫����ת

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, sunTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        glm::vec3 sunPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation = glGetUniformLocation(renderingProgram, "sunTexture");
        glUniform1i(textureLocation, 0); // ������Ԫ0�󶨵�������

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glEnable(GL_CULL_FACE); // ���ñ����޳�
        glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        mvStack.pop(); // �Ӷ�ջ���Ƴ�̫��������ת


        // silver material
        float* silverAmb = Utils::silverAmbient();
        float* silverDif = Utils::silverDiffuse();
        float* silverSpe = Utils::silverSpecular();
        float silverShi = Utils::silverShininess();
        glProgramUniform4fv(renderingProgram, mambLoc, 1, silverAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, silverDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, silverSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, silverShi);
        if (crazy) {
            for (int i = 0; i < 1000; ++i) {
                mvStack.push(mvStack.top());
                //���
                float scale = 0.008f; // �޸������ֵ���ı�ģ�͵Ĵ�С
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f + (static_cast<float>(i)) * roatSpeedBasic), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 40)), glm::vec3(1.0, 0.0, 0.0)); // ��ת�Ƕ�
                mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 2.3, cos((float)currentTime * roatSpeedBasic) * 2.3));
                mvStack.top() *= glm::rotate(glm::mat4(1.0f),
                    (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
                mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
                glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
                glBindTexture(GL_TEXTURE_2D, rockTexture); // ������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);// �󶨷��߻������
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
                glEnableVertexAttribArray(2);// ���÷�������
                invTrMat = glm::transpose(glm::inverse(mvStack.top()));

                glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
                glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

                int textureLocation9 = glGetUniformLocation(renderingProgram, "rockTexture");
                glUniform1i(textureLocation9, 0); // ������Ԫ0�󶨵�������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // �󶨶���VBO
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
                glEnableVertexAttribArray(0); // ���ö�����������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // ����������VBO
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
                glEnableVertexAttribArray(1); // ��������������������

                glEnable(GL_CULL_FACE); // ���ñ����޳�
                glDepthFunc(GL_LEQUAL);
                glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����
                glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // ����������
                mvStack.pop();
            }
        }
        else {
            int size = 40;
            // ��������ķ�Χ
            float min = 84.0f;
            float max = 96.0f;
            vector<float> randomAngles = generateRandomAngle(size, max, min);
            for (int i = 0; i < size; ++i) {
                mvStack.push(mvStack.top());
                //���
                float scale = 0.008f; // �޸������ֵ���ı�ģ�͵Ĵ�С
                float angle = randomAngles[i];
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 40)), glm::vec3(1.0, 0.0, 0.0)); // ��ת�Ƕ�
                mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 2.3, cos((float)currentTime * roatSpeedBasic) * 2.3));
                mvStack.top() *= glm::rotate(glm::mat4(1.0f),
                    (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
                mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
                glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
                glBindTexture(GL_TEXTURE_2D, rockTexture); // ������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);// �󶨷��߻������
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
                glEnableVertexAttribArray(2);// ���÷�������
                invTrMat = glm::transpose(glm::inverse(mvStack.top()));

                glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
                glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
                int textureLocation10 = glGetUniformLocation(renderingProgram, "rockTexture");
                glUniform1i(textureLocation10, 0); // ������Ԫ0�󶨵�������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // �󶨶���VBO
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
                glEnableVertexAttribArray(0); // ���ö�����������
                glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // ����������VBO
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
                glEnableVertexAttribArray(1); // ��������������������

                glEnable(GL_CULL_FACE); // ���ñ����޳�
                glDepthFunc(GL_LEQUAL);
                //glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����
                glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // ����������
                mvStack.pop();
            }
        }
        // blue material
        float* blueAmb = Utils::blueAmbient();
        float* blueDif = Utils::blueDiffuse();
        float* blueSpe = Utils::blueSpecular();
        float blueShi = Utils::blueShininess();
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, blueAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, blueDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, blueSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, blueShi);
        // ˮ��
        float mercuryOrbitSpeed = 3.0f * roatSpeedBasic;
        float mercuryRotationSpeed = 5.0f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�

        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * mercuryOrbitSpeed) * 1.0, 0.0f, cos((float)currentTime * mercuryOrbitSpeed) * 1.0));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * mercuryRotationSpeed, glm::vec3(1.0, 1.0, 0.0)); // ˮ����ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)); // ��Сˮ��

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, mercuryTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        // ����ˮ��λ��
        glm::vec3 mercuryPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation2 = glGetUniformLocation(renderingProgram, "mercuryTexture");
        glUniform1i(textureLocation2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������

        // �󶨷��߻������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        // ���÷�������ָ��
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        // ���÷�������
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        mvStack.pop(); // �Ӷ�ջ���Ƴ�ˮ�ǵ�����ת

        // gold material
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, goldAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, goldDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, goldSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, goldShi);
        // ����
        float venusOrbitSpeed = 2.0f * roatSpeedBasic;
        float venusRotationSpeed = 4.0f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�

        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * venusOrbitSpeed) * 1.25, 0.0f, cos((float)currentTime * venusOrbitSpeed) * 1.25));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * venusRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // ������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)); // ��С����

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, venusTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // �������λ��
        glm::vec3 venusPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation3 = glGetUniformLocation(renderingProgram, "venusTexture");
        glUniform1i(textureLocation3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������vbo
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        mvStack.pop(); // �Ӷ�ջ���Ƴ����ǵ�����ת

        // gold material
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, goldAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, goldDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, goldSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, goldShi);
        // ����
        float earthOrbitSpeed = 0.5f * roatSpeedBasic;
        float earthRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * earthOrbitSpeed) * 1.5, 0.0f, cos((float)currentTime * earthOrbitSpeed) * 1.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * earthRotationSpeed, glm::vec3(0.5, 1.0, 0.0)); // ������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f)); // ��С����

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, earthTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // �������λ��
        glm::vec3 earthPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation4 = glGetUniformLocation(renderingProgram, "earthTexture");
        glUniform1i(textureLocation4, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        glProgramUniform4fv(renderingProgram, mambLoc, 1, silverAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, silverDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, silverSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, silverShi);
        //����
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 0.5, cos((float)currentTime * roatSpeedBasic) * 0.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f),
            (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));

        //������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f,
            0.5f, 0.5f)); // ������СһЩ
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE,
            glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, moonTexture); // ������

        glEnable(GL_CULL_FACE); // ���ñ����޳�
        glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        //for (int i = 0; i < 2; ++i) {
        //    //�������ɴ�
        //    float scale = 0.75f; // �޸������ֵ���ı�ģ�͵Ĵ�С
        //    mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
        //    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 1.5, cos((float)currentTime * roatSpeedBasic) * 0.5));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), toRadians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 0.0f, 1.0f));

        //    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        //    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��

        //    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]); // �󶨶���VBO
        //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        //    glEnableVertexAttribArray(0); // ���ö�����������

        //    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]); // ����������VBO
        //    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        //    glEnableVertexAttribArray(1); // ��������������������

        //    glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        //    glBindTexture(GL_TEXTURE_2D, shuttleTexture); // ������

        //    glEnable(GL_CULL_FACE); // ���ñ����޳�
        //    glDepthFunc(GL_LEQUAL);

        //    //glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����

        //    glDrawArrays(GL_TRIANGLES, 0, SpaceCraftModel.getNumVertices()); // ����������
        //}

        //mvStack.pop();
        mvStack.pop();
        mvStack.pop(); // �Ӷ�ջ���Ƴ����������ת

        // red material
        float* redAmb = Utils::redAmbient();
        float* redDif = Utils::redDiffuse();
        float* redSpe = Utils::redSpecular();
        float redShi = Utils::redShininess();
        glProgramUniform4fv(renderingProgram, mambLoc, 1, redAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, redDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, redSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, redShi);
        // ����
        float marsOrbitSpeed = 0.8f * roatSpeedBasic;
        float marsRotationSpeed = 1.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-11.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * marsOrbitSpeed) * 2.0, 0.0f, cos((float)currentTime * marsOrbitSpeed) * 2.0));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * marsRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // ������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f)); // ��С����
        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, marsTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // �������λ��
        glm::vec3 marsPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation5 = glGetUniformLocation(renderingProgram, "marsTexture");
        glUniform1i(textureLocation5, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������
        mvStack.pop(); // �Ӷ�ջ���Ƴ����ǵ�����ת

        // green material
        float* greenAmb = Utils::greenAmbient();
        float* greenDif = Utils::greenDiffuse();
        float* greenSpe = Utils::greenSpecular();
        float greenShi = Utils::greenShininess();
        glProgramUniform4fv(renderingProgram, mambLoc, 1, greenAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, greenDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, greenSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, greenShi);
        // ľ��
        float jupiterOrbitSpeed = 2.5f * roatSpeedBasic;
        float jupiterRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-12.5f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * jupiterOrbitSpeed) * 2.5, 0.0f, cos((float)currentTime * jupiterOrbitSpeed) * 2.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * jupiterRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // ľ����ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 0.6f)); // ��Сľ��
        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, jupiterTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // ����ľ�����ĵ�λ��
        glm::vec3 jupiterPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation6 = glGetUniformLocation(renderingProgram, "jupiterTexture");
        glUniform1i(textureLocation6, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ������������������
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������
        mvStack.pop(); // �Ӷ�ջ���Ƴ�ľ�ǵ�����ת

        // ����
        glProgramUniform4fv(renderingProgram, mambLoc, 1, copperAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, copperDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, copperSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, copperShi);
        float saturnOrbitSpeed = 1.5f * roatSpeedBasic;
        float saturnRotationSpeed = 0.5f * roatSpeedBasic;

        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * saturnOrbitSpeed) * 2.8, 0.0f, cos((float)currentTime * saturnOrbitSpeed) * 2.8));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * saturnRotationSpeed, glm::vec3(1.0, 1.0, 0.0)); // ������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 0.6f)); // ��С����

        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, saturnTexture); // ������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// �󶨷��߻������
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// ���÷�������ָ��
        glEnableVertexAttribArray(2);// ���÷�������
        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // �����������ĵ�λ��
        glm::vec3 saturnPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
        int textureLocation7 = glGetUniformLocation(renderingProgram, "saturnTexture");
        glUniform1i(textureLocation7, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        glProgramUniform4fv(renderingProgram, mambLoc, 1, copperAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, copperDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, copperSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, copperShi);

        for (int i = 0; i < 1000; ++i) {
            mvStack.push(mvStack.top());
            //���
            float scale = 0.005f; // �޸������ֵ���ı�ģ�͵Ĵ�С
            //mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
            mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 1000)), glm::vec3(1.0, 0.0, 0.0)); // ��ת�Ƕ�

            mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 0.5, cos((float)currentTime * roatSpeedBasic) * 0.5));

            mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
            mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));

            invTrMat = glm::transpose(glm::inverse(mvStack.top()));

            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
            glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
            glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // �󶨶���VBO
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
            glEnableVertexAttribArray(0); // ���ö�����������

            glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // ����������VBO
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
            glEnableVertexAttribArray(1); // ��������������������

            //glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
            //glBindTexture(GL_TEXTURE_2D, rockTexture); // ������

            glEnable(GL_CULL_FACE); // ���ñ����޳�
            glDepthFunc(GL_LEQUAL);

            //glFrontFace(GL_CCW); // ����˳ʱ��Ϊ����

            glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // ����������
            mvStack.pop();
        }
        mvStack.pop(); // �Ӷ�ջ���Ƴ����ǵ�����ת

        // ������
        float uranusOrbitSpeed = 1.0f * roatSpeedBasic;
        float uranusRotationSpeed = 2.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * uranusOrbitSpeed) * 3.2, 0.0f, cos((float)currentTime * uranusOrbitSpeed) * 3.2));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * uranusRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // ��������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)); // ��С������
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������
        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, uranusTexture); // ������
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������
        mvStack.pop(); // �Ӷ�ջ���Ƴ������ǵ�����ת
        // ������
        float neptuneOrbitSpeed = 0.8f * roatSpeedBasic;
        float neptuneRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // ��ת�Ƕ�
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * neptuneOrbitSpeed) * 3.5, 0.0f, cos((float)currentTime * neptuneOrbitSpeed) * 3.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * neptuneRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // ��������ת
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)); // ��С������
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // ����ͶӰ������ɫ��
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // �󶨶���VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // ���ö�������ָ��
        glEnableVertexAttribArray(0); // ���ö�����������
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // ����������VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // ����������������ָ��
        glEnableVertexAttribArray(1); // ��������������������
        glActiveTexture(GL_TEXTURE0); // ��������Ԫ0
        glBindTexture(GL_TEXTURE_2D, neptuneTexture); // ������
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // ����������

        vector<std::pair<std::string, glm::vec3>> planets = {
        {"Saturn", saturnPosition},
        {"Earth", earthPosition},
        {"Mars", marsPosition},
        {"Venus", venusPosition},
        {"Mercury", mercuryPosition},
        {"Jupiter", jupiterPosition},
        {"Sun", sunPosition},
        };

        for (const auto& planet : planets) {
            float distance = calculateDistance(cameraPos, planet.second);
            if (distance < minDistance && distance < threshold) {
                foundPlanet = true;
                minDistance = distance;
                closestPlanet = planet.first;
            }
        }
        currentStar = foundPlanet ? closestPlanet : "None";
        currentDis = foundPlanet ? minDistance : NULL;

        if (currentDis <= 20.0f && currentDis > 10.0f) {
            showProximityAlert = true;
            score -= 5;
        }
        else if (currentDis <= 10.0f && currentDis > 5.0f) {
            showProximityAlert = true;
            score -= 10;
        }
        else if (currentDis <= 5.0f && currentDis > 0.0f) {
            showProximityAlert = true;
            score -= 20;
        }
        else {
            showProximityAlert = false;
        }
    }
    else {
        ;
    }
    // �Ӷ�ջ���Ƴ��������ǵ�λ�þ������ͼ����
    while (!mvStack.empty()) {
        mvStack.pop();
    }
}

void showDetail() {
    if (currentStar != "None") {
        detailStar = currentStar;

        if (triggeredStars.find(detailStar) == triggeredStars.end()) {
            triggeredStars.insert(detailStar); // ��¼����������
            cout << "Detail for " << detailStar << " triggered!" << endl;
        }

        if (detailStar == "Saturn") {
            detail = "Saturn is the second-largest planet in our solar system, known for its stunning ring system made of ice and rock particles.";
        }
        else if (detailStar == "Earth") {
            detail = "Earth is the only planet known to support life, with a diverse environment of oceans, land, and atmosphere.";
        }
        else if (detailStar == "Mars") {
            detail = "Mars, often called the Red Planet, has a thin atmosphere and is home to the tallest volcano and the deepest canyon in the solar system.";
        }
        else if (detailStar == "Venus") {
            detail = "Venus is a rocky planet with a thick, toxic atmosphere and surface temperatures hot enough to melt lead.";
        }
        else if (detailStar == "Mercury") {
            detail = "Mercury, the smallest planet in the solar system, has extreme temperature fluctuations and a surface covered with craters.";
        }
        else if (detailStar == "Jupiter") {
            detail = "Jupiter is the largest planet in the solar system, famous for its Great Red Spot and a powerful magnetic field.";
        }
        else if (detailStar == "Sun") {
            detail = "The Sun is a star at the center of our solar system, providing the heat and light necessary for life on Earth.";
        }

        allStars.erase(detailStar);

        // ����Ƿ���������� detail �Ѿ�����
        if (triggeredStars.size() == 7) {
            cout << "All stars' details have been triggered!" << endl;
            cout << "ʣ���;�: " << score << endl;
            landed = 1;
            // ��������
            //exit(0);
        }
    }
}

// ���ڴ�С�仯�ص�����
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
    aspect = (float)newWidth / (float)newHeight; // ���¿�߱�
    glViewport(0, 0, newWidth, newHeight); // �����ӿ�
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // ����ͶӰ����
}
// �����ֻص�����
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // ���ݹ��ֵĹ�����������������λ��
    cameraZ -= (float)yoffset * 1.0f; // ����������
    //translateModelAlongZ(-(float)yoffset * 0.1f);

}

int mod = 1;
// ���̻ص�����
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float moveSpeed = 5.0f; // �ƶ��ٶ�
    float rotateSpeed = 3.0f; // ��ת�ٶ�

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // ����ģʽ�л�
        if (key == GLFW_KEY_1) {
            mod = 1;
        }
        else if (key == GLFW_KEY_2) {
            mod = 2;
        }
        else if (key == GLFW_KEY_3) {
            mod = 3;
        }

        if (mod == 1) {
            // ���������ǰ������
            glm::vec3 front;
            front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            front.y = sin(glm::radians(cameraPitch));
            front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            front = glm::normalize(front);
            // ����������ҷ�����
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), front));
            // ģʽ1�����������ģ���ƶ�
            switch (key) {
            case GLFW_KEY_ESCAPE:
                exit(0);
                break;
            case GLFW_KEY_W: // ǰ��
                // ����ǰ�������ƶ���Դ
                cameraX += front.x * moveSpeed;
                cameraY += front.y * moveSpeed;
                cameraZ += front.z * moveSpeed;
                //translateModelAlongZ(-1*moveSpeed);
                break;
            case GLFW_KEY_S: // ����
                // ����ǰ�������ķ������ƶ���Դ
                cameraX -= front.x * moveSpeed;
                cameraY -= front.y * moveSpeed;
                cameraZ -= front.z * moveSpeed;
                //translateModelAlongZ(moveSpeed);
                break;
            case GLFW_KEY_A: // ����
                // �����ҷ������ķ������ƶ���Դ
                cameraX += right.x * moveSpeed;
                cameraY += right.y * moveSpeed;
                cameraZ += right.z * moveSpeed;
                //translateModelAlongX(-1*moveSpeed);
                break;
            case GLFW_KEY_D: // ����
                // �����ҷ������ƶ���Դ
                cameraX -= right.x * moveSpeed;
                cameraY -= right.y * moveSpeed;
                cameraZ -= right.z * moveSpeed;
                //translateModelAlongX(moveSpeed);
                break;
            case GLFW_KEY_Q: // ����
                cameraY += moveSpeed;
                //translateModelAlongY(moveSpeed);
                break;
            case GLFW_KEY_E: // ����
                cameraY -= moveSpeed;
                //translateModelAlongY(-1*moveSpeed);
                break;
            case GLFW_KEY_UP: // ���¿�
                cameraPitch += rotateSpeed;
                break;
            case GLFW_KEY_DOWN: // ���Ͽ�
                cameraPitch -= rotateSpeed;
                break;
            case GLFW_KEY_LEFT: // ���ҿ�
                cameraYaw -= rotateSpeed;
                break;
            case GLFW_KEY_RIGHT: // ����
                cameraYaw += rotateSpeed;
                break;
            case GLFW_KEY_V:
                isThirdPerson = !isThirdPerson;
                if (!isThirdPerson) {
                    cameraY -= 0.25f;
                    cameraZ -= 1.5f;
                }
                else {
                    cameraY += 0.25f;
                    cameraZ += 1.5f;
                }
                break;
            case GLFW_KEY_Z:
                roatSpeedBasic -= 0.05;
                break;
            case GLFW_KEY_X:
                roatSpeedBasic += 0.05;
                break;
            case GLFW_KEY_C:
                crazy = !crazy;
                break;
            case GLFW_KEY_F:
                showDetail();
                break;
            case GLFW_KEY_O:
                hasLight = !hasLight;
                break;
            case GLFW_KEY_I:
                hasSpotLight = !hasSpotLight;
                break;
            case GLFW_KEY_SPACE:
                landed = !landed;
                break;
            }
        }
        else if (mod == 2) {
            // ģʽ2�����ƹ�Դλ��
            switch (key) {
            case GLFW_KEY_W: // ��Դ��ǰ�ƶ�
                lightLoc.z -= moveSpeed;
                break;
            case GLFW_KEY_S: // ��Դ����ƶ�
                lightLoc.z += moveSpeed;
                break;
            case GLFW_KEY_A: // ��Դ�����ƶ�
                lightLoc.x -= moveSpeed;
                break;
            case GLFW_KEY_D: // ��Դ�����ƶ�
                lightLoc.x += moveSpeed;
                break;
            case GLFW_KEY_Q: // ��Դ�����ƶ�
                lightLoc.y += moveSpeed;
                break;
            case GLFW_KEY_E: // ��Դ�����ƶ�
                lightLoc.y -= moveSpeed;
                break;
            case GLFW_KEY_UP:
                // ���� numOfSpotLight��ʹ��ȡģ����ʵ��ѭ��
                numOfSpotLight = (numOfSpotLight + 1) % static_cast<int>(spotLights.size());
                break;
            case GLFW_KEY_DOWN:
                // ���� numOfSpotLight��ʹ��ȡģ����ʵ��ѭ��
                numOfSpotLight = (numOfSpotLight - 1 + static_cast<int>(spotLights.size())) % static_cast<int>(spotLights.size());
                break;
            case GLFW_KEY_LEFT:
                renderingProgram = RenderingPrograms[(--renderingProgramNum + RenderingPrograms.size()) % RenderingPrograms.size()];
                break;
            case GLFW_KEY_RIGHT:
                renderingProgram = RenderingPrograms[(++renderingProgramNum) % RenderingPrograms.size()];
                break;
            }
        }
        else if (mod == 3) {
            switch (key) {
            case GLFW_KEY_UP:
                skyboxTexture = Utils::loadTexture(space[(++spaceNum) % space.size()].c_str());
                break;
            case GLFW_KEY_DOWN:
                skyboxTexture = Utils::loadTexture(space[((--spaceNum) + space.size()) % space.size()].c_str());
                break;

            }
        }
        //glfwPostEmptyEvent();  // ����һ�ο��¼�����ʹ�����ػ�
    }
}

// ������
int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); } // ��ʼ��GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // ����OpenGL�汾
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // ����OpenGL�汾
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window); // ���õ�ǰ������
    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); } // ��ʼ��GLEW
    glfwSwapInterval(1); // ���ô�ֱͬ��

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
    io.Fonts->AddFontFromFileTTF("Semibold.ttf", 48.0f);
    ImGui_ImplOpenGL3_CreateFontsTexture();

    glfwSetWindowSizeCallback(window, window_size_callback); // ���ô��ڴ�С�仯�ص�
    // ע�������ֻص�����
    glfwSetScrollCallback(window, scrollCallback);
    // ע����̻ص�����
    glfwSetKeyCallback(window, keyCallback);

    init(window); // ��ʼ��
    while (!glfwWindowShouldClose(window)) { // ��ѭ��
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ���������
        display(window, glfwGetTime()); // ��ʾ
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
        ImGui::SetNextWindowSize(ImVec2(600, 250));
        ImGui::Begin("Spaceship  Monitoring  System", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);  // ���ô��ڳߴ�������۵�����
        //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text("Durability: %i", score);
        ImGui::Text("The nearest star nearby: %s", currentStar.c_str());
        ImGui::Text("Current distance to %s: %.2f", currentStar.c_str(), currentDis);
        //ImGui::PopStyleColor(); // �ָ�ԭ����������ɫ
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(300, 450)); // ���ô�С
        ImGui::Begin("Remaining tasks:", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        for (const auto& star : allStars) {
            //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(star.c_str());
            //ImGui::PopStyleColor(); // �ָ�ԭ����������ɫ
        }
        ImGui::End();

        ImGui::Begin("Nearest  Planet  Detail", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("%s", detailStar.c_str());
        ImGui::PushTextWrapPos(600);  // ��������ȣ��ı����ڴ˿�ȴ��Զ�����
        ImGui::Text("%s", detail.c_str());
        ImGui::PopTextWrapPos();  // �ָ�Ĭ�ϵ��ı���������
        ImGui::End();

        ImVec2 alertBoxSize = ImVec2(500, 120);
        ImVec2 alertBoxPos = ImVec2(
            (windowWidth - alertBoxSize.x) / 2.0f,
            (windowHeight - alertBoxSize.y) / 2.0f
        );

        if (showProximityAlert) {
            ImGui::SetNextWindowSize(alertBoxSize);
            ImGui::SetNextWindowPos(alertBoxPos);
            ImGui::Begin("Alert!!", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // ��ɫ����
            ImGui::Text("ALERT! Too close to %s !", currentStar.c_str());
            ImGui::PopStyleColor();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // ����������
        glfwPollEvents(); // �����¼�
    }

    glfwDestroyWindow(window); // ���ٴ���
    glfwTerminate(); // ��ֹGLFW
    exit(EXIT_SUCCESS); // �˳�����
}
