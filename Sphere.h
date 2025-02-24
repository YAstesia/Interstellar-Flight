#pragma once
#include <cmath>
#include <vector>
#include <glm\glm.hpp>
class Sphere
{
private:
	int numVertices;
	int numIndices;
	float radius;//�뾶����
	//glm::vec3 position; // λ������
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
	float getRadius() const { return radius; } // ��ӻ�ȡ�뾶�ķ���
	void setRadius(float r) { radius = r; } // ������ð뾶�ķ���
	//const glm::vec3& getPosition() const { return position; } // ��ȡλ�õķ���
	//void setPosition(const glm::vec3& pos) { position = pos; } // ����λ�õķ���
};