#define GLM_ENABLE_EXPERIMENTAL // 启用实验性扩展
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

float cameraX, cameraY, cameraZ, cameraYaw, cameraPitch; // 相机位置
float sphLocX, sphLocY, sphLocZ; // 球体位置
bool isThirdPerson = false; // 控制视角切换
GLuint renderingProgram; // 渲染程序
GLuint skyboxRenderingProgram;
GLuint vao[numVAOs]; // VAO数组
GLuint vbo[numVBOs]; // VBO数组
GLuint earthTexture; // 纹理ID
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
GLuint backgroundVAO, backgroundVBO; // 为背景创建单独的VAO和VBO
ImportedModel SpaceCraftModel("shuttle.obj");
ImportedModel CameraModel("shuttle.obj");
ImportedModel RockModel("rock.obj");
string currentStar;
string detailStar;
float currentDis;
string detail;
bool showProximityAlert = false;
int score = 10000;
set<string> triggeredStars; // 记录已触发的星球
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
// 光源的位置
glm::vec3 lightLoc = glm::vec3(0.0f, 0.0f, 0.0f);
float amt = 0.0f; // 用于控制光源旋转的角度
// 聚光灯属性
glm::vec3 spotLightPos = glm::vec3(0.0f, 2.0f, 0.0f);
glm::vec3 spotLightDirection = glm::vec3(0.0f, 1.0f, 0.0f);
float spotLightCutoff = 0.95f;
float spotLightExponent = 2.0f;

// 初始位置为 (0.0f, 0.0f, 10.5f)
glm::mat4 accumulatedTransform;

bool landed = false;//降落在地球上
float rotAmt = 0.0f; // 旋转角度
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

// 变量声明用于显示
GLuint mvLoc, projLoc, nLoc; // 均匀变量位置
GLint spotLightAmbLoc, spotLightDiffLoc, spotLightSpecLoc, spotLightPosLoc, spotLightDirectionLoc, spotLightCutoffLoc, spotLightExponentLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;// 着色器中光照和材质属性的位置
int width, height; // 窗口宽度和高度
float aspect; // 宽高比
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat; // 投影矩阵、视图矩阵、模型矩阵、模型视图矩阵、逆转置模型视图矩阵和旋转矩阵
glm::vec3 currentLightPos, currentSpotLightPos, currentSpotLightDirection, transformed; // 当前光源位置和变换后的光源位置
float lightPos[3]; // 用于传递光源位置的数组

// 点光源属性
float globalAmbient[4] = { 0.01f, 0.01f, 0.01f, 1.0f }; // 全局环境光
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 光源的环境光
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 光源的漫反射光
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 光源的镜面反射光

// 定义聚光灯属性的结构体
struct SpotLight {
    float ambient[4];      // 环境光颜色
    float diffuse[4];      // 漫反射光颜色
    float specular[4];     // 镜面反射光颜色
    float direction[3];    // 聚光灯光线方向
};
// 初始化聚光灯数组
vector<SpotLight> spotLights = {
    // 红色聚光灯
    {
        { 0.1f, 0.0f, 0.0f, 1.0f }, // 柔和的红色环境光
        { 1.0f, 0.0f, 0.0f, 1.0f }, // 强烈的红色漫反射光
        { 1.0f, 0.0f, 0.0f, 1.0f }, // 高亮的红色镜面反射光
        { 0.0f, -1.0f, 0.0f }       // 向下的方向
    },
    // 蓝色聚光灯
    {
        { 0.05f, 0.05f, 0.2f, 1.0f }, // 柔和的蓝色环境光
        { 0.0f, 0.0f, 1.0f, 1.0f },   // 强烈的蓝色漫反射光
        { 0.0f, 0.0f, 1.0f, 1.0f },   // 高亮的蓝色镜面反射光
        { 0.0f, -1.0f, 0.0f }         // 向下的方向
    },
    // 黄色聚光灯
    {
        { 0.1f, 0.1f, 0.0f, 1.0f }, // 柔和的黄色环境光
        { 1.0f, 1.0f, 0.0f, 1.0f }, // 强烈的黄色漫反射光
        { 1.0f, 1.0f, 0.0f, 1.0f }, // 高亮的黄色镜面反射光
        { 0.0f, -1.0f, 0.0f }       // 向下的方向
    },
    // 白色聚光灯
    {
        { 0.1f, 0.1f, 0.1f, 1.0f }, // 柔和的白色环境光
        { 1.0f, 1.0f, 1.0f, 1.0f }, // 强烈的白色漫反射光
        { 1.0f, 1.0f, 1.0f, 1.0f }, // 高亮的白色镜面反射光
        { 0.0f, -1.0f, 0.0f }       // 向下的方向
    },
    // 霓虹粉色聚光灯
    {
        { 0.1f, 0.0f, 0.1f, 1.0f }, // 柔和的粉色环境光
        { 1.0f, 0.0f, 0.5f, 1.0f }, // 强烈的霓虹粉色漫反射光
        { 1.0f, 0.0f, 0.5f, 1.0f }, // 高亮的霓虹粉色镜面反射光
        { 0.0f, -1.0f, 0.0f }       // 向下的方向
    }
};

int numOfSpotLight = 0;

