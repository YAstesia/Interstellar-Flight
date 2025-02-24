#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL2\soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp> // glm::value_ptr
#include <glm\gtc\matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Utils.h"
using namespace std;

Utils::Utils() {}

string Utils::readShaderFile(const char *filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

bool Utils::checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void Utils::printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char *)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

GLuint Utils::prepareShader(int shaderTYPE, const char *shaderPath)
{	GLint shaderCompiled;
	string shaderStr = readShaderFile(shaderPath);
	const char *shaderSrc = shaderStr.c_str();
	GLuint shaderRef = glCreateShader(shaderTYPE);
	glShaderSource(shaderRef, 1, &shaderSrc, NULL);
	glCompileShader(shaderRef);
	checkOpenGLError();
	glGetShaderiv(shaderRef, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != 1)
	{	if (shaderTYPE == 35633) cout << "Vertex ";
		if (shaderTYPE == 36488) cout << "Tess Control ";
		if (shaderTYPE == 36487) cout << "Tess Eval ";
		if (shaderTYPE == 36313) cout << "Geometry ";
		if (shaderTYPE == 35632) cout << "Fragment ";
		cout << "shader compilation error." << endl;
		printShaderLog(shaderRef);
	}
	return shaderRef;
}

void Utils::printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char *)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

int Utils::finalizeShaderProgram(GLuint sprogram)
{	GLint linked;
	glLinkProgram(sprogram);
	checkOpenGLError();
	glGetProgramiv(sprogram, GL_LINK_STATUS, &linked);
	if (linked != 1)
	{	cout << "linking failed" << endl;
		printProgramLog(sprogram);
	}
	return sprogram;
}

GLuint Utils::createShaderProgram(const char *vp, const char *fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vfprogram = glCreateProgram();
	glAttachShader(vfprogram, vShader);
	glAttachShader(vfprogram, fShader);
	finalizeShaderProgram(vfprogram);
	return vfprogram;
}

GLuint Utils::createShaderProgram(const char *vp, const char *gp, const char *fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint gShader = prepareShader(GL_GEOMETRY_SHADER, gp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vgfprogram = glCreateProgram();
	glAttachShader(vgfprogram, vShader);
	glAttachShader(vgfprogram, gShader);
	glAttachShader(vgfprogram, fShader);
	finalizeShaderProgram(vgfprogram);
	return vgfprogram;
}

GLuint Utils::createShaderProgram(const char *vp, const char *tCS, const char* tES, const char *fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint tcShader = prepareShader(GL_TESS_CONTROL_SHADER, tCS);
	GLuint teShader = prepareShader(GL_TESS_EVALUATION_SHADER, tES);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vtfprogram = glCreateProgram();
	glAttachShader(vtfprogram, vShader);
	glAttachShader(vtfprogram, tcShader);
	glAttachShader(vtfprogram, teShader);
	glAttachShader(vtfprogram, fShader);
	finalizeShaderProgram(vtfprogram);
	return vtfprogram;
}

GLuint Utils::createShaderProgram(const char *vp, const char *tCS, const char* tES, char *gp, const char *fp) {
	GLuint vShader = prepareShader(GL_VERTEX_SHADER, vp);
	GLuint tcShader = prepareShader(GL_TESS_CONTROL_SHADER, tCS);
	GLuint teShader = prepareShader(GL_TESS_EVALUATION_SHADER, tES);
	GLuint gShader = prepareShader(GL_GEOMETRY_SHADER, gp);
	GLuint fShader = prepareShader(GL_FRAGMENT_SHADER, fp);
	GLuint vtgfprogram = glCreateProgram();
	glAttachShader(vtgfprogram, vShader);
	glAttachShader(vtgfprogram, tcShader);
	glAttachShader(vtgfprogram, teShader);
	glAttachShader(vtgfprogram, gShader);
	glAttachShader(vtgfprogram, fShader);
	finalizeShaderProgram(vtgfprogram);
	return vtgfprogram;
}

GLuint Utils::loadCubeMap(const char *mapDir) {
	GLuint textureRef;
	string xp = mapDir; xp = xp + "/xp.jpg";
	string xn = mapDir; xn = xn + "/xn.jpg";
	string yp = mapDir; yp = yp + "/yp.jpg";
	string yn = mapDir; yn = yn + "/yn.jpg";
	string zp = mapDir; zp = zp + "/zp.jpg";
	string zn = mapDir; zn = zn + "/zn.jpg";
	textureRef = SOIL_load_OGL_cubemap(xp.c_str(), xn.c_str(), yp.c_str(), yn.c_str(), zp.c_str(), zn.c_str(),
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (textureRef == 0) cout << "didnt find cube map image file" << endl;
	//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureRef);
	// reduce seams
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureRef;
}

GLuint Utils::loadTexture(const char *texImagePath)
{	GLuint textureRef;
	textureRef = SOIL_load_OGL_texture(texImagePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	if (textureRef == 0) cout << "didnt find texture file " << texImagePath << endl;
	// ----- mipmap/anisotropic section
	glBindTexture(GL_TEXTURE_2D, textureRef);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
		GLfloat anisoset = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoset);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoset);
	}
	// ----- end of mipmap/anisotropic section
	return textureRef;
}

