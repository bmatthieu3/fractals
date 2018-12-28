// Include GLM core features
#include <glm/vec2.hpp>           // vec2
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  // radians
#include <glm/gtc/matrix_transform.hpp> // perspective, translate, rotate

#include "viewer.hpp"
#include "settings.hpp"

Viewer::Viewer() {
    glm::vec3 position = glm::vec3(5.f, 5.f, 5.f);
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 center = glm::vec3(0.f);
    m_view_mat = glm::lookAt(position, center, up);

    m_projection_mat = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
}
Viewer::~Viewer() {
}
const glm::mat4& Viewer::getViewMatrix() const {
    return m_view_mat;
}
const glm::mat4& Viewer::getProjectionMatrix() const {
    return m_projection_mat;
}

