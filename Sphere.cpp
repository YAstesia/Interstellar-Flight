#include <cmath>
#include <vector>
#include <iostream>
#include <glm\glm.hpp>
#include "Sphere.h"
using namespace std;

Sphere::Sphere() {
    init(48, 1.0f);
}

// 带有精度、半径和位置参数的构造函数
Sphere::Sphere(int prec, float radius) {
    init(prec, radius);
}


// 将角度转换为弧度
float Sphere::toRadians(float degrees) {
    return (degrees * 2.0f * 3.14159f) / 360.0f;
}
// 初始化球体的方法
void Sphere::init(int prec, float radius) {
    numVertices = (prec + 1) * (prec + 1); // 计算顶点数量
    numIndices = prec * prec * 6; // 计算索引数量
    for (int i = 0; i < numVertices; i++) { vertices.push_back(glm::vec3()); }
    for (int i = 0; i < numVertices; i++) { texCoords.push_back(glm::vec2()); }
    for (int i = 0; i < numVertices; i++) { normals.push_back(glm::vec3()); }
    for (int i = 0; i < numVertices; i++) { tangents.push_back(glm::vec3()); }
    for (int i = 0; i < numIndices; i++) { indices.push_back(0); }

    // 计算三角形顶点
    for (int i = 0; i <= prec; i++) {
        for (int j = 0; j <= prec; j++) {
            float y = (float)cos(toRadians(180.0f - i * 180.0f / prec));
            float x = -(float)cos(toRadians(j * 360.0f / prec)) * (float)abs(cos(asin(y)));
            float z = (float)sin(toRadians(j * 360.0f / (float)(prec))) * (float)abs(cos(asin(y)));
            glm::vec3 vertex = glm::vec3(x, y, z) * radius; // 应用半径
            vertices[i * (prec + 1) + j] = vertex ; // 应用位置偏移
            texCoords[i * (prec + 1) + j] = glm::vec2(((float)j / prec), ((float)i / prec)); // 计算纹理坐标
            normals[i * (prec + 1) + j] = glm::normalize(vertex); // 计算法线方向

            // 计算切线向量
            if (((x == 0) && (y == 1) && (z == 0)) || ((x == 0) && (y == -1) && (z == 0))) {
                tangents[i * (prec + 1) + j] = glm::vec3(0.0f, 0.0f, -1.0f);
            }
            else {
                tangents[i * (prec + 1) + j] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), vertex);
            }
        }
    }

    // 计算三角形索引
    for (int i = 0; i < prec; i++) {
        for (int j = 0; j < prec; j++) {
            indices[6 * (i * prec + j) + 0] = i * (prec + 1) + j;
            indices[6 * (i * prec + j) + 1] = i * (prec + 1) + j + 1;
            indices[6 * (i * prec + j) + 2] = (i + 1) * (prec + 1) + j;
            indices[6 * (i * prec + j) + 3] = i * (prec + 1) + j + 1;
            indices[6 * (i * prec + j) + 4] = (i + 1) * (prec + 1) + j + 1;
            indices[6 * (i * prec + j) + 5] = (i + 1) * (prec + 1) + j;
        }
    }
}

int Sphere::getNumVertices() { return numVertices; }
int Sphere::getNumIndices() { return numIndices; }
std::vector<int> Sphere::getIndices() { return indices; }
std::vector<glm::vec3> Sphere::getVertices() { return vertices; }
std::vector<glm::vec2> Sphere::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Sphere::getNormals() { return normals; }
std::vector<glm::vec3> Sphere::getTangents() { return tangents; }