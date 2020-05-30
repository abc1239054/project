// Project :)
//
// Modify this file according to the lab instructions.
//

#include "utils.h"
#include "utils2.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <time.h>
#define TERRA_WIDTH 128
#define TERRA_LENGTH 128
#define TERRA_SCALE 0.2f



// The attribute locations we will use in the vertex shader
enum AttributeLocation {
    POSITION = 0,
    NORMAL = 1,
    TEXTURE = 2,
    HEIGHT = 3
};

// Struct for representing an indexed triangle mesh
struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
};

// Struct for representing a vertex array object (VAO) created from a
// mesh. Used for rendering.
struct MeshVAO {
    GLuint vao;
    GLuint vertexVBO;
    GLuint normalVBO;
    GLuint indexVBO;
    int numVertices;
    int numIndices;
};

// Struct for resources and state
struct Context {
    int width;
    int height;
    float aspect;
    GLFWwindow* window;
    GLuint program;
    Trackball trackball;
    GLuint terrainTexture;
    GLuint heightVBO;
    GLuint terrainVAO;
    GLuint terrainEBO;
    GLuint terrainVBO;
    GLuint defaultVAO;
    GLfloat *vertices;
    GLuint *indices;
    glm::vec3* faces;
    int verticesSize = 0;
    int indicesSize = 0;
    int facesSize = 0;
    GLuint cubemap;
    float elapsed_time;
    float seed;
};

// Returns the value of an environment variable
std::string getEnvVar(const std::string& name)
{
    char const* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return std::string();
    }
    else {
        return std::string(value);
    }
}