// 聚光灯光源属性
float dirLightAmbient[4] = { 0.1f, 0.0f, 0.0f, 1.0f }; // 光源的环境光
float dirLightDiffuse[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // 光源的漫反射光
float dirLightSpecular[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // 光源的镜面反射光
float dirLightDirection[3] = { 0.0f,-1.0f,0.0f };//方向

// 材质属性（金色）
float* matAmb = Utils::goldAmbient(); // 材质的环境光
float* matDif = Utils::goldDiffuse(); // 材质的漫反射光
float* matSpe = Utils::goldSpecular(); // 材质的镜面反射光
float matShi = Utils::goldShininess(); // 材质的高光指数

Sphere SphereSun = Sphere(48, 0.25f); // 创建球体对象，细分级别为48
//Sphere SphereEarth= Sphere(48, 0.1f); // 创建球体对象，细分级别为48

float calculateDistance(glm::vec3 cameraPos, glm::vec3 planetPos) {
    if (isThirdPerson) {
        cameraPos.y -= 1.0f;
        cameraPos.z -= 2.8f;
    }
    return glm::length(cameraPos - planetPos); // 返回相机与行星之间的距离
}

bool hasLight = true;
bool hasSpotLight = false;
// 设置光源和材质属性
void installLights(glm::mat4 vMatrix) {
    float zeroVec4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float zeroVec3[3] = { 0.0f, 0.0f, 0.0f };
    if (hasLight)
    {
        // 计算光源在视图空间中的位置
        glm::vec3 transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
        lightPos[0] = transformed.x;
        lightPos[1] = transformed.y;
        lightPos[2] = transformed.z;

        // 获取着色器中变量的位置
        globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
        ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
        diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
        specLoc = glGetUniformLocation(renderingProgram, "light.specular");
        posLoc = glGetUniformLocation(renderingProgram, "light.position");
        mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
        mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
        mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
        mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

        // 设置着色器中的光照和材质属性
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
        // 如果没有启用光照，清除光照属性
        glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, zeroVec4); // 全局环境光
        glProgramUniform4fv(renderingProgram, ambLoc, 1, zeroVec4);       // 环境光
        glProgramUniform4fv(renderingProgram, diffLoc, 1, zeroVec4);      // 漫反射光
        glProgramUniform4fv(renderingProgram, specLoc, 1, zeroVec4);      // 镜面反射光
        glProgramUniform3fv(renderingProgram, posLoc, 1, zeroVec3);       // 光源位置
    }

    if (hasSpotLight)
    {
        // 计算聚光灯在视图空间中的位置和方向
        glm::vec3 spotTransformedPos = glm::vec3(vMatrix * glm::vec4(currentSpotLightPos, 1.0)); // 聚光灯位置
        glm::vec3 spotTransformedDir = glm::vec3(vMatrix * glm::vec4(currentSpotLightDirection, 0.0)); // 聚光灯方向

        // 获取聚光灯属性的 uniform 变量位置
        spotLightAmbLoc = glGetUniformLocation(renderingProgram, "spotLight.ambient");
        spotLightDiffLoc = glGetUniformLocation(renderingProgram, "spotLight.diffuse");
        spotLightSpecLoc = glGetUniformLocation(renderingProgram, "spotLight.specular");
        spotLightPosLoc = glGetUniformLocation(renderingProgram, "spotLight.position");
        spotLightDirectionLoc = glGetUniformLocation(renderingProgram, "spotLight.direction");
        spotLightCutoffLoc = glGetUniformLocation(renderingProgram, "spotLight.cutoff");
        spotLightExponentLoc = glGetUniformLocation(renderingProgram, "spotLight.exponent");

        // 设置聚光灯属性
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
        // 如果没有启用聚光灯，清除聚光灯属性
        glProgramUniform4fv(renderingProgram, spotLightAmbLoc, 1, zeroVec4); // 聚光灯环境光
        glProgramUniform4fv(renderingProgram, spotLightDiffLoc, 1, zeroVec4); // 聚光灯漫反射光
        glProgramUniform4fv(renderingProgram, spotLightSpecLoc, 1, zeroVec4); // 聚光灯镜面反射光
        glProgramUniform3fv(renderingProgram, spotLightPosLoc, 1, zeroVec3);  // 聚光灯位置
        glProgramUniform3fv(renderingProgram, spotLightDirectionLoc, 1, zeroVec3); // 聚光灯方向
        glProgramUniform1f(renderingProgram, spotLightCutoffLoc, 0.0f); // 聚光灯截止角
        glProgramUniform1f(renderingProgram, spotLightExponentLoc, 0.0f); // 聚光灯光衰减指数
    }
}
void setupVerticesForSphere(Sphere& sphere, GLuint vaoIndex) {
    std::vector<int> ind = sphere.getIndices(); // 获取索引
    std::vector<glm::vec3> vert = sphere.getVertices(); // 获取顶点
    std::vector<glm::vec2> tex = sphere.getTexCoords(); // 获取纹理坐标
    std::vector<glm::vec3> norm = sphere.getNormals(); // 获取法线

    std::vector<float> pvalues; // 顶点数据
    std::vector<float> tvalues; // 纹理坐标数据
    std::vector<float> nvalues; // 法线数据

    int numIndices = sphere.getNumIndices(); // 获取索引数量
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

    glBindVertexArray(vao[vaoIndex]); // 绑定VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3]); // 绑定顶点VBO
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW); // 上传顶点数据

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 1]); // 绑定纹理坐标VBO
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW); // 上传纹理坐标数据

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 2]); // 绑定法线VBO
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW); // 上传法线数据
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

    glBindVertexArray(vao[vaoIndex]); // 绑定VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3]); // 绑定顶点VBO
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW); // 上传顶点数据

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 1]); // 绑定纹理坐标VBO
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW); // 上传纹理坐标数据

    glBindBuffer(GL_ARRAY_BUFFER, vbo[vaoIndex * 3 + 2]); // 绑定法线VBO
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW); // 上传法线数据

}

