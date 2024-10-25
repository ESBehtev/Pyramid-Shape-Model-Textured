//
//  main.cpp
//  Pyramid-Shape-Model-Textured
//
//  Created by Эдвард on 25.10.2024.
//

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Connecting the library to work with images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Vertex coordinates, normals, and texture coordinates for the pyramid
GLfloat vertices[] = {
    // Vertex 1 (lower left)
    -2.0f,  0.0f, 0.0f,  // Position
     1.0f,  1.0f, 1.0f,  // Color
     0.0f,  0.0f, 1.0f,  // Normale
     0.0f,  0.0f,        // Texture coordinates

    // Vertex 2 (upper left)
    -1.0f,  3.0f, 0.0f,
     1.0f,  1.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.0f,  1.0f,

    // Vertex 3 (bottom right)
     2.0f,  0.0f, 0.0f,
     1.0f,  1.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     1.0f,  0.0f,

    // Vertex 4 (upper right)
     3.0f,  3.0f, 0.0f,
     1.0f,  1.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     1.0f,  1.0f,

    // Vertex pyramid (up)
     0.0f,  0.0f, 5.0f,
     1.0f,  1.0f, 1.0f,
     0.0f,  0.0f, 1.0f,
     0.5f,  0.5f
};

// Indexes for rendering the pyramid
GLuint indices[] = {
    // The base of the parallelogram
    0, 1, 2,
    1, 2, 3,

    // The side faces of the pyramid
    0, 1, 4,
    1, 3, 4,
    3, 2, 4,
    2, 0, 4   
};

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
    return textureID;
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
out vec3 ourColor;
out vec3 fragNormal;
out vec2 TexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    fragNormal = aNormal;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
in vec3 fragNormal;
in vec2 TexCoord;
out vec4 FragColor;
uniform vec3 lightDir;
uniform sampler2D texture1;
void main()
{
    float ambient = 0.1;
    float diff = max(dot(normalize(fragNormal), normalize(lightDir)), 0.0);
    vec3 result = (ambient + diff) * ourColor;
    vec4 textureColor = texture(texture1, TexCoord);
    FragColor = textureColor * vec4(result, 1.0);
}
)";

GLuint setupShaders(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Pyramid", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Включаем буфер глубины
    glEnable(GL_DEPTH_TEST);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShaders(vertexShaderSource, fragmentShaderSource);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    GLuint texture = loadTexture("/Users/mercurial/Downloads/Pyramid-Shape-Model-Textured/Pyramid-Shape-Model-Textured/Pyramid-Shape-Model-Textured/texture.png");
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glm::mat4 view = glm::lookAt(glm::vec3(20.0f, 20.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    float t = 0.0f;
    float A = 10.0f, B = 5.0f;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        int lightDirLoc = glGetUniformLocation(shaderProgram, "lightDir");

        // The movement of light along an elliptical trajectory
        float lightX = A * cos(t);
        float lightY = B * sin(t);
        float lightZ = -(lightX - lightY + 10);
        glm::vec3 lightDir = glm::normalize(glm::vec3(-lightX, -lightY, -lightZ));

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
        glUniform3f(lightDirLoc, lightDir.x, lightDir.y, lightDir.z);

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(t * 50.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model1 = glm::translate(rotation, glm::vec3(-2.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model1[0][0]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

       
        t += 0.01f;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