// GOLD material - ambient, diffuse, specular, and shininess
float* Utils::goldAmbient() { static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 }; return (float*)a; }
float* Utils::goldDiffuse() { static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 }; return (float*)a; }
float* Utils::goldSpecular() { static float a[4] = { 0.6283f, 0.5559f, 0.3661f, 1 }; return (float*)a; }
float Utils::goldShininess() { return 51.2f; }

// SILVER material - ambient, diffuse, specular, and shininess
float* Utils::silverAmbient() { static float a[4] = { 0.1923f, 0.1923f, 0.1923f, 1 }; return (float*)a; }
float* Utils::silverDiffuse() { static float a[4] = { 0.5075f, 0.5075f, 0.5075f, 1 }; return (float*)a; }
float* Utils::silverSpecular() { static float a[4] = { 0.5083f, 0.5083f, 0.5083f, 1 }; return (float*)a; }
float Utils::silverShininess() { return 51.2f; }

// BRONZE material - ambient, diffuse, specular, and shininess
float* Utils::bronzeAmbient() { static float a[4] = { 0.2125f, 0.1275f, 0.0540f, 1 }; return (float*)a; }
float* Utils::bronzeDiffuse() { static float a[4] = { 0.7140f, 0.4284f, 0.1814f, 1 }; return (float*)a; }
float* Utils::bronzeSpecular() { static float a[4] = { 0.3936f, 0.2719f, 0.1667f, 1 }; return (float*)a; }
float Utils::bronzeShininess() { return 25.6f; }


// WHITE material - ambient, diffuse, specular, and shininess
 float* Utils::whiteAmbient() {
	static float a[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	return a;
}
float* Utils::whiteDiffuse() {
	static float a[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	return a;
}
 float* Utils::whiteSpecular() {
	static float a[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	return a;
}
float Utils::whiteShininess() {
	return 50.0f; // 高光强度，可以根据需要调整
}

// COPPER material - ambient, diffuse, specular, and shininess
float* Utils::copperAmbient() {
	static float a[4] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
	return (float*)a;
}

float* Utils::copperDiffuse() {
	static float a[4] = { 0.7038f, 0.27048f, 0.07038f, 1.0f };
	return (float*)a;
}

float* Utils::copperSpecular() {
	static float a[4] = { 0.256777f, 0.137622f, 0.086014f, 1.0f };
	return (float*)a;
}

float Utils::copperShininess() {
	return 12.8f;
}

float* Utils::blueAmbient() {
	static float a[4] = { 0.0f, 0.0f, 0.2f, 1.0f };
	return (float*)a;
}
float* Utils::blueDiffuse() {
	static float a[4] = { 0.0f, 0.0f, 0.8f, 1.0f };
	return (float*)a;
}
float* Utils::blueSpecular() {
	static float a[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
	return (float*)a;
}
float Utils::blueShininess() {
	return 10.0f;
}

float* Utils::redAmbient() {
	static float a[4] = { 0.2f, 0.0f, 0.0f, 1.0f };
	return (float*)a;
}
float* Utils::redDiffuse() {
	static float a[4] = { 0.8f, 0.1f, 0.1f, 1.0f };
	return (float*)a;
}
float* Utils::redSpecular() {
	static float a[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	return (float*)a;
}
float Utils::redShininess() {
	return 10.0f;
}

float* Utils::greenAmbient() {
	static float a[4] = { 0.0f, 0.2f, 0.0f, 1.0f };
	return (float*)a;
}
float* Utils::greenDiffuse() {
	static float a[4] = { 0.1f, 0.8f, 0.1f, 1.0f };
	return (float*)a;
}
float* Utils::greenSpecular() {
	static float a[4] = { 0.5f, 1.0f, 0.5f, 1.0f };
	return (float*)a;
}
float Utils::greenShininess() {
	return 10.0f;
}