// 设置顶点数据
void setupVertices(void) {
    glGenVertexArrays(numVAOs, vao); // 生成VAO
    glGenBuffers(numVBOs, vbo); // 生成VBO
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
    // 天空盒的立方体纹理坐标
    float cubeTextureCoord[72] = {
    1.00f, 0.6666f, 1.00f, 0.3333f, 0.75f, 0.3333f, // 背面右下角
    0.75f, 0.3333f, 0.75f, 0.6666f, 1.00f, 0.6666f, // 背面左上角
    0.75f, 0.3333f, 0.50f, 0.3333f, 0.75f, 0.6666f, // 右面右下角
    0.50f, 0.3333f, 0.50f, 0.6666f, 0.75f, 0.6666f, // 右面左上角
    0.50f, 0.3333f, 0.25f, 0.3333f, 0.50f, 0.6666f, // 正面右下角
    0.25f, 0.3333f, 0.25f, 0.6666f, 0.50f, 0.6666f, // 正面左上角
    0.25f, 0.3333f, 0.00f, 0.3333f, 0.25f, 0.6666f, // 左面右下角
    0.00f, 0.3333f, 0.00f, 0.6666f, 0.25f, 0.6666f, // 左面左上角
    0.25f, 0.3333f, 0.50f, 0.3333f, 0.50f, 0.00f, // 下面右下角
    0.50f, 0.00f, 0.25f, 0.00f, 0.25f, 0.3333f, // 下面左上角
    0.25f, 1.00f, 0.50f, 1.00f, 0.50f, 0.6666f, // 上面右下角
    0.50f, 0.6666f, 0.25f, 0.6666f, 0.25f, 1.00f // 上面左上角
    };
    // 绑定并填充顶点VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // 绑定并填充纹理坐标VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTextureCoord), cubeTextureCoord, GL_STATIC_DRAW);
    setupVerticesForSphere(SphereSun, 0);
    setupVerticesForModel(SpaceCraftModel, 1);
    setupVerticesForModel(RockModel, 2);
    setupVerticesForModel(CameraModel, 3);

    //setupVerticesForSphere(SphereEarth, 1 );
}


// 初始化函数
void init(GLFWwindow* window) {
    for (int i = 0; i < renderingProgramName.size(); ++i) {
        RenderingPrograms.push_back(Utils::createShaderProgram(renderingProgramName[i][0].c_str(), renderingProgramName[i][1].c_str()));
    }
    renderingProgram = RenderingPrograms[renderingProgramNum];
    skyboxRenderingProgram = Utils::createShaderProgram("./skyboxVertShader.glsl", "./skyboxFragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("./BlinnPhongShaders/vertShader.glsl", "./BlinnPhongShaders/fragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("./GouraudShaders/vertShader.glsl", "./GouraudShaders/fragShader.glsl");
    //renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl"); // 创建并链接着色器程序
    cameraX = 0.0f; cameraY = 0.0f; cameraZ = 300.0f; // 设置相机位置
    accumulatedTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraX, cameraY, cameraZ + 0.25));
    cameraPitch = 0.0f, cameraYaw = -90.0f; // 相机角度
    sphLocX = 0.0f; sphLocY = 0.0f; sphLocZ = -1.0f; // 设置球体位置
    srand(static_cast<unsigned int>(time(0)));
    glfwGetFramebufferSize(window, &width, &height); // 获取窗口尺寸
    aspect = (float)width / (float)height; // 计算宽高比
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 创建透视投影矩阵

    setupVertices(); // 设置顶点数据
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

    glEnable(GL_DEPTH_TEST); // 启用深度测试
    //glDepthFunc(GL_LESS); // 设置深度测试函数
}

stack<glm::mat4>mvStack;

//// 渲染模型的函数
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


