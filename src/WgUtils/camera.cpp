#include "WgUtils/camera.hpp"

namespace wgut {
  
    Camera::Camera(float horizontalFOVRadians, float invAspect, float near, float far){

        view = falg::Mat4(falg::FLATALG_MATRIX_IDENTITY);
        projection = falg::Mat4(falg::FLATALG_MATRIX_PROJECTION, horizontalFOVRadians, invAspect, near, far);
        altered = true;
    }

    void Camera::setPosition(const falg::Vec3& v){
        view(0, 3) = -falg::Vec3(view(0, 0), view(0, 1), view(0, 2))*v;
        view(1, 3) = -falg::Vec3(view(1, 0), view(1, 1), view(1, 2))*v;
        view(2, 3) = -falg::Vec3(view(2, 0), view(2, 1), view(2, 2))*v;
        altered = true;
    }

    void Camera::setLookAt(const falg::Vec3& pos,
                           const falg::Vec3& target,
                           const falg::Vec3& up){
        view = falg::Mat4(falg::FLATALG_MATRIX_LOOK_AT, pos, target, up);
        altered = true;
    }

    void Camera::setLookDirection(float rightAngle, float downAngle,
                                  const falg::Vec3& up){
        float cd = cos(-downAngle);
        falg::Vec3 dir(sin(-rightAngle) * cd, sin(-downAngle), cos(-rightAngle) * cd);
        falg::Vec3 right = cross(dir, up).normalized();
        falg::Vec3 viewUp = cross(right, dir);

        falg::Vec3 pos = (~view.submatrix<3, 3>(0, 0))*-falg::Vec3(view(0, 3), view(1, 3), view(2, 3));
  
        view = falg::Mat4(right.x(), right.y(), right.z(), 0.f,
                          viewUp.x(), viewUp.y(), viewUp.z(), 0.f,
                          -dir.x(), -dir.y(), -dir.z(), 0.f,
                          0.f, 0.f, 0.f, 1.f);
        setPosition(pos);
    }

    falg::Mat4 Camera::getRenderMatrix(){
        return ~getTransformMatrix();
    }

    falg::Mat4 Camera::getTransformMatrix(){
        if(altered){
            total = clip*projection*view;
            altered = false;
        }

        return total;
    }

    falg::Mat4 Camera::getViewMatrix(){
        return view;
    }

    falg::Vec3 Camera::getForwardVector() {
        return falg::Vec3(-view(2, 0), -view(2, 1), -view(2, 2));
    }

    falg::Vec3 Camera::getRightVector() {
        return falg::Vec3(view(0, 0), view(0, 1), view(0, 2));
    }

    falg::Vec3 Camera::getUpVector() {
        return falg::Vec3(view(1, 0), view(1, 1), view(1, 2));
    }

    falg::Vec3 Camera::getPosition() {
        return (~view.submatrix<3, 3>(0, 0))*-falg::Vec3(view(0, 3), view(1, 3), view(2, 3));
    }
};
