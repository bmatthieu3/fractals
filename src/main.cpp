#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "shader.hpp"
#include "model.hpp"
#include "screen.hpp"
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
            m_mainViewer = Viewer::createPerspectiveViewer(glm::vec3(5, 5, 5), glm::vec3(0));
            m_sunViewer = Viewer::createOrthoViewer(glm::vec3(5, 5, -5), glm::vec3(0), 5.f);

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

            // Create a new framebuffer associated with a depth map.
            // This framebuffer will write into the map the depth of all the fragments
            // from a light point of view.
            m_depthMap = Texture::createDepthMap();

            glGenFramebuffers(1, &m_depthFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, m_depthFBO);
            // Attach the depth map texture to the currently bound framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMap->id, 0);
            // As a framebuffer must be attached with a color buffer too, we tell him we will not use read and/or draw
            // from it so that we do not have to specify it.
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Set camera movement
            unique_ptr<CircleMovement> movement = make_unique<CircleMovement>(glm::vec3(0.f), 5, 5.f);
            m_mainViewer->applyMovement(std::move(movement));

            // Loading shaders
            shared_ptr<Shader> animated = make_shared<Shader>("../shaders/vertex_anim.glsl", "../shaders/frag_textured.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("animated-model", animated));
            shared_ptr<Shader> simple = make_shared<Shader>("../shaders/vertex.glsl", "../shaders/frag_textured.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("primitive", simple));
            // Shader that draw a texture map onto a quad.
            // Useful for debugging purposes.
            shared_ptr<Shader> quad = make_shared<Shader>("../shaders/vertex_screen.glsl", "../shaders/frag_depth_map.glsl");
            m_shaders.insert(pair<string, shared_ptr<Shader>>("simple-screen", quad));

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

            // Create a screen quad object
            m_quad = make_unique<ScreenQuad>(m_depthMap);
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
                m_mainViewer->update(time);
                for(uint32_t i = 0; i < m_models.size(); i++) {
                    m_models[i]->update(time);
                }
                // render
                // ------
                // Write onto the depth FBO
                glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
                glBindFramebuffer(GL_FRAMEBUFFER, m_depthFBO);
                glClear(GL_DEPTH_BUFFER_BIT);

                writeToCurrentFBO(*m_sunViewer, time);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // Write onto the screen FBO
                glViewport(0, 0, m_mode->width, m_mode->height);
                glClearColor(0.f, 1.f, 0.f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                writeToCurrentFBO(*m_mainViewer, time);
                //renderDebugTextureMap();

                // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
                // -------------------------------------------------------------------------------
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
        }

    private:
        void writeToCurrentFBO(const Viewer& viewer, float time) {
            for(auto& model: m_models) {
                const shared_ptr<Shader> shader = model->getShader();
                shader->bind();
                // Set global uniforms
                shader->sendUniform1f("time", time);
                shader->sendUniform1i("screen_w", m_mode->width);
                shader->sendUniform1i("screen_h", m_mode->height);

                shader->sendUniformMatrix4fv("viewLightSpace", m_sunViewer->getViewMatrix());
                shader->sendUniformMatrix4fv("clipLightSpace", m_sunViewer->getProjectionMatrix());

                glActiveTexture(GL_TEXTURE0);
                shader->sendUniform1i("depth_map", 0);
                glBindTexture(GL_TEXTURE_2D, m_depthMap->id);

                model->draw(viewer);
            }
            const shared_ptr<Shader> primitiveShader = m_shaders["primitive"];
            for(auto& primitive: m_primitives) {
                primitiveShader->bind();
                // Set global uniforms
                primitiveShader->sendUniform1f("time", time);
                primitiveShader->sendUniform1i("screen_w", m_mode->width);
                primitiveShader->sendUniform1i("screen_h", m_mode->height);
                
                primitiveShader->sendUniformMatrix4fv("viewLightSpace", m_sunViewer->getViewMatrix());
                primitiveShader->sendUniformMatrix4fv("clipLightSpace", m_sunViewer->getProjectionMatrix());
                
                glActiveTexture(GL_TEXTURE0);
                primitiveShader->sendUniform1i("depth_map", 0);
                glBindTexture(GL_TEXTURE_2D, m_depthMap->id);
    
                primitive->draw(primitiveShader, viewer);
            }
        }

        void renderDebugTextureMap() {
            m_quad->draw(m_shaders["simple-screen"]);
        }

    private:
        bool m_closed;
        GLFWwindow* window;
        const GLFWvidmode* m_mode;
        unique_ptr<Viewer> m_mainViewer;
        unique_ptr<Viewer> m_sunViewer;

        // Depth FrameBuffer object
        shared_ptr<Texture> m_depthMap;
        GLuint m_depthFBO;

        map<string, shared_ptr<Shader>> m_shaders;

        std::vector<unique_ptr<Model>> m_models;
        vector<unique_ptr<Mesh>> m_primitives;
        
        // Texture map screen vizualizer (for debugging purposes)
        unique_ptr<ScreenQuad> m_quad;
};

int main(void)
{	
    App app("Asia Engine");
    app.run();
	
    return 0;
}