// 显示函数
void display(GLFWwindow* window, double currentTime) {
    glClear(GL_DEPTH_BUFFER_BIT); // 清除深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区

    glUseProgram(renderingProgram);

    float threshold = 60.0f;
    float minDistance = std::numeric_limits<float>::max();
    string closestPlanet = "None";
    bool foundPlanet = false;
    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix"); // 获取模型视图矩阵均匀变量位置
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix"); // 获取投影矩阵均匀变量位置
    nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

    // 限制俯仰角，防止万向锁
    if (cameraPitch > 89.0f)
        cameraPitch = 89.0f;
    if (cameraPitch < -89.0f)
        cameraPitch = -89.0f;
    //if (cameraYaw > -1.0f)
    //    cameraYaw = -1.0f;
    //if (cameraYaw < -179.0f)
    //    cameraYaw = -179.0f;

    // 根据 yaw 和 pitch 计算相机的前方向量
    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front = glm::normalize(front);
    // 定义相机的右方和上方向量
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, front));
    up = glm::cross(front, right); // 确保 'up' 是正交于 'front' 和 'right'
    // 创建视图矩阵 (View Matrix) 使用 glm::lookAt
    vMat = glm::lookAt(
        glm::vec3(cameraX, cameraY, cameraZ), // 相机位置
        glm::vec3(cameraX, cameraY, cameraZ) + front, // 相机目标点
        up // "上" 方向
    );
    // 创建视图矩阵，将相机位置移到原点
    //vMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));

     // 切换到天空盒专用的着色器程序
    glUseProgram(skyboxRenderingProgram);

    // 获取天空盒着色器中的统一变量位置
    GLint skyboxMvLoc = glGetUniformLocation(skyboxRenderingProgram, "mv_matrix");
    GLint skyboxProjLoc = glGetUniformLocation(skyboxRenderingProgram, "proj_matrix");

    // 构建MODEL-VIEW矩阵，将天空盒放置在摄像机位置
    glm::mat4 mMat = glm::translate(glm::mat4(1.0f), glm::vec3(cameraX, cameraY, cameraZ));
    glm::mat4 mvMat = vMat * mMat;

    // 传递矩阵给着色器
    glUniformMatrix4fv(skyboxMvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器

    // 设置包含顶点的缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // 设置包含纹理坐标的缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // 激活并绑定天空盒纹理
    glActiveTexture(GL_TEXTURE0);
    if (!landed)
        glBindTexture(GL_TEXTURE_2D, skyboxTexture);
    else
        glBindTexture(GL_TEXTURE_2D, earthSkyboxTexture);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
    glEnableVertexAttribArray(2);// 启用法线属性

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW); // 立方体缠绕顺序是顺时针的，但我们从内部查看，因此使用逆时针缠绕顺序GL_CCW
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 36); // 在没有深度测试的情况下绘制天空盒
    glEnable(GL_DEPTH_TEST);
    // 恢复之前的着色器程序
    glUseProgram(renderingProgram);

    mvStack.push(vMat);

    glm::vec3 cameraPos(cameraX, cameraY, cameraZ);
    glm::vec3 shipPosition;
    // 计算飞船的位置：相机位置 + 前方向量 * 1.5 - 上方向量 * 0.5
    if (isThirdPerson) {
        shipPosition = glm::vec3(cameraX, cameraY, cameraZ) + front * 1.5f - up * 0.5f;
        spotLightPos = shipPosition + 0.25f;
    }
    else {
        shipPosition = glm::vec3(cameraX, cameraY, cameraZ);
        spotLightPos = shipPosition;
    }
    // 更新光源位置
    currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);

    currentSpotLightPos = glm::vec3(spotLightPos.x, spotLightPos.y, spotLightPos.z);

    // 增加旋转角度
    //amt += 0.5f;
    // 创建旋转矩阵，使光源绕 z 轴旋转
    //rMat = glm::rotate(glm::mat4(1.0f), toRadians(amt), glm::vec3(0.0f, 0.0f, 1.0f));
    // 应用旋转矩阵，更新光源位置
    //currentSpotLightPos = glm::vec3(rMat * glm::vec4(currentSpotLightPos, 1.0f));

    // 计算聚光灯光源的方向向量，使其始终指向原点 (0, 0, 0)
    glm::vec3 targetPosition = spotLightPos + front;
    currentSpotLightDirection = glm::normalize(currentSpotLightPos - targetPosition);
    // 设置光源和材质属性
    installLights(vMat);
    //光源
    //绘制球体
    float* whiteAmb = Utils::whiteAmbient();
    float* whiteDif = Utils::whiteDiffuse();
    float* whiteSpe = Utils::whiteSpecular();
    float whiteShi = Utils::whiteShininess();
    glProgramUniform4fv(renderingProgram, mambLoc, 1, whiteAmb);
    glProgramUniform4fv(renderingProgram, mdiffLoc, 1, whiteDif);
    glProgramUniform4fv(renderingProgram, mspecLoc, 1, whiteSpe);
    glProgramUniform1f(renderingProgram, mshiLoc, whiteShi);
    //点光源
    mMat = glm::translate(glm::mat4(1.0f), currentLightPos);
    mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
    mvMat = vMat * mMat;
    invTrMat = glm::transpose(glm::inverse(mvMat));

    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器

    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    glEnable(GL_CULL_FACE); // 启用背面剔除
    glFrontFace(GL_CCW); // 设置顺时针为正面
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
    // 绘制飞船
    if (isThirdPerson) {


        float scale = 0.25f; // 控制模型大小

        // 将当前的矩阵压入堆栈
        mvStack.push(mvStack.top());

        mvStack.top() *= glm::translate(glm::mat4(1.0f), shipPosition);
        mvStack.top() = glm::rotate(mvStack.top(), glm::acos(glm::dot(glm::vec3(0, 0, -1), front)), glm::cross(glm::vec3(0, 0, -1), front));
        mvStack.top() = glm::scale(mvStack.top(), glm::vec3(scale, scale, scale));

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shuttleTexture);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[5]); // 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置法线属性指针
        glEnableVertexAttribArray(2); // 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        // 将更新的模型矩阵传递给着色器
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation11 = glGetUniformLocation(renderingProgram, "shuttleTexture");
        glUniform1i(textureLocation11, 0); // 将纹理单元0绑定到采样器

        // 绑定顶点缓冲区和属性
        glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // 启用深度测试和背面剔除
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        // 绘制模型
        glDrawArrays(GL_TRIANGLES, 0, CameraModel.getNumVertices());

        // 恢复矩阵堆栈
        mvStack.pop();
    }

    if (!landed) {
        mvStack.top() = glm::scale(mvStack.top(), glm::vec3(100, 100, 100));

        // 太阳
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
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)); // 太阳位置
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * sunSpeed, glm::vec3(0.0f, 1.0f, 0.0f)); // 太阳旋转

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, sunTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        glm::vec3 sunPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation = glGetUniformLocation(renderingProgram, "sunTexture");
        glUniform1i(textureLocation, 0); // 将纹理单元0绑定到采样器

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glEnable(GL_CULL_FACE); // 启用背面剔除
        glFrontFace(GL_CCW); // 设置顺时针为正面

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        mvStack.pop(); // 从堆栈中移除太阳的轴旋转


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
                //添加
                float scale = 0.008f; // 修改这里的值来改变模型的大小
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f + (static_cast<float>(i)) * roatSpeedBasic), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 40)), glm::vec3(1.0, 0.0, 0.0)); // 公转角度
                mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 2.3, cos((float)currentTime * roatSpeedBasic) * 2.3));
                mvStack.top() *= glm::rotate(glm::mat4(1.0f),
                    (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
                mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
                glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
                glBindTexture(GL_TEXTURE_2D, rockTexture); // 绑定纹理
                glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);// 绑定法线缓冲对象
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
                glEnableVertexAttribArray(2);// 启用法线属性
                invTrMat = glm::transpose(glm::inverse(mvStack.top()));

                glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
                glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

                int textureLocation9 = glGetUniformLocation(renderingProgram, "rockTexture");
                glUniform1i(textureLocation9, 0); // 将纹理单元0绑定到采样器
                glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // 绑定顶点VBO
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
                glEnableVertexAttribArray(0); // 启用顶点属性数组
                glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // 绑定纹理坐标VBO
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
                glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

                glEnable(GL_CULL_FACE); // 启用背面剔除
                glDepthFunc(GL_LEQUAL);
                glFrontFace(GL_CCW); // 设置顺时针为正面
                glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // 绘制三角形
                mvStack.pop();
            }
        }
        else {
            int size = 40;
            // 定义所需的范围
            float min = 84.0f;
            float max = 96.0f;
            vector<float> randomAngles = generateRandomAngle(size, max, min);
            for (int i = 0; i < size; ++i) {
                mvStack.push(mvStack.top());
                //添加
                float scale = 0.008f; // 修改这里的值来改变模型的大小
                float angle = randomAngles[i];
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
                mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 40)), glm::vec3(1.0, 0.0, 0.0)); // 公转角度
                mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 2.3, cos((float)currentTime * roatSpeedBasic) * 2.3));
                mvStack.top() *= glm::rotate(glm::mat4(1.0f),
                    (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
                mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
                glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
                glBindTexture(GL_TEXTURE_2D, rockTexture); // 绑定纹理
                glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);// 绑定法线缓冲对象
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
                glEnableVertexAttribArray(2);// 启用法线属性
                invTrMat = glm::transpose(glm::inverse(mvStack.top()));

                glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
                glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
                glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
                int textureLocation10 = glGetUniformLocation(renderingProgram, "rockTexture");
                glUniform1i(textureLocation10, 0); // 将纹理单元0绑定到采样器
                glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // 绑定顶点VBO
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
                glEnableVertexAttribArray(0); // 启用顶点属性数组
                glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // 绑定纹理坐标VBO
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
                glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

                glEnable(GL_CULL_FACE); // 启用背面剔除
                glDepthFunc(GL_LEQUAL);
                //glFrontFace(GL_CCW); // 设置顺时针为正面
                glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // 绘制三角形
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
        // 水星
        float mercuryOrbitSpeed = 3.0f * roatSpeedBasic;
        float mercuryRotationSpeed = 5.0f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(20.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度

        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * mercuryOrbitSpeed) * 1.0, 0.0f, cos((float)currentTime * mercuryOrbitSpeed) * 1.0));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * mercuryRotationSpeed, glm::vec3(1.0, 1.0, 0.0)); // 水星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)); // 缩小水星

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, mercuryTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));
        // 计算水星位置
        glm::vec3 mercuryPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation2 = glGetUniformLocation(renderingProgram, "mercuryTexture");
        glUniform1i(textureLocation2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组

        // 绑定法线缓冲对象
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        // 设置法线属性指针
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        // 启用法线属性
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        mvStack.pop(); // 从堆栈中移除水星的轴旋转

        // gold material
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, goldAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, goldDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, goldSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, goldShi);
        // 金星
        float venusOrbitSpeed = 2.0f * roatSpeedBasic;
        float venusRotationSpeed = 4.0f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(15.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度

        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * venusOrbitSpeed) * 1.25, 0.0f, cos((float)currentTime * venusOrbitSpeed) * 1.25));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * venusRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // 金星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f)); // 缩小金星

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, venusTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // 计算金星位置
        glm::vec3 venusPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation3 = glGetUniformLocation(renderingProgram, "venusTexture");
        glUniform1i(textureLocation3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标vbo
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        mvStack.pop(); // 从堆栈中移除金星的轴旋转

        // gold material
        //glProgramUniform4fv(renderingProgram, mambLoc, 1, goldAmb);
        //glProgramUniform4fv(renderingProgram, mdiffLoc, 1, goldDif);
        //glProgramUniform4fv(renderingProgram, mspecLoc, 1, goldSpe);
        //glProgramUniform1f(renderingProgram, mshiLoc, goldShi);
        // 地球
        float earthOrbitSpeed = 0.5f * roatSpeedBasic;
        float earthRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * earthOrbitSpeed) * 1.5, 0.0f, cos((float)currentTime * earthOrbitSpeed) * 1.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * earthRotationSpeed, glm::vec3(0.5, 1.0, 0.0)); // 地球旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f)); // 缩小地球

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, earthTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // 计算地球位置
        glm::vec3 earthPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation4 = glGetUniformLocation(renderingProgram, "earthTexture");
        glUniform1i(textureLocation4, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        glProgramUniform4fv(renderingProgram, mambLoc, 1, silverAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, silverDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, silverSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, silverShi);
        //月球
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 0.5, cos((float)currentTime * roatSpeedBasic) * 0.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f),
            (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));

        //月球旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f,
            0.5f, 0.5f)); // 让月球小一些
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE,
            glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, moonTexture); // 绑定纹理

        glEnable(GL_CULL_FACE); // 启用背面剔除
        glFrontFace(GL_CCW); // 设置顺时针为正面

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        //for (int i = 0; i < 2; ++i) {
        //    //添加宇宙飞船
        //    float scale = 0.75f; // 修改这里的值来改变模型的大小
        //    mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
        //    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 1.5, cos((float)currentTime * roatSpeedBasic) * 0.5));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), toRadians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //    mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0f, 0.0f, 1.0f));

        //    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        //    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器

        //    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]); // 绑定顶点VBO
        //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        //    glEnableVertexAttribArray(0); // 启用顶点属性数组

        //    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]); // 绑定纹理坐标VBO
        //    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        //    glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        //    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        //    glBindTexture(GL_TEXTURE_2D, shuttleTexture); // 绑定纹理

        //    glEnable(GL_CULL_FACE); // 启用背面剔除
        //    glDepthFunc(GL_LEQUAL);

        //    //glFrontFace(GL_CCW); // 设置顺时针为正面

        //    glDrawArrays(GL_TRIANGLES, 0, SpaceCraftModel.getNumVertices()); // 绘制三角形
        //}

        //mvStack.pop();
        mvStack.pop();
        mvStack.pop(); // 从堆栈中移除地球的轴旋转

        // red material
        float* redAmb = Utils::redAmbient();
        float* redDif = Utils::redDiffuse();
        float* redSpe = Utils::redSpecular();
        float redShi = Utils::redShininess();
        glProgramUniform4fv(renderingProgram, mambLoc, 1, redAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, redDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, redSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, redShi);
        // 火星
        float marsOrbitSpeed = 0.8f * roatSpeedBasic;
        float marsRotationSpeed = 1.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-11.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * marsOrbitSpeed) * 2.0, 0.0f, cos((float)currentTime * marsOrbitSpeed) * 2.0));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * marsRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // 火星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f)); // 缩小火星
        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, marsTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // 计算火星位置
        glm::vec3 marsPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation5 = glGetUniformLocation(renderingProgram, "marsTexture");
        glUniform1i(textureLocation5, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形
        mvStack.pop(); // 从堆栈中移除火星的轴旋转

        // green material
        float* greenAmb = Utils::greenAmbient();
        float* greenDif = Utils::greenDiffuse();
        float* greenSpe = Utils::greenSpecular();
        float greenShi = Utils::greenShininess();
        glProgramUniform4fv(renderingProgram, mambLoc, 1, greenAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, greenDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, greenSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, greenShi);
        // 木星
        float jupiterOrbitSpeed = 2.5f * roatSpeedBasic;
        float jupiterRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-12.5f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * jupiterOrbitSpeed) * 2.5, 0.0f, cos((float)currentTime * jupiterOrbitSpeed) * 2.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * jupiterRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // 木星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 0.6f)); // 缩小木星
        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, jupiterTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性

        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // 计算木星中心点位置
        glm::vec3 jupiterPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        int textureLocation6 = glGetUniformLocation(renderingProgram, "jupiterTexture");
        glUniform1i(textureLocation6, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形
        mvStack.pop(); // 从堆栈中移除木星的轴旋转

        // 土星
        glProgramUniform4fv(renderingProgram, mambLoc, 1, copperAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, copperDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, copperSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, copperShi);
        float saturnOrbitSpeed = 1.5f * roatSpeedBasic;
        float saturnRotationSpeed = 0.5f * roatSpeedBasic;

        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * saturnOrbitSpeed) * 2.8, 0.0f, cos((float)currentTime * saturnOrbitSpeed) * 2.8));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * saturnRotationSpeed, glm::vec3(1.0, 1.0, 0.0)); // 土星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 0.6f)); // 缩小土星

        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, saturnTexture); // 绑定纹理
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);// 绑定法线缓冲对象
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);// 设置法线属性指针
        glEnableVertexAttribArray(2);// 启用法线属性
        invTrMat = glm::transpose(glm::inverse(mvStack.top()));

        // 计算土星中心点位置
        glm::vec3 saturnPosition = glm::vec3(
            mvStack.top()[3][0],
            mvStack.top()[3][1],
            mvStack.top()[3][2]
        );

        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
        int textureLocation7 = glGetUniformLocation(renderingProgram, "saturnTexture");
        glUniform1i(textureLocation7, 1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

        glProgramUniform4fv(renderingProgram, mambLoc, 1, copperAmb);
        glProgramUniform4fv(renderingProgram, mdiffLoc, 1, copperDif);
        glProgramUniform4fv(renderingProgram, mspecLoc, 1, copperSpe);
        glProgramUniform1f(renderingProgram, mshiLoc, copperShi);

        for (int i = 0; i < 1000; ++i) {
            mvStack.push(mvStack.top());
            //添加
            float scale = 0.005f; // 修改这里的值来改变模型的大小
            //mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
            mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(i * 360 / 1000)), glm::vec3(1.0, 0.0, 0.0)); // 公转角度

            mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, sin((float)currentTime * roatSpeedBasic) * 0.5, cos((float)currentTime * roatSpeedBasic) * 0.5));

            mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * roatSpeedBasic, glm::vec3(0.0, 0.0, 1.0));
            mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));

            invTrMat = glm::transpose(glm::inverse(mvStack.top()));

            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
            glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
            glBindBuffer(GL_ARRAY_BUFFER, vbo[6]); // 绑定顶点VBO
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
            glEnableVertexAttribArray(0); // 启用顶点属性数组

            glBindBuffer(GL_ARRAY_BUFFER, vbo[7]); // 绑定纹理坐标VBO
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
            glEnableVertexAttribArray(1); // 启用纹理坐标属性数组

            //glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
            //glBindTexture(GL_TEXTURE_2D, rockTexture); // 绑定纹理

            glEnable(GL_CULL_FACE); // 启用背面剔除
            glDepthFunc(GL_LEQUAL);

            //glFrontFace(GL_CCW); // 设置顺时针为正面

            glDrawArrays(GL_TRIANGLES, 0, RockModel.getNumVertices()); // 绘制三角形
            mvStack.pop();
        }
        mvStack.pop(); // 从堆栈中移除土星的轴旋转

        // 天王星
        float uranusOrbitSpeed = 1.0f * roatSpeedBasic;
        float uranusRotationSpeed = 2.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * uranusOrbitSpeed) * 3.2, 0.0f, cos((float)currentTime * uranusOrbitSpeed) * 3.2));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * uranusRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // 天王星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)); // 缩小天王星
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组
        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, uranusTexture); // 绑定纹理
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形
        mvStack.pop(); // 从堆栈中移除天王星的轴旋转
        // 海王星
        float neptuneOrbitSpeed = 0.8f * roatSpeedBasic;
        float neptuneRotationSpeed = 0.5f * roatSpeedBasic;
        mvStack.push(mvStack.top());
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(12.0f), glm::vec3(0.0, 0.0, 1.0)); // 公转角度
        mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime * neptuneOrbitSpeed) * 3.5, 0.0f, cos((float)currentTime * neptuneOrbitSpeed) * 3.5));
        mvStack.top() *= glm::rotate(glm::mat4(1.0f), (float)currentTime * neptuneRotationSpeed, glm::vec3(0.0, 1.0, 0.0)); // 海王星旋转
        mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)); // 缩小海王星
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat)); // 传递投影矩阵到着色器
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // 绑定顶点VBO
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 设置顶点属性指针
        glEnableVertexAttribArray(0); // 启用顶点属性数组
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // 绑定纹理坐标VBO
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // 设置纹理坐标属性指针
        glEnableVertexAttribArray(1); // 启用纹理坐标属性数组
        glActiveTexture(GL_TEXTURE0); // 激活纹理单元0
        glBindTexture(GL_TEXTURE_2D, neptuneTexture); // 绑定纹理
        glDrawArrays(GL_TRIANGLES, 0, SphereSun.getNumIndices()); // 绘制三角形

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
    // 从堆栈中移除所有行星的位置矩阵和视图矩阵
    while (!mvStack.empty()) {
        mvStack.pop();
    }
}

