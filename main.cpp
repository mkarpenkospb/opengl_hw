#include <iostream>
#include <vector>
#include <chrono>

#include <fmt/format.h>

#include <GL/glew.h>

// Imgui + bindings
#include "imgui.h"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// Math constant and routines for OpenGL interop
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shader.h"

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
        {0x00, 0x00, 0x00, 0xFF}, //
        {0x00, 0xbf, 0xff, 0xFF}, //
        {0xcc, 0xdd, 0x00, 0xFF}, //
        {0xcc, 0xdd, 0x00, 0xFF}, //
        {0xcc, 0xdd, 0x00, 0xFF}, //
        {0xFF, 0xFF, 0xFF, 0xFF}, //
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
            // 0.0f, 0.25f, 0.0f,	// position vertex 1
            -1.0f, -1.0f, 0.0f,
            1.0f, 0.4f, 0.2f,     // color vertex 1

            -1.0f, 1.0f, 0.0f,  // position vertex 1
            0.9f, 1.0f, 0.5f,     // color vertex 1

            1.0f, 1.0f, 0.0f, // position vertex 1
            0.1f, 0.5f, 1.0f,

            1.0f, -1.0f, 0.0f, // position vertex 1
            0.1f, 0.5f, 1.0f,
    };
    unsigned int triangle_indices[] = {0, 1, 2, 0, 2, 3};
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangle_indices), triangle_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
    // ------------------------ mouse --------------------

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
//    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
    glfwSetScrollCallback( window, scrollCallback );

    // Initialize GLEW, i.e. fill all possible function pointers for current OpenGL context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return 1;
    }
//   glfwMaximizeWindow(window);
    // create our geometries
    GLuint vbo, vao, ebo;
    create_triangle(vbo, vao, ebo);

    // init shader
    shader_t triangle_shader("simple-shader.vs", "simple-shader.fs");

    // Setup GUI context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
//    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
//    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    auto const start_time = std::chrono::steady_clock::now();

//   glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_1D, texture);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, texturewidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturedata);
    glBindTexture(GL_TEXTURE_1D, 0);




    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get windows size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Set viewport to fill the whole window area
        glViewport(0, 0, display_w, display_h);

        // Fill background with solid color
        glClearColor(0.30f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Gui start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // GUI
        ImGui::Begin("Fractal options");
        static int iteration = 600;
        ImGui::InputInt("iteration", &iteration, 100);

        static float rscale = 1;
        ImGui::InputFloat("rscale", &rscale, 0.02);

//        static float c_real = -0.70176;
//        ImGui::InputFloat("real", &c_real);
//
//        static float c_img = -0.3842;
//        ImGui::InputFloat("imgn", &c_img);

        ImGui::End();

        float const time_from_start = (float) (
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_time).count() /
                1000.0);
        triangle_shader.set_uniform("u_time", time_from_start);

        triangle_shader.set_uniform("u_iteration", iteration);
        triangle_shader.set_uniform("rscale", rscale);

        static glm::vec2 const u_resolution = glm::vec2((float)width, (float)height);
        triangle_shader.set_uniform("u_resolution", u_resolution[0], u_resolution[1]);

        triangle_shader.set_uniform("c", c[0], c[1]);

        triangle_shader.set_uniform("r", (float)r);

        auto model = glm::rotate(glm::mat4(1), glm::radians((float) 0), glm::vec3(0, 1, 0));
        auto view = glm::lookAt<float>(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0));
        auto projection = glm::perspective<float>(100, 1, 0.1, 100);
        auto mvp = projection * view * model;

        triangle_shader.set_uniform("u_mvp", glm::value_ptr(mvp));

        // Bind triangle shader
        triangle_shader.use();
        // ------------------------------------- texture --------------------------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, texture);
        // ------------------------------------- mouse -----------------------------------------------
        if (follow_mouse) {
            glfwGetCursorPos(window, &xpos, &ypos);
            double delta_x = (xpos - x_start_pos) / width * (x_up_right - x_down_left);
            double delta_y = (ypos - y_start_pos) / height * (y_down_left - y_up_right);
            x_start_pos = xpos;
            y_start_pos = ypos;
            x_down_left -= delta_x;
            x_up_right -= delta_x;
            y_down_left -= delta_y;
            y_up_right -= delta_y;
        }

        triangle_shader.set_uniform("x_down_left", (float)(x_down_left));
        triangle_shader.set_uniform("y_down_left", (float)(y_down_left));
        triangle_shader.set_uniform("x_up_right", (float)(x_up_right));
        triangle_shader.set_uniform("y_up_right", (float)(y_up_right));
        // Bind vertex array = buffers + indices
        glBindVertexArray(vao);
        // Execute draw call
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Generate gui render commands
        ImGui::Render();

        // Execute gui render commands using OpenGL backend
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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