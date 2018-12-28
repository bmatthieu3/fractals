#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__

#include <glm/mat4x4.hpp>         // mat4

class Viewer {
    public:
        Viewer();
        ~Viewer();

        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjectionMatrix() const;

    private:
        glm::mat4 m_view_mat;
        glm::mat4 m_projection_mat;
};

#endif