void showDetail() {
    if (currentStar != "None") {
        detailStar = currentStar;

        if (triggeredStars.find(detailStar) == triggeredStars.end()) {
            triggeredStars.insert(detailStar); // 记录触发的星球
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

        // 检测是否所有星球的 detail 已经触发
        if (triggeredStars.size() == 7) {
            cout << "All stars' details have been triggered!" << endl;
            cout << "剩余耐久: " << score << endl;
            landed = 1;
            // 结束程序
            //exit(0);
        }
    }
}

// 窗口大小变化回调函数
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
    aspect = (float)newWidth / (float)newHeight; // 更新宽高比
    glViewport(0, 0, newWidth, newHeight); // 更新视口
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 更新投影矩阵
}
// 鼠标滚轮回调函数
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // 根据滚轮的滚动方向调整摄像机的位置
    cameraZ -= (float)yoffset * 1.0f; // 调整灵敏度
    //translateModelAlongZ(-(float)yoffset * 0.1f);

}

int mod = 1;
// 键盘回调函数
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float moveSpeed = 5.0f; // 移动速度
    float rotateSpeed = 3.0f; // 旋转速度

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // 处理模式切换
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
            // 计算相机的前方向量
            glm::vec3 front;
            front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            front.y = sin(glm::radians(cameraPitch));
            front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            front = glm::normalize(front);
            // 计算相机的右方向量
            glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), front));
            // 模式1：控制相机和模型移动
            switch (key) {
            case GLFW_KEY_ESCAPE:
                exit(0);
                break;
            case GLFW_KEY_W: // 前进
                // 沿着前方向量移动光源
                cameraX += front.x * moveSpeed;
                cameraY += front.y * moveSpeed;
                cameraZ += front.z * moveSpeed;
                //translateModelAlongZ(-1*moveSpeed);
                break;
            case GLFW_KEY_S: // 后退
                // 沿着前方向量的反方向移动光源
                cameraX -= front.x * moveSpeed;
                cameraY -= front.y * moveSpeed;
                cameraZ -= front.z * moveSpeed;
                //translateModelAlongZ(moveSpeed);
                break;
            case GLFW_KEY_A: // 左移
                // 沿着右方向量的反方向移动光源
                cameraX += right.x * moveSpeed;
                cameraY += right.y * moveSpeed;
                cameraZ += right.z * moveSpeed;
                //translateModelAlongX(-1*moveSpeed);
                break;
            case GLFW_KEY_D: // 右移
                // 沿着右方向量移动光源
                cameraX -= right.x * moveSpeed;
                cameraY -= right.y * moveSpeed;
                cameraZ -= right.z * moveSpeed;
                //translateModelAlongX(moveSpeed);
                break;
            case GLFW_KEY_Q: // 上移
                cameraY += moveSpeed;
                //translateModelAlongY(moveSpeed);
                break;
            case GLFW_KEY_E: // 下移
                cameraY -= moveSpeed;
                //translateModelAlongY(-1*moveSpeed);
                break;
            case GLFW_KEY_UP: // 向下看
                cameraPitch += rotateSpeed;
                break;
            case GLFW_KEY_DOWN: // 向上看
                cameraPitch -= rotateSpeed;
                break;
            case GLFW_KEY_LEFT: // 向右看
                cameraYaw -= rotateSpeed;
                break;
            case GLFW_KEY_RIGHT: // 向左看
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
            // 模式2：控制光源位置
            switch (key) {
            case GLFW_KEY_W: // 光源向前移动
                lightLoc.z -= moveSpeed;
                break;
            case GLFW_KEY_S: // 光源向后移动
                lightLoc.z += moveSpeed;
                break;
            case GLFW_KEY_A: // 光源向左移动
                lightLoc.x -= moveSpeed;
                break;
            case GLFW_KEY_D: // 光源向右移动
                lightLoc.x += moveSpeed;
                break;
            case GLFW_KEY_Q: // 光源向上移动
                lightLoc.y += moveSpeed;
                break;
            case GLFW_KEY_E: // 光源向下移动
                lightLoc.y -= moveSpeed;
                break;
            case GLFW_KEY_UP:
                // 增加 numOfSpotLight，使用取模运算实现循环
                numOfSpotLight = (numOfSpotLight + 1) % static_cast<int>(spotLights.size());
                break;
            case GLFW_KEY_DOWN:
                // 减少 numOfSpotLight，使用取模运算实现循环
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
        //glfwPostEmptyEvent();  // 触发一次空事件，促使窗口重绘
    }
}

