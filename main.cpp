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
#define STB_IMAGE_IMPLEMENTATION
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

// https://learnopengl.com/code_viewer.php?code=advanced/cubemaps_skybox_data
float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

int width = 1080;
int height = 1080;

bool follow_mouse = false;
double x_start_pos = 0, y_start_pos = 0, xpos = 0, ypos = 0;
double delta_x = 0;
double delta_y = 0;
double y_offset = 0, scale_base = 9.0 / 10, scale = 1;

void create_cube(GLuint &vbo, GLuint &vao, GLuint &ebo, unsigned int &vertex_num) {

    unsigned int triangle_indices[36]{};
    for (int i = 0 ; i < 36; ++i) {
        triangle_indices[i] = i;
    }
    vertex_num = 36;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indices), triangle_indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);
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

    std::vector<float> vertex_attributes_data; // # вершин * (pos + normals + textures)
    std::vector<unsigned int> vertex_indexes;
    vertex_num = size;

    get_vertex_array(vertex_indexes, vertex_attributes_data, shapes[0], attrib);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_attributes_data.size(), vertex_attributes_data.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertex_indexes.size(), vertex_indexes.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, (3 + 3 + 2) * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, (3 + 3 + 2) * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, (3 + 3 + 2) * sizeof(float), (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void load_image(GLuint & texture)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *image = stbi_load("../pear_diffuse.jpg",
                                     &width,
                                     &height,
                                     &channels,
                                     STBI_rgb);

    std::cout << "width: " << width << ", height: "<< height << ", channels: " << channels;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
}

int main(int, char **) {

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
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
    glfwSetScrollCallback( window, scrollCallback );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

    // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }
//   glfwMaximizeWindow(window);
    // create our geometries


    // init shader
    shader_t triangle_shader("simple-shader.vs", "simple-shader.fs");
    shader_t cube_shader("cube-shader.vs", "cube-shader.fs");

    unsigned int vertex_num = 0;
    GLuint vbo, vao, ebo;
    create_pear(vbo, vao, ebo, vertex_num);

    unsigned int cube_vertex_num = 0;
    GLuint c_vbo, c_vao, c_ebo;
    create_cube(c_vbo, c_vao, c_ebo, cube_vertex_num);

    GLuint texture;
    load_image(texture);

    std::vector<std::string> faces =
    {
        "../mountain-skyboxes/Maskonaive/posx.jpg",
        "../mountain-skyboxes/Maskonaive/negx.jpg",
        "../mountain-skyboxes/Maskonaive/posy.jpg",
        "../mountain-skyboxes/Maskonaive/negy.jpg",
        "../mountain-skyboxes/Maskonaive/posz.jpg",
        "../mountain-skyboxes/Maskonaive/negz.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
//    auto const start_time = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);

        // Fill background with solid color
        glClearColor(0.5f, 0.3f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Gui start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // GUI
//        ImGui::Begin("Triangle Position/Color");
//
//        static float dx = 0.0;
//        static float dy = 0.0;
//        ImGui::SliderFloat("dx", &dx, 0, 40);
//        ImGui::SliderFloat("dy", &dy, 0, 40);
//
//        ImGui::End();


        if (follow_mouse) {
            glfwGetCursorPos(window, &xpos, &ypos);
            delta_x += (xpos - x_start_pos) * 0.01;
            delta_y += (ypos - y_start_pos) * 0.01;
            x_start_pos = xpos;
            y_start_pos = ypos;
        }


        auto model = glm::mat4(1.0);
        auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        view = glm::rotate(view, glm::radians( (float) delta_x * 60),  glm::vec3(0,1,0));
        view = glm::rotate(view, glm::radians( (float) delta_y * 60), glm::vec3(view[0]));

        auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);
        auto pear_scale = glm::scale(glm::vec3(0.3 / scale , 0.3 / scale, 0.3 / scale));
        auto mvp2 = projection * view * model;
        glm::mat4 identity(1.0);
        auto pear_view =  view * pear_scale;
        auto cameraPos = glm::vec3(glm::inverse(pear_view)[3]);

        // use Cube shader
        glDepthMask(GL_FALSE);
        cube_shader.use();
        cube_shader.set_uniform("projection", glm::value_ptr(projection));
        cube_shader.set_uniform("view", glm::value_ptr(view));
        cube_shader.set_uniform("u_mvp2", glm::value_ptr(mvp2));
        glBindVertexArray(c_vao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertex_num);
        glDepthMask(GL_TRUE);

        glEnable(GL_DEPTH_TEST);

        // Bind triangle shader
        triangle_shader.use();
        triangle_shader.set_uniform("model", glm::value_ptr(model));
        triangle_shader.set_uniform("view", glm::value_ptr(pear_view));
        triangle_shader.set_uniform("projection", glm::value_ptr(projection));
        triangle_shader.set_uniform("cameraPos", glm::value_ptr(cameraPos));
        glActiveTexture(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, vertex_num, GL_UNSIGNED_INT, 0);

        glDisable(GL_DEPTH_TEST);
        // Generate gui render commands
        ImGui::Render();

        // Execute gui render commands using OpenGL backend
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
            glfwGetCursorPos(window, &x_start_pos, &x_start_pos);
        }
    }
}



void scrollCallback(GLFWwindow *window, double xoffset, double yoffset ) {
    y_offset = yoffset;
    scale *= std::pow(scale_base, y_offset);
}