// Returns the absolute path to the shader directory
std::string shaderDir(void)
{
    std::string rootDir = getEnvVar("PROJECT_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: PROJECT_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/model_viewer/src/shaders/";
}

// Returns the absolute path to the 3D model directory
std::string modelDir(void)
{
    std::string rootDir = getEnvVar("PROJECT_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: PROJECT_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/model_viewer/3d_models/";
}

// Returns the absolute path to the cubemap texture directory
std::string cubemapDir(void)
{
    std::string rootDir = getEnvVar("PROJECT_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: PROJECT_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/model_viewer/cubemaps/";
}

// Returns the absolute path to the cubemap texture directory
std::string texDir(void)
{
    std::string rootDir = getEnvVar("PROJECT_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: PROJECT_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/model_viewer/textures/";
}

void loadMesh(const std::string& filename, Mesh* mesh)
{
    OBJMesh obj_mesh;
    objMeshLoad(obj_mesh, filename);
    mesh->vertices = obj_mesh.vertices;
    mesh->normals = obj_mesh.normals;
    mesh->indices = obj_mesh.indices;
}

void createMeshVAO(Context& ctx, const Mesh& mesh, MeshVAO* meshVAO)
{
    // Generates and populates a VBO for the vertices
    glGenBuffers(1, &(meshVAO->vertexVBO));
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
    auto verticesNBytes = mesh.vertices.size() * sizeof(mesh.vertices[0]);
    glBufferData(GL_ARRAY_BUFFER, verticesNBytes, mesh.vertices.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the vertex normals
    glGenBuffers(1, &(meshVAO->normalVBO));
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
    auto normalsNBytes = mesh.normals.size() * sizeof(mesh.normals[0]);
    glBufferData(GL_ARRAY_BUFFER, normalsNBytes, mesh.normals.data(), GL_STATIC_DRAW);

    // Generates and populates a VBO for the element indices
    glGenBuffers(1, &(meshVAO->indexVBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
    auto indicesNBytes = mesh.indices.size() * sizeof(mesh.indices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, mesh.indices.data(), GL_STATIC_DRAW);

    // Creates a vertex array object (VAO) for drawing the mesh
    glGenVertexArrays(1, &(meshVAO->vao));
    glBindVertexArray(meshVAO->vao);
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
    glBindVertexArray(ctx.defaultVAO); // unbinds the VAO

    // Additional information required by draw calls
    meshVAO->numVertices = mesh.vertices.size();
    meshVAO->numIndices = mesh.indices.size();
}

void initializeTrackball(Context& ctx)
{
    double radius = double(std::min(ctx.width, ctx.height)) / 2.0;
    ctx.trackball.radius = radius;
    glm::vec2 center = glm::vec2(ctx.width, ctx.height) / 2.0f;
    ctx.trackball.center = center;
}

void createRawData(Context &ctx){
    ctx.verticesSize = 12 * (TERRA_LENGTH + 1) * (TERRA_WIDTH + 1);
    ctx.vertices =  new GLfloat[ctx.verticesSize];
    for (int i = 0; i <= TERRA_LENGTH; ++i)
        for (int j = 0; j <= TERRA_WIDTH; ++j) {
            int n = 12 * (i * (TERRA_WIDTH + 1) + j);
            ctx.vertices[n] = 0.0 + TERRA_SCALE * j;
            ctx.vertices[n + 1] = 0.0;
            ctx.vertices[n + 2] = 0.0 + TERRA_SCALE * i;

            ctx.vertices[n + 3] = 1.0;
            ctx.vertices[n + 4] = 1.0;
            ctx.vertices[n + 5] = 1.0;

            ctx.vertices[n + 6] = i % 2;
            ctx.vertices[n + 7] = j % 2;

            ctx.vertices[n + 8] = sin(j * 0.2) + cos(i * 0.3) + (0.2 - (-0.2)) * rand() / (RAND_MAX + 1.0) - 0.2;
        }

    ctx.indicesSize = 6 * TERRA_LENGTH * TERRA_WIDTH;
    ctx.indices = new GLuint[ctx.indicesSize];
    for (GLuint i = 0; i < TERRA_LENGTH; ++i)
        for (GLuint j = 0; j < TERRA_WIDTH; ++j) {
            ctx.indices[6 * (i * TERRA_WIDTH + j)] = i * (TERRA_WIDTH + 1) + j;
            ctx.indices[6 * (i * TERRA_WIDTH + j) + 1] = i * (TERRA_WIDTH + 1) + j + 1;
            ctx.indices[6 * (i * TERRA_WIDTH + j) + 2] = (i + 1) * (TERRA_WIDTH + 1) + j;

            ctx.indices[6 * (i * TERRA_WIDTH + j) + 3] = (i + 1) * (TERRA_WIDTH + 1) + 1 + j;
            ctx.indices[6 * (i * TERRA_WIDTH + j) + 4] = (i + 1) * (TERRA_WIDTH + 1) + j;
            ctx.indices[6 * (i * TERRA_WIDTH + j) + 5] = i * (TERRA_WIDTH + 1) + j + 1;
        }
	
	ctx.facesSize = 2 * TERRA_LENGTH * TERRA_WIDTH;
	ctx.faces = new glm::vec3[ctx.facesSize];
    for (int i = 0; i < 6 * TERRA_LENGTH * TERRA_WIDTH; i+=3) {
        int p1 = ctx.indices[i], p2 = ctx.indices[i+1],  p3 = ctx.indices[i+2];
        glm::vec3 v1(ctx.vertices[12 * p1] - ctx.vertices[12 * p2],
                     ctx.vertices[12 * p1 + 1] - ctx.vertices[12 * p2 + 1] + ctx.vertices[12 * p1 + 8] - ctx.vertices[12 * p2 + 8],
                     ctx.vertices[12 * p1 + 2] - ctx.vertices[12 * p2 + 2]);
        glm::vec3 v2(ctx.vertices[12 * p3] - ctx.vertices[12 * p2],
                     ctx.vertices[12 * p3 + 1] - ctx.vertices[12 * p2 + 1] + ctx.vertices[12 * p3 + 8] - ctx.vertices[12 * p2 + 8],
                     ctx.vertices[12 * p3 + 2] - ctx.vertices[12 * p2 + 2]);
        ctx.faces[i / 3] = glm::normalize(glm::cross(v1, v2));
    }
	
	for (int i = 0; i <= TERRA_LENGTH; ++i)
        for (int j = 0; j <= TERRA_WIDTH; ++j) {
            int n = (i * (TERRA_WIDTH + 1) + j);
            glm::vec3 sum(0);
            bool leftBorder = j == 0, topBorder = i == TERRA_LENGTH, bottomBorder = i == 0;

            if (j != TERRA_WIDTH) {
                if (i != 0) {
                    sum += ctx.faces[2 * n - 2 * i - 2 * TERRA_WIDTH] + ctx.faces[2 * n - 2 * i - 2 * TERRA_WIDTH + 1];
                }
                if (i != TERRA_LENGTH) {
                    sum += ctx.faces[2 * n - 2 * i];
                }
            }
            if (j != 0) {
                if (i != TERRA_LENGTH) {
                    sum += ctx.faces[2 * n - 2 * i - 1] + ctx.faces[2 * n - 2 * i - 2];
                }
                if (i != 0) {
                    sum += ctx.faces[2 * n - 2 * i - 1 - 2 * TERRA_WIDTH];
                }
            }

            sum = glm::normalize(sum);

            ctx.vertices[12 * n + 9] = sum.x;
            ctx.vertices[12 * n + 10] = sum.y;
            ctx.vertices[12 * n + 11] = sum.z;
        }
}


void createCube(Context& ctx)
{
    unsigned int EBO;
    glGenBuffers(1, &ctx.terrainEBO);
    glGenBuffers(1, &ctx.terrainVBO);
    glGenVertexArrays(1, &ctx.terrainVAO);
    
    glBindVertexArray(ctx.terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * ctx.verticesSize, ctx.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * ctx.verticesSize , ctx.indices, GL_STATIC_DRAW);

    glGenTextures(1, &ctx.terrainTexture);
    glBindTexture(GL_TEXTURE_2D, ctx.terrainTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texW, texH, nChannels;
    unsigned char* texData = stbi_load((texDir() + "grass.jpg").c_str(), &texW, &texH, &nChannels, 0);
    if (texData)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        std::cout << "Error: texture load problem\n";
    stbi_image_free(texData);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.terrainVAO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(NORMAL);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(TEXTURE);
    glVertexAttribPointer(TEXTURE, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(HEIGHT);
    glVertexAttribPointer(HEIGHT, 1, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(8 * sizeof(float)));
    glBindVertexArray(ctx.defaultVAO); // unbinds the VAO
}

void kill(Context& ctx){
	free(ctx.vertices);
	free(ctx.indices);
	free(ctx.faces);
}

void init(Context& ctx)
{
    ctx.program = loadShaderProgram(shaderDir() + "terrain.vert",
        shaderDir() + "terrain.frag");

    createRawData(ctx);
    createCube(ctx);

    ctx.seed=(99999 - 10000) * rand() / (RAND_MAX + 1.0) + 10000;
}

// MODIFY THIS FUNCTION
void drawTerrain(Context& ctx)
{
    glUseProgram(ctx.program);

    // Define the model, view, and projection matrices here
    glm::mat4 model = trackballGetRotationMatrix(ctx.trackball);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    view = glm::lookAt(glm::vec3(-3.0f, 6.0f, -3.0f), glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    projection = glm::perspective(1.0f, 1.0f, 0.1f, 100.0f);

    glm::mat4 mvp = projection * view * model, mv = view * model;

    // Light
    glm::vec3 lightPos = glm::vec3(glm::vec4(-10.0, 10.0, 0.0, 1.0));
    glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
    glm::vec3 ambColor = glm::vec3(0.01, 0.01, 0.01);
    glm::vec3 difColor = glm::vec3(1.0, 1.0, 1.0);
    glm::vec3 specColor = glm::vec3(0.04, 0.04, 0.04);
    GLfloat specPower = 4;

    // Concatenate the model, view, and projection matrices to a
    // ModelViewProjection (MVP) matrix and pass it as a uniform
    // variable to the shader program.
    //
    // Hint: you pass GLM matrices to shader programs like this:
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_mvp"),
        1, GL_FALSE, &mvp[0][0]);

    // Bind textures
    // ...

    // Pass uniforms
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_mv"), 1, GL_FALSE, &mv[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsed_time);

    glUniform3fv(glGetUniformLocation(ctx.program, "u_light_pos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_light_clr"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "ambient_color"), 1, &ambColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "diffuse_color"), 1, &difColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "specular_color"), 1, &specColor[0]);
    glUniform1f(glGetUniformLocation(ctx.program, "specular_power"), specPower);

    glUniform1f(glGetUniformLocation(ctx.program, "seed"), ctx.seed);
    // ...

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.terrainTexture);
    glBindVertexArray(ctx.terrainVAO);
    glDrawElements(GL_TRIANGLES, 6 * TERRA_LENGTH*TERRA_WIDTH, GL_UNSIGNED_INT, 0);
    glBindVertexArray(ctx.defaultVAO);

    glUseProgram(0);
}

/*
void display(Context& ctx)
{
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
    drawMesh(ctx, ctx.program, ctx.terrainVAO);
}*/

void display(Context& ctx)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
    drawTerrain(ctx);
}

void reloadShaders(Context* ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = loadShaderProgram(shaderDir() + "mesh.vert",
        shaderDir() + "mesh.frag");
}

void mouseButtonPressed(Context* ctx, int button, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        trackballStartTracking(ctx->trackball, glm::vec2(x, y));
    }
}

void mouseButtonReleased(Context* ctx, int button, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        trackballStopTracking(ctx->trackball);
    }
}

void moveTrackball(Context* ctx, int x, int y)
{
    if (ctx->trackball.tracking) {
        trackballMove(ctx->trackball, glm::vec2(x, y));
    }
}

void errorCallback(int /*error*/, const char* description)
{
    std::cerr << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) { return; }  // Skip other handling

    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        reloadShaders(ctx);
    }
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) { return; }  // Skip other handling
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        mouseButtonPressed(ctx, button, x, y);
    }
    else {
        mouseButtonReleased(ctx, button, x, y);
    }
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling   

    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    moveTrackball(ctx, x, y);
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    ctx->aspect = float(width) / float(height);
    ctx->trackball.radius = double(std::min(width, height)) / 2.0;
    ctx->trackball.center = glm::vec2(width, height) / 2.0f;
    glViewport(0, 0, width, height);
}

int main(void)
{
    Context ctx;
    srand(time(NULL));

    // Create a GLFW window
    glfwSetErrorCallback(errorCallback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.width = 500;
    ctx.height = 500;
    ctx.aspect = float(ctx.width) / float(ctx.height);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, keyCallback);
    glfwSetCharCallback(ctx.window, charCallback);
    glfwSetMouseButtonCallback(ctx.window, mouseButtonCallback);
    glfwSetCursorPosCallback(ctx.window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(ctx.window, resizeCallback);

    // Load OpenGL functions
    glewExperimental = true;
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize GUI
    ImGui_ImplGlfwGL3_Init(ctx.window, false /*do not install callbacks*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.defaultVAO);
    glBindVertexArray(ctx.defaultVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    init(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsed_time = glfwGetTime();
        ImGui_ImplGlfwGL3_NewFrame();
        display(ctx);
        ImGui::Render();
        glfwSwapBuffers(ctx.window);
    }
	
	kill(ctx);

    // Shutdown
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
