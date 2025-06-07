#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "model.h"

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
float angle_x = 0;
float angle_y = 0;

ShaderProgram* sp;
Models::ModelObj clockBody;
Models::ModelObj gearBig;
Models::ModelObj gearSmall;
Models::ModelObj pendulum;

GLuint metalTex;
float radius = 12.0f;

GLuint readTexture(const char* filename) {
    GLuint tex;
    std::vector<unsigned char> image;
    unsigned width, height;
    lodepng::decode(image, width, height, filename);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    return tex;
}

void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_LEFT) speed_x = -PI / 2;
        if (key == GLFW_KEY_RIGHT) speed_x = PI / 2;
        if (key == GLFW_KEY_UP) speed_y = PI / 2;
        if (key == GLFW_KEY_DOWN) speed_y = -PI / 2;
        if (key == GLFW_KEY_W) radius -= 0.5f;
        if (key == GLFW_KEY_S) radius += 0.5f;
    }
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) speed_x = 0;
        if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) speed_y = 0;
    }
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (height == 0) return;
    aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

void initOpenGLProgram(GLFWwindow* window) {
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");
    clockBody.loadFromOBJ("clock.obj");
    gearBig.loadFromOBJ("new_gear_clean.obj");
    gearSmall.loadFromOBJ("new_gear_clean.obj");
    metalTex = readTexture("metal.png");
	pendulum.loadFromOBJ("pendulum.obj");
}

void freeOpenGLProgram(GLFWwindow* window) {
    delete sp;
}

void drawScene(GLFWwindow* window, float angle_x, float angle_y) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float gearRotation = 0.0f;
    gearRotation += 0.0005f;

    static float localTime = 0.0f;
    localTime += 0.001f; // tempo animacji wahadła


    glm::vec3 cameraPos = glm::vec3(
        radius * sin(angle_x),
        radius * sin(angle_y),
        radius * cos(angle_x)
    );

    glm::mat4 V = glm::lookAt(
        cameraPos,
        glm::vec3(0, 2.0f, 0),
        glm::vec3(0, 1, 0)
    );

    glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 50.0f);

    sp->use();
    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniform4f(sp->u("lp"), 5.0f, 10.0f, 5.0f, 1.0f);
    glUniform1i(sp->u("tex"), 0); // aktywne gniazdo tekstury

    // clock body (DREWNO – bez tekstury)
    glm::mat4 M_clock = glm::mat4(1.0f);
    M_clock = glm::rotate(M_clock, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_clock));
    glUniform1i(sp->u("useTexture"), 0); // NIE używamy tekstury
    glUniform4f(sp->u("color"), 0.7f, 0.4f, 0.2f, 1.0f); // kolor drewna
    glBindTexture(GL_TEXTURE_2D, 0); // brak tekstury
    clockBody.draw();

    // gears – metalowe, z teksturą
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    glUniform1i(sp->u("useTexture"), 1); // używamy tekstury
    glUniform4f(sp->u("color"), 1.0f, 1.0f, 1.0f, 1.0f); // tekstura 100%

    // BIG gear
    glm::mat4 M_big = glm::mat4(1.0f);
    M_big = glm::translate(M_big, glm::vec3(-0.4f, 2.8f, 0.0f));
    M_big = glm::rotate(M_big, gearRotation, glm::vec3(0, 0, 1));
    M_big = glm::scale(M_big, glm::vec3(0.8f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_big));
    gearBig.draw();

    // SMALL gear
    glm::mat4 M_small = glm::mat4(1.0f);
    M_small = glm::translate(M_small, glm::vec3(0.4f, 2.8f, 0.0f));
    M_small = glm::rotate(M_small, -gearRotation + 0.196f, glm::vec3(0, 0, 1));
    M_small = glm::scale(M_small, glm::vec3(0.8f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_small));
    gearSmall.draw();

    // PENDULUM – wahadło
    float anglePendulum = sin(localTime * 1.5f) * glm::radians(10.0f); // bujanie ±10°

    glm::mat4 M_pendulum = glm::mat4(1.0f);
    M_pendulum = glm::translate(M_pendulum, glm::vec3(0.0f, 4.0f, 0.2f));
    M_pendulum = glm::rotate(M_pendulum, anglePendulum, glm::vec3(0, 0, 1));
    M_pendulum = glm::scale(M_pendulum, glm::vec3(1.4f));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_pendulum));
    glUniform4f(sp->u("color"), 0.9f, 0.8f, 0.3f, 1.0f);
    pendulum.draw();




    glfwSwapBuffers(window);
}




int main(void) {
    GLFWwindow* window;
    if (!glfwInit()) {
        fprintf(stderr, "Nie mozna zainicjowac GLFW.\n");
        return -1;
    }
    window = glfwCreateWindow(800, 800, "Zegar", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Nie mozna stworzyc okna GLFW.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    initOpenGLProgram(window);

    while (!glfwWindowShouldClose(window)) {
        angle_x += speed_x * 0.01;
        angle_y += speed_y * 0.01;
        drawScene(window, angle_x, angle_y);
        glfwPollEvents();
    }

    freeOpenGLProgram(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
