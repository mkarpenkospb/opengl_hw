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

int width = 1920;
int height = 1080;

bool follow_mouse = false;
double x_start_pos = 0, y_start_pos = 0, xpos = 0, ypos = 0;
double delta_x = 0;
double delta_y = 0;
double y_offset = 0, scale_base = 9.0 / 10, scale = 1;

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
        "../mountain-skyboxes/Teide/posx.jpg",
        "../mountain-skyboxes/Teide/negx.jpg",
        "../mountain-skyboxes/Teide/posy.jpg",
        "../mountain-skyboxes/Teide/negy.jpg",
        "../mountain-skyboxes/Teide/posz.jpg",
        "../mountain-skyboxes/Teide/negz.jpg"
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


        static float n_air = 1;
        static float n_pear = 1.1;
        static float a = 0.4;
        ImGui::SliderFloat("n_air", &n_air, 1, 10);
        ImGui::SliderFloat("n_pear", &n_pear, 1, 10);
        ImGui::SliderFloat("texture", &a, 0, 1);


        if (follow_mouse) {
            glfwGetCursorPos(window, &xpos, &ypos);
            delta_x += (xpos - x_start_pos) * 0.01;
            delta_y += (ypos - y_start_pos) * 0.01;
            x_start_pos = xpos;
            y_start_pos = ypos;
        }


        auto model = glm::mat4(1.0);

        auto pear_model = model * glm::scale(glm::vec3(0.2 / scale , 0.2 / scale, 0.2 / scale));
        auto cube_model = model;

        auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        view = glm::rotate(view, glm::radians((float) delta_x * 60),  glm::vec3(0,1,0));
        view = glm::rotate(view, glm::radians((float) delta_y * 60), normalize(glm::vec3(glm::inverse(view)[0])));

        auto projection = glm::perspective<float>(90, float(display_w) / display_h, 0.1, 100);

        glm::mat4 identity(1.0);

        auto cameraPos = glm::vec3(glm::inverse(view)[3]);

        // use Cube shader
        glDepthMask(GL_FALSE);
        cube_shader.use();
        cube_shader.set_uniform("projection", glm::value_ptr(projection));
        cube_shader.set_uniform("view", glm::value_ptr(view));
        cube_shader.set_uniform("model", glm::value_ptr(cube_model));

        glBindVertexArray(c_vao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, cube_vertex_num);
        glDepthMask(GL_TRUE);

        glEnable(GL_DEPTH_TEST);

        // Bind triangle shader
        triangle_shader.use();
        triangle_shader.set_uniform("model", glm::value_ptr(pear_model));
        triangle_shader.set_uniform("view", glm::value_ptr(view));
        triangle_shader.set_uniform("projection", glm::value_ptr(projection));
        triangle_shader.set_uniform("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
        triangle_shader.set_uniform("n_to", n_pear);
        triangle_shader.set_uniform("n_from", n_air);
        triangle_shader.set_uniform("a", a);
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