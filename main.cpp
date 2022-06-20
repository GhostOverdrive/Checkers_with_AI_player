#define GLEW_STATIC
#include <GL/glew.h>

#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

unsigned int CreateShaderProgram(const GLchar* vertexShaderText, const GLchar* fragmentShaderText);
std::string LoadFileContents(const std::string filePath);

int main()
{
    sf::ContextSettings settings;

    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.depthBits = 24;

    sf::RenderWindow window(sf::VideoMode(500, 500), "Chessboard", sf::Style::Titlebar | sf::Style::Close, settings);
    window.setActive(true);

    glewExperimental = true;
    GLenum result = glewInit();
    if (result != GLEW_OK)
    {
        std::cout << "Glew failed to initialize: " << glewGetErrorString(result) << std::endl;
    }

    float vertices[] = {
        0.0f, 1.0f, // (x, y)
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    GLuint vbo;
    GLuint vao;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //std::string vertexShaderText = LoadFileContents("shader.vert.glsl");
    //std::string fragmentShaderText = LoadFileContents("shader.frag.glsl");

    std::string vertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec2 a_position;"
        "uniform mat4 u_mpMatrix;"
        "void main()"
        "{"
        "    gl_Position = u_mpMatrix * vec4(a_position, 0.0, 1.0);"
        "}";

    std::string fragmentShaderSource =
        "#version 330 core\n"
        "uniform vec3 u_color;"
        "out vec4 outColor;"
        "void main()"
        "{"
        "    outColor = vec4(u_color, 1.0);"
        "}";

    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
    glUseProgram(shaderProgram);

    GLint u_Color = glGetUniformLocation(shaderProgram, "u_color");

    glm::mat4x4 modelMatrix;
    glm::mat4 mpMatrix;
    glm::mat4 projectionMatrix = glm::ortho(0.0f, 8.0f, 0.0f, 8.0f);

    glViewport(20, 20, 460, 460);

    GLint u_mpMatrix = glGetUniformLocation(shaderProgram, "u_mpMatrix");

    while (window.isOpen())
    {
        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t row = 0; row < 8; row++)
        {
            for (size_t col = 0; col < 8; col++)
            {
                modelMatrix = glm::translate(glm::mat4(), glm::vec3(col, row, 0.0f));
                mpMatrix = projectionMatrix * modelMatrix;
                glUniformMatrix4fv(u_mpMatrix, 1, false, &mpMatrix[0][0]);
                if ((row % 2 == 0 && col % 2 == 0) || (row % 2 != 0 && col % 2 != 0))
                {
                    glUniform3f(u_Color, 0.0f, 0.0f, 0.0f);
                }
                else
                {
                    glUniform3f(u_Color, 0.823f, 0.690f, 0.356f);
                }
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }

        window.display();
    }

    return 0;
}

GLuint CreateShaderProgram(const GLchar * vertexShaderText, const GLchar * fragmentShaderText)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaderProgram = glCreateProgram();

    glShaderSource(vertexShader, 1, &vertexShaderText, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "Vertex Shader compilation failed: " << infoLog << std::endl;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderText, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "Fragment Shader compilation failed: " << infoLog << std::endl;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "Program failed to link: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

std::string LoadFileContents(const std::string filePath)
{
    std::ifstream file(filePath);
    std::stringstream sstream;

    if (!file.is_open())
    {
        std::cout << "Could not find the file: " << filePath << std::endl;
    }

    sstream << file.rdbuf();

    return sstream.str();
}
