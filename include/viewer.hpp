#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__

#include <memory>
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  // radians


#include "settings.hpp"

class Movement;

class Viewer {
    public:
        Viewer(const glm::vec3& position, const glm::vec3& center);
        ~Viewer();
        const glm::mat4& getViewMatrix() const;
        const glm::mat4& getProjectionMatrix() const;

        const glm::vec3& getPosition() const;
        const glm::vec3& getSightDirection() const;

        void setPosition(const glm::vec3& position);
        void applyMovement(std::unique_ptr<Movement> movement);

        void update(float time);

        static std::unique_ptr<Viewer> createPerspectiveViewer(const glm::vec3& position,
            const glm::vec3& center,
            float fov = glm::radians(45.f),
            float ratio = static_cast<float>(SCR_WIDTH) / SCR_HEIGHT,
            float zNear = 0.1f,
            float zFar = 100.f  
        );
        static std::unique_ptr<Viewer> createOrthoViewer(const glm::vec3& position,
            const glm::vec3& center,
            float halfSize = 10.f,
            float zNear = 0.1f,
            float zFar = 100.f 
        );

    private:
        glm::mat4 m_view_mat;
        glm::mat4 m_projection_mat;

        glm::vec3 m_position;
        glm::vec3 m_center;

        std::unique_ptr<Movement> m_movement;
};

class Movement {
    public:
        Movement() {}
        virtual ~Movement() {}
        virtual void update(Viewer& viewer, float time) = 0;
};
class CircleMovement: public Movement {
    public:
        CircleMovement(const glm::vec3& center, float height, float radius);
        ~CircleMovement();

        void setCenter(const glm::vec3& center);

        void update(Viewer& viewer, float time);
    private:
        glm::vec3 m_center;
        float m_radius;
        float m_height;
};

#endif