#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "shader.hpp"
#include "screen.hpp"
#include "settings.hpp"
#include "stb_image.h"

using namespace std;

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}



class App {
    public:
        App(const std::string& name) : m_closed(false) {
            // glfw: initialize and configure
            // ------------------------------
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
        #endif

            // glfw window creation
            // --------------------
            GLFWmonitor* primary = glfwGetPrimaryMonitor();
            m_mode = glfwGetVideoMode(primary);
            glfwWindowHint(GLFW_RED_BITS, m_mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, m_mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, m_mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, m_mode->refreshRate);
            window = glfwCreateWindow(m_mode->width, m_mode->height, name.c_str(), primary, NULL);
            if (window == NULL)
            {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return;
            }
            glfwMakeContextCurrent(window);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

            // Set key callback
            //glfwSetKeyCallback(window, key_callback);

            // glad: load all OpenGL function pointers
            // ---------------------------------------
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                std::cout << "Failed to initialize GLAD" << std::endl;
                return;
            }
            // configure global opengl state
            glEnable(GL_DEPTH_TEST);
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            // Tell stbi to flip the y-axis of the loaded image.
            stbi_set_flip_vertically_on_load(true);

            // Loading shaders
            shared_ptr<Shader> fractals_shader = make_shared<Shader>("./shaders/vertex_fractals.glsl", "./shaders/frag_fractals.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("fractals", fractals_shader));

            m_screen = make_unique<ScreenQuad>();
            std::cout << "Init terminated successfully" << std::endl;
        }

        ~App() {
            m_shaders.clear();

            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void run() {
            // render loop
            // -----------
            float time = glfwGetTime();
            float prev_time = time;
            float pos_center_x = 0.f;
            float pos_center_y = 0.f;
            float depl_val = 0.1f;
            float zoom = 1.f;
            while (!glfwWindowShouldClose(window)) {
                prev_time = time;
                time = glfwGetTime();

                float dt = 10.f*(time - prev_time);
                // clear the screen
                // ------
                glClearColor(0.f, 0.0f, 0.f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // input
                // -----
                // Quit the program when pressing escape
                if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                    glfwSetWindowShouldClose(window, true);
                }

                if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                    pos_center_y += dt*(depl_val/zoom);
                }
                if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                    pos_center_y -= dt*(depl_val/zoom);
                }

                if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                    pos_center_x += dt*(depl_val/zoom);
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                    pos_center_x -= dt*(depl_val/zoom);
                }

                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                    zoom += 1.f;
                }
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                    zoom -= 1.f;

                    zoom = std::max(1.f, zoom);
                }

                // draw
                // ------
                // Update the viewers
                m_screen->draw(m_shaders["fractals"], time, pos_center_x, pos_center_y, zoom);

                // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
                // -------------------------------------------------------------------------------
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }

    private:
        bool m_closed;
        GLFWwindow* window;
        const GLFWvidmode* m_mode;

        map<string, shared_ptr<Shader>> m_shaders;

        unique_ptr<ScreenQuad> m_screen;
};

int main(void)
{	
    App app("Fractals");
    app.run();
	
    return 0;
}