// 主函数
int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); } // 初始化GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 设置OpenGL版本
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window); // 设置当前上下文
    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); } // 初始化GLEW
    glfwSwapInterval(1); // 启用垂直同步

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
    io.Fonts->AddFontFromFileTTF("Semibold.ttf", 48.0f);
    ImGui_ImplOpenGL3_CreateFontsTexture();

    glfwSetWindowSizeCallback(window, window_size_callback); // 设置窗口大小变化回调
    // 注册鼠标滚轮回调函数
    glfwSetScrollCallback(window, scrollCallback);
    // 注册键盘回调函数
    glfwSetKeyCallback(window, keyCallback);

    init(window); // 初始化
    while (!glfwWindowShouldClose(window)) { // 主循环
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除缓冲区
        display(window, glfwGetTime()); // 显示
        int windowWidth, windowHeight;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
        ImGui::SetNextWindowSize(ImVec2(600, 250));
        ImGui::Begin("Spaceship  Monitoring  System", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);  // 禁用窗口尺寸调整和折叠功能
        //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text("Durability: %i", score);
        ImGui::Text("The nearest star nearby: %s", currentStar.c_str());
        ImGui::Text("Current distance to %s: %.2f", currentStar.c_str(), currentDis);
        //ImGui::PopStyleColor(); // 恢复原来的字体颜色
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(300, 450)); // 设置大小
        ImGui::Begin("Remaining tasks:", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        for (const auto& star : allStars) {
            //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text(star.c_str());
            //ImGui::PopStyleColor(); // 恢复原来的字体颜色
        }
        ImGui::End();

        ImGui::Begin("Nearest  Planet  Detail", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("%s", detailStar.c_str());
        ImGui::PushTextWrapPos(600);  // 设置最大宽度，文本将在此宽度处自动换行
        ImGui::Text("%s", detail.c_str());
        ImGui::PopTextWrapPos();  // 恢复默认的文本换行设置
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
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // 红色文字
            ImGui::Text("ALERT! Too close to %s !", currentStar.c_str());
            ImGui::PopStyleColor();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // 交换缓冲区
        glfwPollEvents(); // 处理事件
    }

    glfwDestroyWindow(window); // 销毁窗口
    glfwTerminate(); // 终止GLFW
    exit(EXIT_SUCCESS); // 退出程序
}
