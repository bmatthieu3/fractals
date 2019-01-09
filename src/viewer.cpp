// Include GLM core features
#include <glm/vec2.hpp>           // vec2
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  // radians
#include <glm/gtc/matrix_transform.hpp> // perspective, translate, rotate

#include <memory>

#include "viewer.hpp"
#include "settings.hpp"

using namespace std;

void Viewer::setPerspectiveProjection(float fov,
    float ratio,
    float zNear,
    float zFar) {
    m_projection_mat = glm::perspective(fov, ratio, zNear, zFar);
}

void Viewer::setOrthoProjection(float halfSize,
    float zNear,
    float zFar 
) {
    m_projection_mat = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, zNear, zFar);
}

Viewer::Viewer(const glm::vec3& position, const glm::vec3& center): 
    // No movement, so static view
    m_movement(nullptr),
    m_center(center),
    m_position(position) {

    m_view_mat = glm::lookAt(m_position, m_center, glm::vec3(0, 1, 0));
    this->setPerspectiveProjection();
}

Viewer::Viewer(const glm::vec3& position, float theta, float delta, bool degree):
    m_movement(nullptr),
    m_position(position) {
    if(degree) {
        theta = glm::radians(theta);
        delta = glm::radians(delta);
    }
    const glm::vec3 direction(glm::vec3(sin(theta)*sin(delta), cos(delta), cos(theta)*sin(delta)));
    m_center = m_position + direction;

    m_view_mat = glm::lookAt(m_position, m_center, glm::vec3(0, 1, 0));
    this->setPerspectiveProjection();
}

Viewer::~Viewer() {
}

void Viewer::applyMovement(std::unique_ptr<Movement> movement) {
    m_movement = std::move(movement);
    m_movement->initState(*this);
}

void Viewer::setPosition(const glm::vec3& position) {
    m_position = position;
    m_view_mat = glm::lookAt(m_position, m_center, glm::vec3(0.f, 1.f, 0.f));
}

const glm::mat4& Viewer::getViewMatrix() const {
    return m_view_mat;
}

const glm::mat4& Viewer::getProjectionMatrix() const {
    return m_projection_mat;
}

const glm::vec3& Viewer::getPosition() const {
    return m_position;
}

glm::vec3 Viewer::getSightDirection() const {
    return glm::normalize(m_center - m_position);
}

void Viewer::update(float dt) {
    if(m_movement) {
        m_movement->update(*this, dt);
    }
}

void Viewer::setDirection(const glm::vec3& dir) {
    m_center = m_position + dir;
    m_view_mat = glm::lookAt(m_position, m_center, glm::vec3(0.f, 1.f, 0.f));
}

CircleMovement::CircleMovement(const glm::vec3& center, float height, float radius): 
    m_center(center),
    m_radius(radius),
    m_height(height) {
}

CircleMovement::~CircleMovement() {

}

void CircleMovement::setCenter(const glm::vec3& center) {
    m_center = center;
}

void CircleMovement::update(Viewer& viewer, float dt) {
    float time = glfwGetTime();
    float posZ = cos(time) * m_radius;
    float posX = sin(time) * m_radius;

    glm::vec3 pos = glm::vec3(posX, m_height, posZ);

    viewer.setPosition(pos);
}

FirstPerson::FirstPerson(GLFWwindow *window, const GLFWvidmode* mode): m_window(window), m_mode(mode) {
}
FirstPerson::~FirstPerson() {
}

void FirstPerson::initState(Viewer& viewer) {
    const glm::vec3& d = viewer.getSightDirection();

    theta = glm::atan(d.x/d.z) + M_PI;
    // d is unit length
    phi = glm::acos(d.y);
}

void FirstPerson::update(Viewer& viewer, float dt) {
    // Get the mouse position
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    glm::vec3 d(viewer.getSightDirection());

    float speed = 10.f;
    glm::vec3 h(0);
    bool move = false;
    const glm::vec3& position = viewer.getPosition();

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
        move = true;
        h += d;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        move = true;
        h += glm::normalize(glm::vec3(d.z, 0, -d.x));
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
        move = true;
        h -= d;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        move = true;
        h += glm::normalize(glm::vec3(-d.z, 0, d.x));    
    }
    
    if(move && glm::length(h) > 0) {
        h = glm::normalize(h);
        viewer.setPosition(position + h*dt*speed);
    }

    glm::vec2 dx(xpos - m_mode->width/2, ypos - m_mode->height/2);
    if(glm::length(dx) >= 100) { 
        theta -= 0.002 * dt * dx.x;
        phi += 0.002 * dt * dx.y;
        if(phi >= 3.14 - 0.1) {
            phi = 3.14 - 0.1;
        } else if(phi <= 0.1) {
            phi = 0.1;
        }
        d = glm::vec3(sin(theta)*sin(phi), cos(phi), cos(theta)*sin(phi));
    }
    viewer.setDirection(d);
}
