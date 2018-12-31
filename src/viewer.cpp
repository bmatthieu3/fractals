// Include GLM core features
#include <glm/vec2.hpp>           // vec2
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  // radians
#include <glm/gtc/matrix_transform.hpp> // perspective, translate, rotate

#include <memory>

#include "viewer.hpp"
#include "settings.hpp"

Viewer::Viewer(): m_movement(nullptr),
    m_center(glm::vec3(0.f)),
    m_position(glm::vec3(5, 5, 5)) {
    m_view_mat = glm::lookAt(m_position, m_center, glm::vec3(0, 1, 0));

    m_projection_mat = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
}
Viewer::~Viewer() {
}
void Viewer::applyMovement(std::unique_ptr<Movement> movement) {
    m_movement = std::move(movement);
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
void Viewer::update(float time) {
    if(m_movement) {
        m_movement->update(*this, time);
    }
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
void CircleMovement::update(Viewer& viewer, float time) {
    float posZ = cos(time) * m_radius;
    float posX = sin(time) * m_radius;

    glm::vec3 pos = glm::vec3(posX, m_height, posZ);

    viewer.setPosition(pos);
}