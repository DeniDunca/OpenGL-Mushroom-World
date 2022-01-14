#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 beeModel;
glm::mat4 beaverModel;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 spotlightDir;
glm::vec3 spotlightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint spotlightColorLoc;

// mouse
bool firstMouse = true;
float lastX;
float lastY;
float pitch = 0.0f;

glm::vec3 cameraPosition;

// camera
gps::Camera myCamera(
    glm::vec3(5.0f, 5.0f, 5.0f),
    glm::vec3(5.0f, 5.0f, 5.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];
GLfloat lightAngle;

// models
gps::Model3D world;
gps::Model3D beaver;
gps::Model3D bee;
GLfloat angle = 0.5f;

// shaders
gps::Shader myBasicShader;


// animations
bool flyBee = false;
GLfloat beeAngles = 0.1f;
bool walkbeaver = true;
GLfloat beaverAngles = 0.01f;

// fog
float fogDensity = 0.0f;
GLuint fogDensityLoc;

//point lights
GLint point1Pos, point2Pos;
GLint point1Col, point2Col;
GLint point1Con, point2Con;
GLint point1Lin, point2Lin;
GLint point1Qua, point2Qua;

glm::vec3 pointLightPositions[] =
{
    glm::vec3(-1.11224f, 0.097029f, 1.7791f),
    glm::vec3(-2.1942f, 0.37991f, -6.8907f),
};
glm::vec3 pointLightColors[] =
{
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 0.0f)
};
GLfloat pointLightConsts[] =
{
    1.0f,
    1.0f
};
GLfloat pointLightLinears[] =
{
    0.09f,
    0.09f
};
GLfloat pointLightQuads[] =
{
    0.032f,
    0.032f
};


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    //wireframe
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //solid
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    //polygonal
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    //smooth
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    }
    //fog down
    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        fogDensity -= 0.01f;
        glUniform1f(fogDensityLoc, fogDensity);
    }
    //fog up
    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        fogDensity += 0.01f;
        glUniform1f(fogDensityLoc, fogDensity);
    }

    //blue
    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        lightColor = glm::vec3(0.255f, 0.227f, 0.115f);
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        point1Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[2]");
        glUniform3fv(point1Col, 1, glm::value_ptr(pointLightColors[2]));
        point2Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[2]");
        glUniform3fv(point2Col, 1, glm::value_ptr(pointLightColors[2]));

    }

    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        lightColor = glm::vec3(0.0f, 0.0f, 0.0f);
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        point1Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[0]");
        glUniform3fv(point1Col, 1, glm::value_ptr(pointLightColors[0]));
        point2Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[0]");
        glUniform3fv(point2Col, 1, glm::value_ptr(pointLightColors[0]));
    }
    
    //bees fly
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        flyBee = !flyBee;
    }
    //beaver front
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        walkbeaver = true;
    }

    //beaver back
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        walkbeaver = false;
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    angle += xoffset;
    pitch += yoffset;

    if (pitch > 80.0f)
        pitch = 80.0f;
    if (pitch < -80.0f)
        pitch = -80.0f;

    myCamera.rotate(pitch, angle);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for map
        myCamera.rotate(0.0f, angle);

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // update normal matrix for map
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for map
        myCamera.rotate(0.0f, angle);

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // update normal matrix for map
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_SPACE]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for map
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for map
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (flyBee) {
        beeAngles -= 0.5f;
        beeModel = glm::rotate(glm::mat4(1.0f), glm::radians(beeAngles), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (walkbeaver) {
        beaverAngles += 0.5f;
        glm::mat4 trans = glm::translate(glm::mat4(0.5f), glm::vec3(7.0f, 0.0f, 7.0f));
        glm::mat4 rot = glm::rotate(glm::mat4(0.5f), glm::radians(beaverAngles), glm::vec3(0.0f, 0.5f, 0.0f));
        
        beaverModel = trans * rot;
    }
    if (!walkbeaver) {
        beaverAngles -= 0.5f;
        glm::mat4 trans = glm::translate(glm::mat4(0.5f), glm::vec3(7.0f, 0.0f, 7.0f));
        glm::mat4 rot = glm::rotate(glm::mat4(0.5f), glm::radians(beaverAngles), glm::vec3(0.0f, 0.5f, 0.0f));

        beaverModel = trans * rot;
    }
   
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "Mushroom World");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    world.LoadModel("models/world/world.obj");
    bee.LoadModel("models/albinute/albinute.obj");
    beaver.LoadModel("models/harciog/harciog.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for world
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    // create model matrix for bee
    beeModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    // create model matrix for beaver
    beaverModel = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix 
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 50.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 2.0f, 0.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
   
    
    lightColor = glm::vec3(0.255f, 0.227f, 0.115f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    point1Pos = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPositions[0]");
    glUniform3fv(point1Pos, 1, glm::value_ptr(pointLightPositions[0]));
    point1Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[0]");
    glUniform3fv(point1Col, 1, glm::value_ptr(pointLightColors[0]));
    point1Con = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightConsts[0]");
    glUniform1f(point1Con, pointLightConsts[0]);
    point1Lin = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightLinears[0]");
    glUniform1f(point1Lin, pointLightLinears[0]);
    point1Qua = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightQuads[0]");
    glUniform1f(point1Qua, pointLightQuads[0]);

    point2Pos = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightPositions[1]");
    glUniform3fv(point2Pos, 1, glm::value_ptr(pointLightPositions[1]));
    point2Col = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightColors[1]");
    glUniform3fv(point2Col, 1, glm::value_ptr(pointLightColors[1]));
    point2Con = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightConsts[1]");
    glUniform1f(point2Con, pointLightConsts[1]);
    point2Lin = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightLinears[1]");
    glUniform1f(point2Lin, pointLightLinears[1]);
    point2Qua = glGetUniformLocation(myBasicShader.shaderProgram, "pointLightQuads[1]");
    glUniform1f(point2Qua, pointLightQuads[1]);
    
    // fog
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);
}

void renderWorld(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    world.Draw(shader);
}

void renderBee(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send map model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(beeModel));

    //send map normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw map
    bee.Draw(shader);
}

void renderBeaver(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send map model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(beaverModel));

    //send map normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw map
    beaver.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
	//render the scene

	// render the teapot
	renderWorld(myBasicShader);
	renderBee(myBasicShader);
	renderBeaver(myBasicShader);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
  
	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());
		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
