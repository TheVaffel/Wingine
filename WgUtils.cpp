#define WG_UTILS_MATRIX_LIB_FLATALG

#include <WgUtils.hpp>


namespace wgut {

  Camera::Camera(float horizontalFOVRadians, float invAspect, float near, float far){
    view = Matrix4(FLATALG_MATRIX_IDENTITY);
    // projection = flatalg::projection(horizontalFOVRadians, invAspect, near, far);
    projection = Matrix4(FLATALG_MATRIX_PROJECTION, horizontalFOVRadians, invAspect, near, far);
    altered = true;
  }

  void Camera::setPosition(const Vector3& v){
    view(0, 3) = -Vector3(view(0, 0), view(0, 1), view(0, 2))*v;
    view(1, 3) = -Vector3(view(1, 0), view(1, 1), view(1, 2))*v;
    view(2, 3) = -Vector3(view(2, 0), view(2, 1), view(2, 2))*v;
    altered = true;
  }

  void Camera::setLookAt(const Vector3& pos,
			 const Vector3& target,
			 const Vector3& up){
    // view = flatalg::lookAt(pos, target, up);
    view = Matrix4(FLATALG_MATRIX_LOOK_AT, pos, target, up);
    altered = true;
  }

  void Camera::setLookDirection(float rightAngle, float downAngle,
				const Vector3& up){
    float cd = cos(-downAngle);
    Vector3 dir(sin(-rightAngle) * cd, sin(-downAngle), cos(-rightAngle) * cd);
    Vector3 right = cross(dir, up).normalized();
    Vector3 viewUp = cross(right, dir);

    Vector3 pos = (~view.submatrix<3, 3>(0, 0))*-Vector3(view(0, 3), view(1, 3), view(2, 3));
  
    view = Matrix4(right.x(), right.y(), right.z(), 0.f,
			viewUp.x(), viewUp.y(), viewUp.z(), 0.f,
			-dir.x(), -dir.y(), -dir.z(), 0.f,
			0.f, 0.f, 0.f, 1.f);
    setPosition(pos);
  }

  Matrix4 Camera::getRenderMatrix(){
    return ~getTransformMatrix();
  }

  Matrix4 Camera::getTransformMatrix(){
    if(altered){
      total = clip*projection*view;
      altered = false;
    }

    return total;
  }

  Matrix4 Camera::getViewMatrix(){
    return view;
  }

  Vector3 Camera::getForwardVector() {
    return Vector3(-view(2, 0), -view(2, 1), -view(2, 2));
  }

  Vector3 Camera::getRightVector() {
    return Vector3(view(0, 0), view(0, 1), view(0, 2));
  }

  Vector3 Camera::getUpVector() {
    return Vector3(view(1, 0), view(1, 1), view(1, 2));
  }

  Vector3 Camera::getPosition() {
    return (~view.submatrix<3, 3>(0, 0))*-Vector3(view(0, 3), view(1, 3), view(2, 3));
  }
  
};
