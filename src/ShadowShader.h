#pragma once
#include "Model.h"
#include "Camera.h"
#include "Matrix.h"
#include "Light.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace glm;

#define MAX_LIGHTS 4

class ShadowShader {
private:
	int programID;
	int vertexShaderID;
	int fragmentShaderID;

	int transMatrixLocation;
	int projectMatrixLocation;
	int viewMatrixLocation;
	int lightPosLocation[MAX_LIGHTS]={0};
	int lightColorLocation[MAX_LIGHTS]={0};
	int attenuationLocation[MAX_LIGHTS]={0};
	int shineDamperLocation;
	int reflectLocation;
	int useFakeLighting;
	int skyColorLocation;
	int numberOfRowsLocation;
	int offsetLocation;
	
    int mvpMatrixLocation;

	unsigned int loadShader(int type, const char* Path) {
		std::string Code;
		std::ifstream ShaderFile;
		try
		{
			ShaderFile.open(Path);
			std::stringstream ShaderStream;
			ShaderStream << ShaderFile.rdbuf();
			ShaderFile.close();
			Code = ShaderStream.str();
		}
		catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
		const char* ShaderCode = Code.c_str();

		unsigned int ShaderID;
		ShaderID = glCreateShader(type);
		glShaderSource(ShaderID, 1, &ShaderCode, NULL);
		glCompileShader(ShaderID);
		int  success;
		char infoLog[512];
		glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ShaderID, 512, NULL, infoLog);
			if(type == GL_VERTEX_SHADER)
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			else
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		return ShaderID;
	}

public:
	ShadowShader(){}
	ShadowShader(const char* vertexPath, const char* fragmentPath) {
		vertexShaderID = loadShader(GL_VERTEX_SHADER, vertexPath);
		fragmentShaderID = loadShader(GL_FRAGMENT_SHADER, fragmentPath);
		programID = glCreateProgram();
		glAttachShader(programID, vertexShaderID);
		glAttachShader(programID, fragmentShaderID);
		BindAttrib();
		glLinkProgram(programID);
		
		int  success;
		char infoLog[512];
		glGetProgramiv(programID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(programID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glValidateProgram(programID);
		getAllUniformLocations();
	}

	void Start() {
		glUseProgram(programID);
	}

	void Stop() {
		glUseProgram(0);
	}

	void Clear() {
		Stop();
		glDetachShader(programID, vertexShaderID);
		glDetachShader(programID, fragmentShaderID);
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		glDeleteProgram(programID);
	}

	void BindAttrib() {
		glBindAttribLocation(programID, 0, "position");
	}

	void getAllUniformLocations(){
        mvpMatrixLocation = getUniformLocation("mvpMatrix");
	}

	void loadNumberOfRows(float numberOfRows) {
		glUniform1f(numberOfRowsLocation, numberOfRows);
	}

	void loadOffset(float x, float y) {
		glUniform2f(offsetLocation, x, y);
	}
	
	void loadSkyColor(float r, float g, float b){
		loadVector(skyColorLocation, vec3(r,g,b));
	}

	void loadFakeLightingVariable(int useFake){
		loadFloat(useFakeLighting, float(useFake));
	}

	unsigned int getUniformLocation(std::string uniformName) {
		return glGetUniformLocation(programID, uniformName.c_str());		
	}

	void loadShine(float damper, float reflectivity){
		loadFloat(shineDamperLocation, damper);
		loadFloat(reflectLocation, reflectivity);
	}

	void loadLights(vector<Light> lights){
		for(int i=0; i<MAX_LIGHTS; i++){
			if(i < lights.size()){
				loadVector(attenuationLocation[i], lights[i].attenuation); //load attenuation
				loadVector(lightPosLocation[i], lights[i].pos);
				loadVector(lightColorLocation[i], lights[i].color);
			}else{
				loadVector(attenuationLocation[i], vec3(1, 0, 0)); //load attenuation
				loadVector(lightPosLocation[i], vec3(0, 0, 0));
				loadVector(lightColorLocation[i], vec3(0, 0, 0));
			}
		}
	}

	void loadTransMatrix(mat4 matrix) {
		loadMatrix(transMatrixLocation, matrix);
	}

	void loadProjectMatrix(mat4 matrix){
		loadMatrix(projectMatrixLocation, matrix);
	}

	void loadViewMatrix(Camera camera){
		mat4 viewMatrix = createViewMatrix(camera);
		loadMatrix(viewMatrixLocation, viewMatrix);
	}

	void loadInt(int location, int value){
		glUniform1i(location, value);
	}

	void loadFloat(int location, float value) {
		glUniform1f(location, value);
	}

	void loadVector(int location, vec3 vector) {
		glUniform3f(location, vector.x, vector.y, vector.z);
	}

	void loadBoolean(int location, bool value){
		float toLoad = 0;
		if(value){
			toLoad = 1;
		}
		glUniform1f(location, toLoad);
	}
	
	void loadMatrix(int location, mat4 matrix) {
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(matrix));
	}

    void loadMvpMatrix(mat4 mvpMatrix){
		loadMatrix(mvpMatrixLocation, mvpMatrix);
	}

	int getID(){
		return programID;
	}
};