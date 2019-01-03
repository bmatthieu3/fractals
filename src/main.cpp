#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "shader.hpp"
#include "model.hpp"
#include "settings.hpp"
#include "viewer.hpp"
#include "stb_image.h"

using namespace std;
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

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
            const GLFWvidmode* mode = glfwGetVideoMode(primary);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            window = glfwCreateWindow(800, 600, name.c_str(), NULL, NULL);
            if (window == NULL)
            {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return;
            }
            glfwMakeContextCurrent(window);
            glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

            // glad: load all OpenGL function pointers
            // ---------------------------------------
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                std::cout << "Failed to initialize GLAD" << std::endl;
                return;
            }
            // configure global opengl state
            glEnable(GL_DEPTH_TEST);
            // Tell stbi to flip the y-axis of the loaded image.
            stbi_set_flip_vertically_on_load(true);

            // Set camera movement
            unique_ptr<CircleMovement> movement = make_unique<CircleMovement>(glm::vec3(0.f), 5, 5.f);
            m_viewer.applyMovement(std::move(movement));

            // Loading shaders
            shared_ptr<Shader> animated = make_shared<Shader>("../shaders/vertex_anim.glsl", "../shaders/frag_textured.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("animated-model", animated));
            shared_ptr<Shader> simple = make_shared<Shader>("../shaders/vertex.glsl", "../shaders/frag_colored.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("primitive", simple));

            // Loading meshes
            /*unique_ptr<Model> bob = make_unique<Model>(m_shaders["textured"], "../resources/Content/boblampclean.md5mesh");
            
            bob->applyTransformation(glm::scale(glm::mat4(1.f), glm::vec3(0.05f)));
            m_models.push_back(std::move(bob));*/
            unique_ptr<Model> nanosuit = make_unique<Model>(m_shaders["animated-model"], "../resources/nanosuit/nanosuit.obj");
            nanosuit->applyTransformation(glm::scale(glm::mat4(1.f), glm::vec3(0.2f)));
            m_models.push_back(std::move(nanosuit));
            
            Material material = Material {0.1};
            unique_ptr<Mesh> plane = Mesh::createPlane(material);
            plane->applyTransformation(glm::scale(glm::mat4(1.f), glm::vec3(10.f)));
            m_primitives.push_back(std::move(plane));
        }

        ~App() {
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        void run() {
            // render loop
            // -----------
            while (!glfwWindowShouldClose(window))
            {
                float time = glfwGetTime();
                // input
                // -----
                processInput(window);

                // update
                // ------
                m_viewer.update(time);
                for(uint32_t i = 0; i < m_models.size(); i++) {
                    m_models[i]->update(time);
                }
                // render
                // ------
                glClearColor(0.f, 1.f, 0.f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                for(uint32_t i = 0; i < m_models.size(); i++) {
                    m_models[i]->draw(m_viewer, time);
                }
                for(uint32_t i = 0; i < m_primitives.size(); i++) {
                    m_primitives[i]->draw(m_shaders["primitive"], m_viewer, time);
                }
        
                // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
                // -------------------------------------------------------------------------------
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }

    private:
        bool m_closed;
        GLFWwindow* window;
        Viewer m_viewer;

        map<string, shared_ptr<Shader>> m_shaders;

        std::vector<unique_ptr<Model>> m_models;
        vector<unique_ptr<Mesh>> m_primitives;
};

int main(void)
{	
    App app("Asia Engine");
    app.run();
	
    return 0;
}