#ifndef WGUT_CAMERA_HPP
#define WGUT_CAMERA_HPP

#include <FlatAlg.hpp>

namespace wgut {
    class Camera{
        const falg::Mat4 clip = {1.f, 0.f, 0.f, 0.f,
                                 0.f, -1.f, 0.f, 0.f,
                                 0.f, 0.f, 0.5f, 0.5f,
                                 0.f, 0.f, 0.0f, 1.f};
        falg::Mat4 projection, view, total;
        bool altered;
    public:

        Camera(float horizontalFOVRadians = 45.f/180.f*F_PI, float invAspect = 9.0f/16.0f, float near = 0.1f, float far = 100.0f);

        void setPosition(const falg::Vec3& v);

        void setLookAt(const falg::Vec3& pos,
                       const falg::Vec3& target,
                       const falg::Vec3& up);
        void setLookDirection(float rightAngle, float upAngle,
                              const falg::Vec3& up);
  
        falg::Mat4 getRenderMatrix();
        falg::Mat4 getTransformMatrix();
        falg::Mat4 getViewMatrix();

        falg::Vec3 getForwardVector();
        falg::Vec3 getRightVector();
        falg::Vec3 getUpVector();

        falg::Vec3 getPosition();
    };
};

#endif // WGUT_CAMERA_HPP
