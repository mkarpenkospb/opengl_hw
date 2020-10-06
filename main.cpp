#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#define TINYOBJLOADER_IMPLEMENTATION
// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include "tiny_obj_loader.h"

#include "opengl_shader.h"
#include "utils.hpp"

static void glfw_error_callback(int error, const char *description) {
    std::cerr << fmt::format("Glfw Error {}: {}\n", error, description);
}

// -------------- mouse variables --------------------------------------
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods );
void scrollCallback( GLFWwindow *window, double xoffset, double yoffset );

struct Texel {
    unsigned char r, g, b, a;
};

Texel texturedata[] = {
        {0x00, 0x00, 0x00, 0xFF},
        {0x00, 0xbf, 0xff, 0xFF},
        {0xcc, 0xdd, 0x00, 0xFF},
        {0xcc, 0xdd, 0x00, 0xFF},
        {0xcc, 0xdd, 0x00, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF},
};

unsigned int texturewidth = 6;
double xpos = 0, ypos  = 0;
glm::vec2 c = glm::vec2(-0.70176, 0.3842);
static float r = ((1 + sqrt(1 + 4 * c.length())) / 2);
double x_down_left = -r, y_down_left = r, x_up_right = r, y_up_right = -r;
double y_offset = 0;
double scale_base = 9.0 / 10.0;
double scale = 1;
int width = 1080;
int height = 1080;

bool follow_mouse = false;
double x_start_pos = 0, y_start_pos = 0;


void create_triangle(GLuint &vbo, GLuint &vao, GLuint &ebo) {
    // create the triangle
    float triangle_vertices[] = {
            -1, 1, 0,	// position vertex 1
            0, 1, 0.0f,	 // color vertex 1

            -1, -1, 0.0f,  // position vertex 1
            0, 0, 0.0f,	 // color vertex 1

            1, -1, 0.0f, // position vertex 1
            1, 0, 0,	 // color vertex 1

            1, 1, 0.0f, // position vertex 1
            1, 1, 0,	 // color vertex 1
    };
    unsigned int triangle_indices[] = {
            0, 1, 2, 0, 3, 2 };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indices), triangle_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void create_pear(GLuint &vbo, GLuint &vao, GLuint &ebo, unsigned int &vertex_num) {
//    std::string inputfile = "../erato-1.obj";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                "../pear_export.obj", "../");

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }
    unsigned int size = shapes[0].mesh.indices.size();

    std::vector<unsigned int> vertex_indexes(size);
    std::vector<unsigned int> normals_indexes(size);
    std::vector<unsigned int> tex_indexes(size);
    vertex_num = size;

//    unsigned int normals_indexes[size];
//    unsigned int tex_indexes[size];

    get_vertex_array(vertex_indexes.data(), normals_indexes.data(), tex_indexes.data(), shapes[0]);

    std::cout << "hoooh" << std::endl;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * attrib.vertices.size(), attrib.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertex_indexes.size(), vertex_indexes.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



int main(int, char **) {
//    test_obj_read();
//    GLuint vbo, vao, ebo;
//    create_pear(vbo, vao, ebo);

    // Use GLFW to create a simple window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;


    // GL 3.3 + GLSL 330
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(width, height, "Dear ImGui - Conan", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    // ------------------------ mouse --------------------

//    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
//    glfwSetMouseButtonCallback( window, mouseButtonCallback );
//    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
//    glfwSetScrollCallback( window, scrollCallback );

    // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }
//   glfwMaximizeWindow(window);
    // create our geometries
    unsigned int vertex_num = 0;
    GLuint vbo, vao, ebo;
    create_pear(vbo, vao, ebo, vertex_num);

    // init shader
    shader_t triangle_shader("simple-shader.vs", "simple-shader.fs");

    // Setup GUI context
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO &io = ImGui::GetIO();
//    ImGui_ImplGlfw_InitForOpenGL(window, true);
//    ImGui_ImplOpenGL3_Init(glsl_version);
//    ImGui::StyleColorsDark();
//    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
//    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    auto const start_time = std::chrono::steady_clock::now();

//   glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
//    GLuint texture;
//    glGenTextures(1, &texture);
//    glBindTexture(GL_TEXTURE_1D, texture);
//
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, texturewidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturedata);
//    glBindTexture(GL_TEXTURE_1D, 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);

        // Fill background with solid color
        glClearColor(0.5f, 0.3f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);



        auto model = glm::rotate(glm::mat4(1), 0.0f, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.4, 0.4, 0.4));
        auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);
        auto mvp = projection * view * model;
//        glm::mat4 identity(1.0);
//        mvp = identity;
        triangle_shader.set_uniform("u_mvp", glm::value_ptr(mvp));
        // Bind triangle shader
        triangle_shader.use();

        // Bind vertex array = buffers + indices
        glBindVertexArray(vao);
        // Execute draw call
        glDrawElements(GL_TRIANGLES, vertex_num, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Generate gui render commands
//        ImGui::Render();
//
//        // Execute gui render commands using OpenGL backend
//        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the backbuffer with the frontbuffer that is used for screen display
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods )
{
    if (button == GLFW_MOUSE_BUTTON_LEFT )
    {
        if (action == GLFW_PRESS) {
            follow_mouse = true;
            glfwGetCursorPos(window, &x_start_pos, &y_start_pos);
        } else {
            follow_mouse = false;
            glfwGetCursorPos(window, &xpos, &ypos);
        }
    }
}



void scrollCallback(GLFWwindow *window, double xoffset, double yoffset ) {
    y_offset = yoffset;
    scale = std::pow(scale_base, y_offset);
    glfwGetCursorPos(window, &xpos, &ypos);
    double x_prop = xpos / width;
    double y_prop = ypos / height;
    double new_width = (x_up_right - x_down_left) * scale;
    double new_height = (y_down_left - y_up_right) * scale;
    double difx = (x_up_right - x_down_left) - new_width;
    double dify = (y_down_left - y_up_right) - new_height;
    x_down_left = x_down_left + difx * x_prop;
    x_up_right = x_up_right - difx * (1-x_prop);
    y_up_right = y_up_right + dify * y_prop;
    y_down_left = y_down_left - dify * (1-y_prop);
}