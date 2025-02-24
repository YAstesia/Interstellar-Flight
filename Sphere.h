#pragma once
#include <cmath>
#include <vector>
#include <glm\glm.hpp>
class Sphere
{
private:
	int numVertices;
	int numIndices;
	float radius;//半径属性
	//glm::vec3 position; // 位置属性
	std::vector<int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	void init(int,float radius);
	float toRadians(float degrees);

public:
	Sphere();
	Sphere(int prec,float radius=1.0f);
	int getNumVertices();
	int getNumIndices();
	std::vector<int> getIndices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
	std::vector<glm::vec3> getTangents();
	float getRadius() const { return radius; } // 添加获取半径的方法
	void setRadius(float r) { radius = r; } // 添加设置半径的方法
	//const glm::vec3& getPosition() const { return position; } // 获取位置的方法
	//void setPosition(const glm::vec3& pos) { position = pos; } // 设置位置的方法
};