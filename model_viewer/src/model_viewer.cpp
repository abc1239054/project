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

float lastTime, timeDelta, cameraSpeed;


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
    GLuint programClouds;
    Trackball trackball;
    GLuint heightVBO;
    GLuint terrainVAO;
    GLuint terrainEBO;
    GLuint terrainVBO;
    GLuint cloudsVAO;
    GLuint cloudsVBO;
    GLuint cloudsTexture;
    GLuint defaultVAO = 0;
    GLfloat* vertices;
    GLuint* indices;
    glm::vec3* faces;
    int verticesSize = 0;
    int indicesSize = 0;
    int facesSize = 0;
    GLuint terrainTexture;
    GLuint cubemap;
    float elapsed_time = 0;
    float seed;
    bool enableAnima = false;
    bool enableFog = true;
    glm::vec3 camPos;
    glm::vec3 target;
    float widthOffset = (TERRA_WIDTH * TERRA_SCALE) / 2.0f;
    float lengthOffset = (TERRA_LENGTH * TERRA_SCALE) / 2.0f;
};

struct ContextWtr {
    float elapsed_time = 0;
    GLFWwindow* window;
    GLuint program;
    GLuint waterVAO;
    GLuint waterVBO;
    GLuint wtrBump;
    GLuint defaultVAO = 0;
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

float randomFloat(glm::vec2 st, float seed){
    return glm::fract(glm::sin(glm::dot(st,
                        glm::vec2(12.9898f,78.233f))) * seed);
}

float noise(glm::vec2 st, float seed){
    glm::vec2 i = glm::floor(st);
    glm::vec2 f = glm::fract(st);

    // Four corners in 2D of a tile
    float a = randomFloat(i, seed);
    float b = randomFloat(i + glm::vec2(1.0f, 0.0f), seed);
    float c = randomFloat(i + glm::vec2(0.0f, 1.0f), seed);
    float d = randomFloat(i + glm::vec2(1.0f, 1.0f), seed);

    // Smooth Interpolation
    // Cubic Hermine Curve.  Same as SmoothStep()
    glm::vec2 u = f*f*(3.0f-2.0f*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return glm::mix(a, b, u.x) +
            (c - a)* u.y * (1.0f - u.x) +
            (d - b) * u.x * u.y;
}

void setHeightMap(Context& ctx) {
    int texW, texH, nChannels, incW, incH;
    // Heightmap loading
    unsigned char* mapData = stbi_load((texDir() + "map.jpg").c_str(), &texW, &texH, &nChannels, 3);

    incW = texW / (TERRA_WIDTH + 1);
    incH = texH / (TERRA_LENGTH + 1);
    if (!incH || !incW)
        std::cout << "Error: height map does not fit the terrain";
    else if (mapData)
    {
        for (int i = 0; i <= TERRA_LENGTH; ++i) {
            for (int j = 0; j <= TERRA_WIDTH; ++j) {
                int n = i * (TERRA_WIDTH + 1) + j;
                ctx.vertices[12 * n + 8] = 2 * mapData[(i * texW * incH + j * incW) * nChannels] / 255.0;
            }
        }
    }
    else
        std::cout << "Error: height map load problem\n";
    
    stbi_image_free(mapData);
}

void createRawData(Context &ctx){
    ctx.verticesSize = 12 * (TERRA_LENGTH + 1) * (TERRA_WIDTH + 1);
    ctx.vertices =  new GLfloat[ctx.verticesSize];
    for (int i = 0; i <= TERRA_LENGTH; ++i)
        for (int j = 0; j <= TERRA_WIDTH; ++j) {
            int n = 12 * (i * (TERRA_WIDTH + 1) + j);
            ctx.vertices[n] = - TERRA_SCALE * TERRA_WIDTH / 2 + TERRA_SCALE * j;
            ctx.vertices[n + 1] = 0.0f;
            ctx.vertices[n + 2] = - TERRA_SCALE * TERRA_LENGTH / 2 + TERRA_SCALE * i;
            
            ctx.vertices[n + 3] = 1.0f;
            ctx.vertices[n + 4] = 1.0f;
            ctx.vertices[n + 5] = 1.0f;

            ctx.vertices[n + 6] = i % 2;
            ctx.vertices[n + 7] = j % 2;

            //ctx.vertices[n + 8] = sin(j * 0.2) + cos(i * 0.3) + (0.2 - (-0.2)) * rand() / (RAND_MAX + 1.0) - 0.2;
            ctx.vertices[n + 8] = noise(glm::vec2(ctx.vertices[n], ctx.vertices[n + 2]) * 0.5f, ctx.seed) * 2.0f +
                                  (0.15 - (-0.15)) * rand() / (RAND_MAX + 1.0) - 0.15;
        }
    
    setHeightMap(ctx);

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

void createWater(ContextWtr& ctxWtr)
{
    const GLfloat vertices[] = {
        -50.0,  0.1, -50.0,  0.0,  0.0,
        -50.0,  0.1,  50.0,  0.0, 20.0,
         50.0,  0.1,  50.0, 20.0, 20.0,
         50.0,  0.1,  50.0, 20.0, 20.0,
         50.0,  0.1, -50.0, 20.0,  0.0,
        -50.0,  0.1, -50.0,  0.0,  0.0
    };

    // Generates and populates a vertex buffer object (VBO) for the
    // vertices (DO NOT CHANGE THIS)
    glGenBuffers(1, &ctxWtr.waterVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ctxWtr.waterVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenTextures(1, &ctxWtr.wtrBump);
    glBindTexture(GL_TEXTURE_2D, ctxWtr.wtrBump);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texW, texH, nChannels;
    unsigned char* texData = stbi_load((texDir() + "_water.png").c_str(), &texW, &texH, &nChannels, 3);
    if (texData)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        std::cout << "Error: texture load problem\n";
    stbi_image_free(texData);

    // Creates a vertex array object (VAO) for drawing
    glGenVertexArrays(1, &ctxWtr.waterVAO);
    glBindVertexArray(ctxWtr.waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctxWtr.waterVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(TEXTURE);
    glVertexAttribPointer(TEXTURE, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
    glBindVertexArray(ctxWtr.defaultVAO); // unbinds the VAO
}

void createCube(Context& ctxSky)
{
    // MODIFY THIS PART: Define the six faces (front, back, left,
    // right, top, and bottom) of the cube. Each face should be
    // constructed from two triangles, and each triangle should be
    // constructed from three vertices. That is, you should define 36
    // vertices that together make up 12 triangles. One triangle is
    // given; you have to define the rest!

    const GLfloat vertices[] = {
        -50.0,  50.0, -50.0,
        -50.0, -50.0, -50.0,
         50.0, -50.0, -50.0,
         50.0, -50.0, -50.0,
         50.0,  50.0, -50.0,
        -50.0,  50.0, -50.0,

        -50.0, -50.0,  50.0,
        -50.0, -50.0, -50.0,
        -50.0,  50.0, -50.0,
        -50.0,  50.0, -50.0,
        -50.0,  50.0,  50.0,
        -50.0, -50.0,  50.0,

         50.0, -50.0, -50.0,
         50.0, -50.0,  50.0,
         50.0,  50.0,  50.0,
         50.0,  50.0,  50.0,
         50.0,  50.0, -50.0,
         50.0, -50.0, -50.0,

        -50.0, -50.0,  50.0,
        -50.0,  50.0,  50.0,
         50.0,  50.0,  50.0,
         50.0,  50.0,  50.0,
         50.0, -50.0,  50.0,
        -50.0, -50.0,  50.0,

        -50.0,  50.0, -50.0,
         50.0,  50.0, -50.0,
         50.0,  50.0,  50.0,
         50.0,  50.0,  50.0,
        -50.0,  50.0,  50.0,
        -50.0,  50.0, -50.0,

        -50.0, -50.0, -50.0,
        -50.0, -50.0,  50.0,
         50.0, -50.0, -50.0,
         50.0, -50.0, -50.0,
        -50.0, -50.0,  50.0,
         50.0, -50.0,  50.0
    };

    // Generates and populates a vertex buffer object (VBO) for the
    // vertices (DO NOT CHANGE THIS)
    glGenBuffers(1, &ctxSky.terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ctxSky.terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Creates a vertex array object (VAO) for drawing the cube
    // (DO NOT CHANGE THIS)
    glGenVertexArrays(1, &ctxSky.terrainVAO);
    glBindVertexArray(ctxSky.terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctxSky.terrainVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(ctxSky.defaultVAO); // unbinds the VAO
}

void createTerrain(Context& ctx)
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
    unsigned char* texData = stbi_load((texDir() + "grass.jpg").c_str(), &texW, &texH, &nChannels, 3);
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

void createClouds(Context& ctx) {
    GLfloat vertices[] = {
        -50.0, 6.0, -50.0, 0.0, 0.0,
        -50.0, 6.0,  50.0, 0.0, 1.0,
         50.0, 6.0, -50.0, 1.0, 0.0,
         50.0, 6.0, -50.0, 1.0, 0.0,
        -50.0, 6.0,  50.0, 0.0, 1.0,
         50.0, 6.0,  50.0, 1.0, 1.0
    };


    glGenBuffers(1, &ctx.cloudsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.cloudsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenTextures(1, &ctx.cloudsTexture);
    glBindTexture(GL_TEXTURE_2D, ctx.cloudsTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    int texW, texH, nChannels;
    unsigned char* texData = stbi_load((texDir() + "clouds.png").c_str(), &texW, &texH, &nChannels, STBI_rgb_alpha);
    if (texData)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
        std::cout << "Error: texture load problem\n";
    stbi_image_free(texData);

    glGenVertexArrays(1, &ctx.cloudsVAO);
    glBindVertexArray(ctx.cloudsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.cloudsVBO);
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(TEXTURE);
    glVertexAttribPointer(TEXTURE, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(ctx.defaultVAO);
}

void kill(Context& ctx){
	free(ctx.vertices);
	free(ctx.indices);
	free(ctx.faces);

}

void init(Context& ctx, Context& ctxSky, ContextWtr& ctxWtr)
{
    ctx.program = loadShaderProgram(shaderDir() + "terrain.vert",
        shaderDir() + "terrain.frag");
    
    ctxSky.program = loadShaderProgram(shaderDir() + "skybox.vert",
                                       shaderDir() + "skybox.frag");

    ctxWtr.program = loadShaderProgram(shaderDir() + "water.vert",
                                       shaderDir() + "water.frag");
    
    ctxSky.cubemap = loadCubemap(cubemapDir() + "/Skybox/");

    ctx.seed=(99999 - 10000) * rand() / (RAND_MAX + 1.0) + 10000;

    createRawData(ctx);
    createTerrain(ctx);
    createCube(ctxSky);
    createWater(ctxWtr);

    ctx.elapsed_time = glfwGetTime();

    ctx.camPos = glm::vec3(-3.0f, 7.0f, -3.0f);
    ctx.target = glm::vec3(0.0f, 6.0f, 0.0f);
}

void drawTerrain(Context& ctx, Context& ctxSky, ContextWtr& ctxWtr, glm::vec3& backColor)
{
    static bool isTexture = true, isHeightMap = true, isDiffuse = true, isWtrDiffuse = true, isSpecular = true, isLight = true,
        isAmbient = true, isSkybox = true, isNoise = true, isMoving = true, isWater = true;

    // Vectors and matrices for the skybox shaders
    glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::mat4 t_m = glm::translate(trackballGetRotationMatrix(ctx.trackball), glm::vec3(0.0f, 7.0f, 0.0f));

    //For camera animation
    if (isMoving)
    {
        ctx.elapsed_time += timeDelta * cameraSpeed;

        ctx.camPos.x = 8.0f * cos(ctx.elapsed_time * 0.3f);
        ctx.camPos.y = 3.0f;
        ctx.camPos.z = 8.0f * sin(ctx.elapsed_time * 0.3f);

        ctx.target.x = 8.0f * cos(ctx.elapsed_time * 0.3f + 0.2f);
        ctx.target.y = 2.5f;
        ctx.target.z = 8.0f * sin(ctx.elapsed_time * 0.3f + 0.2f);
    }

    // Define the model, view, and projection matrices here
    glm::mat4 model = glm::mat4(1.0);
    glm::mat4 model_fog = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(ctx.camPos, ctx.target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(1.0f, 1.0f, 0.1f, 100.0f);

    glm::mat4 mvp = projection * view * model, mv = view * model;
    glm::mat4 t_mvp = projection * view * t_m;
    glm::mat4 mvp_fog = projection * view * model_fog;

    // Light
    static glm::vec3 lightPos = glm::vec3(glm::vec4(0.0, 30.0, 50.0, 1.0));
    static glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
    static glm::vec3 ambColor = glm::vec3(0.01, 0.01, 0.01);
    static glm::vec3 difColor = glm::vec3(1.0, 1.0, 1.0);
    static glm::vec3 wtrdifColor = glm::vec3(1.0, 1.0, 1.0);
    static glm::vec3 wtrDifColor = glm::vec3(0.1, 0.3, 1.0);
    static glm::vec3 wtrSpecColor = glm::vec3(0.1, 0.05, 0.00);
    static GLfloat specPower = 4, lightIntensity = 1, ambIntensity = 1, mapHeight = 1, wavingSpeed = 1;

    ctxWtr.elapsed_time += timeDelta * wavingSpeed;
    
    ////// GUI
    if (ImGui::CollapsingHeader("Terrain"))
    {
        ImGui::ColorEdit3("Diffuse Color", &difColor[0]);
        ImGui::DragFloat("Height", &mapHeight, 0.1f, 0.0f, 10.0f);
    }
    
    if (ImGui::CollapsingHeader("Lighting"))
    {
        ImGui::ColorEdit3("Light Color", &lightColor[0]);
        ImGui::DragFloat("Light Intensity", &lightIntensity, 0.1f, 0.0f, 100.0f);
        ImGui::SliderFloat3("Light Position", &lightPos[0], -50.0, 50.0);
        ImGui::Checkbox("Light Enabled", &isLight);

        ImGui::ColorEdit3("Ambient Color", &ambColor[0]);
        ImGui::DragFloat("Ambient Intensity", &ambIntensity, 0.1f, 0.0f, 100.0f);
        ImGui::Checkbox("Ambient Enabled", &isAmbient);
    }

    if (ImGui::CollapsingHeader("Background"))
    {
        ImGui::ColorEdit3("Background Color", &backColor[0]); 
        ImGui::Checkbox("Skybox", &isSkybox);  
    }

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::Checkbox("Moving Camera", &isMoving);  
        if (!isMoving) {
            ImGui::SliderFloat3("Camera Position", &ctx.camPos[0], -50.0, 50.0);  
            ImGui::SliderFloat3("Camera Target", &ctx.target[0], -50.0, 50.0);  
        }
        else
        {
            ImGui::DragFloat("Moving Speed", &cameraSpeed, 0.05f, 0.0f, 5.0f); 
        }
    }

    if (ImGui::CollapsingHeader("Water"))
    {
        ImGui::ColorEdit3("Diffuse Color", &wtrDifColor[0]);
        ImGui::Checkbox("Diffuse Enabled", &isWtrDiffuse);

        ImGui::ColorEdit3("Specular Color", &wtrSpecColor[0]); 
        ImGui::DragFloat("Specular Power", &specPower, 1.0f, 0.0f, 100.0f);
        ImGui::Checkbox("Specular Enabled", &isSpecular);   

        ImGui::DragFloat("Waving Speed", &wavingSpeed, 0.1f, 0.0f, 5.0f);
    }
    


    // draw the skybox
    if (isSkybox) {
        glDepthMask(GL_FALSE);
        glUseProgram(ctxSky.program);
        glUniformMatrix4fv(glGetUniformLocation(ctxSky.program, "u_mvp"),
            1, GL_FALSE, &mvp_fog[0][0]);
        glUniform3fv(glGetUniformLocation(ctxSky.program, "u_view_pos"), 1, &viewPos[0]);
        glBindVertexArray(ctxSky.terrainVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ctxSky.cubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);
    }

    /*glUseProgram(ctx.programClouds);


    glUniformMatrix4fv(glGetUniformLocation(ctx.programClouds, "u_mvp"),
        1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ctx.cloudsTexture);
    glUniform1i(glGetUniformLocation(ctx.programClouds, "u_texture2"), 2);

    glBindVertexArray(ctx.cloudsVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(ctx.defaultVAO);
    glUseProgram(0);*/

    
    glUseProgram(ctx.program);

    // Pass uniforms to terrain shaders
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_mv"), 1, GL_FALSE, &mv[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsed_time);
    glUniform1f(glGetUniformLocation(ctx.program, "u_map_height"), mapHeight);

    glUniform3fv(glGetUniformLocation(ctx.program, "u_light_pos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_light_clr"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "ambient_color"), 1, &ambColor[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "diffuse_color"), 1, &difColor[0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_ambient_int"), ambIntensity);
    glUniform1i(glGetUniformLocation(ctx.program, "u_is_amb"), (int)isAmbient);
    glUniform1i(glGetUniformLocation(ctx.program, "u_is_light"), (int)isLight);
    glUniform1f(glGetUniformLocation(ctx.program, "u_light_int"), lightIntensity);
    glUniform1i(glGetUniformLocation(ctx.program, "u_is_diffuse"), (int)isDiffuse);
    glUniform1i(glGetUniformLocation(ctx.program, "u_is_texture"), (int)isTexture);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.terrainTexture);
    glUniform1i(glGetUniformLocation(ctx.program, "textureFile"), 0);
    glBindVertexArray(ctx.terrainVAO);
    glDrawElements(GL_TRIANGLES, 6 * TERRA_LENGTH*TERRA_WIDTH, GL_UNSIGNED_INT, 0);
    glBindVertexArray(ctx.defaultVAO);
    glUseProgram(0);

    if (isWater) {
        glUseProgram(ctxWtr.program);
        glUniformMatrix4fv(glGetUniformLocation(ctxWtr.program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(ctxWtr.program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1f(glGetUniformLocation(ctxWtr.program, "u_time"), ctxWtr.elapsed_time);

        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_light_pos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_view_pos"), 1, &ctx.camPos[0]);
        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_light_clr"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_ambient_color"), 1, &ambColor[0]);
        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_wtr_diffuse_color"), 1, &wtrDifColor[0]);
        glUniform3fv(glGetUniformLocation(ctxWtr.program, "u_specular_color"), 1, &wtrSpecColor[0]);
        glUniform1f(glGetUniformLocation(ctxWtr.program, "u_specular_power"), specPower);
        glUniform1f(glGetUniformLocation(ctxWtr.program, "u_ambient_int"), ambIntensity);
        glUniform1f(glGetUniformLocation(ctxWtr.program, "u_light_int"), lightIntensity);
        glUniform1i(glGetUniformLocation(ctxWtr.program, "u_is_spec"), (int)isSpecular);
        glUniform1i(glGetUniformLocation(ctxWtr.program, "u_is_diffuse"), (int)isWtrDiffuse);
        glUniform1i(glGetUniformLocation(ctxWtr.program, "u_is_amb"), (int)isAmbient);
        glUniform1i(glGetUniformLocation(ctxWtr.program, "u_is_light"), (int)isLight);

        glBindVertexArray(ctxWtr.waterVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(ctxWtr.defaultVAO);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ctxWtr.wtrBump);
        glUniform1i(glGetUniformLocation(ctxWtr.program, "normalMap"), 1);

        glBindVertexArray(ctxWtr.wtrBump);
        glDrawElements(GL_TRIANGLES, 6 * TERRA_LENGTH * TERRA_WIDTH, GL_UNSIGNED_INT, 0);
        glBindVertexArray(ctxWtr.defaultVAO);
    }

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

void display(Context& ctx, Context& ctxSky, ContextWtr& ctxWtr)
{
    static glm::vec3 backColor = glm::vec3(0.0, 0.0, 0.0);
    glClearColor(backColor.x, backColor.y, backColor.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
    drawTerrain(ctx, ctxSky, ctxWtr, backColor);
}

void reloadShaders(Context* ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = loadShaderProgram(shaderDir() + "terrain.vert",
        shaderDir() + "terrain.frag");
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
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        ctx->enableAnima = !ctx->enableAnima;
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
    Context ctx, ctxSky;
    ContextWtr ctxWtr;
    srand(time(NULL));

    // Create a GLFW window
    glfwSetErrorCallback(errorCallback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.width = 1280;
    ctx.height = 720;
    ctx.aspect = float(ctx.width) / float(ctx.height);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, keyCallback);
    glfwSetCharCallback(ctx.window, charCallback);
    glfwSetMouseButtonCallback(ctx.window, mouseButtonCallback);
    glfwSetCursorPosCallback(ctx.window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(ctx.window, resizeCallback);
    lastTime = glfwGetTime();
    timeDelta = 0;
    cameraSpeed = 1;

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
    init(ctx, ctxSky, ctxWtr);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        timeDelta = glfwGetTime() - lastTime;
        lastTime = glfwGetTime();
        ImGui_ImplGlfwGL3_NewFrame();
        display(ctx, ctxSky, ctxWtr);
        ImGui::Render();
        glfwSwapBuffers(ctx.window);
    }
	
	kill(ctx);

    // Shutdown
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
