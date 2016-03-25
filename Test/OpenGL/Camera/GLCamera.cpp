#include "GLCamera.h"

#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp >


const float Camera::DEFAULT_ROTATION_SPEED = 0.3f;
const float Camera::DEFAULT_FOVX = 90.0f;
const float Camera::DEFAULT_ZNEAR = 0.1f;
const float Camera::DEFAULT_ZFAR = 1000.0f;

const float Camera::DEFAULT_ORBIT_MIN_ZOOM = DEFAULT_ZNEAR + 1.0f;
const float Camera::DEFAULT_ORBIT_MAX_ZOOM = DEFAULT_ZFAR * 0.5f;

const float Camera::DEFAULT_ORBIT_OFFSET_DISTANCE = DEFAULT_ORBIT_MIN_ZOOM +
	(DEFAULT_ORBIT_MAX_ZOOM - DEFAULT_ORBIT_MIN_ZOOM) * 0.25f;

const glm::vec3 Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const glm::vec3 Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const glm::vec3 Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera()
{
	this->camera_behavior_ = CAMERA_FIRST_PERSON;
	this->prefer_targetYaxis_orbiting_ = true;

	pitch_accum_degrees_ = 0.0f;
	pitch_accum_degrees_saved_ = 0.0f;

	this->rotate_speed_ = DEFAULT_ROTATION_SPEED;
	this->fovy_ = DEFAULT_FOVX;
	this->aspect_ = 0.0f;
	this->znear_ = DEFAULT_ZNEAR;
	this->zfar_ = DEFAULT_ZFAR;   

	this->orbit_min_zoom_ = DEFAULT_ORBIT_MIN_ZOOM;
	this->orbit_max_zoom_ = DEFAULT_ORBIT_MAX_ZOOM;
	this->orbit_offset_distance_ = DEFAULT_ORBIT_OFFSET_DISTANCE;

	this->eye_		= glm::vec3(0.0f, 0.0f, 0.0f);
	this->eye_saved_= glm::vec3(0.0f, 0.0f, 0.0f);
	this->target_	= glm::vec3(0.0f, 0.0f, 0.0f);
	this->x_axis_	= glm::vec3(1.0f, 0.0f, 0.0f);
	this->y_axis_	= glm::vec3(0.0f, 1.0f, 0.0f);
	this->target_y_axis_= glm::vec3(0.0f, 1.0f, 0.0f);
	this->z_axis_	= glm::vec3(0.0f, 0.0f, 1.0f);
	this->view_dir_	= glm::vec3(0.0f, 0.0f, -1.0f);

	this->acceletation_ = glm::vec3(0.0f, 0.0f, 0.0f);
	this->velocity_current_ = glm::vec3(0.0f, 0.0f, 0.0f);
	this->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);

	this->orientation_ = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
	this->orientation_saved_ = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );

	this->viewMatrix_ = glm::mat4( 1.0f );
	this->projectionMatrix_ = glm::mat4( 1.0f );
	
}

Camera::~Camera()
{

}

void Camera::setPerspective(const float fovy, const float aspectRatio, const float near/*=0.1f*/, const float far/*=1000.0f*/)
{
	this->fovy_ = fovy;
	this->aspect_ = aspectRatio;
	this->znear_ = near;
	this->zfar_	 = far;

	this->projectionMatrix_ = glm::perspective( fovy, aspectRatio, near, far );
}

void Camera::lookAt( const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
{
	this->eye_ = eye;
	this->target_ = target;
	
	this->z_axis_ = eye - target;
	this->z_axis_ = glm::normalize( this->z_axis_ );
	this->view_dir_ = - z_axis_;
	this->x_axis_ = glm::normalize( glm::cross( up, this->z_axis_ ) );
	this->y_axis_ = glm::normalize( glm::cross( this->z_axis_, this->x_axis_ ) );

	this->viewMatrix_ = glm::lookAt( eye, target, up );
	//this->orientation_ = glm::toQuat( this->viewMatrix_ );
}


void Camera::move(const float dx, const float dy, const float dz)
{
	glm::mat4 m_t = glm::translate( this->viewMatrix_, glm::vec3( dx, dy, dz ) );
	this->viewMatrix_ = m_t;
}

void Camera::move(const glm::vec3& direction, const glm::vec3& amount)
{
	glm::mat4 m_t = glm::translate( this->viewMatrix_, direction );
	this->viewMatrix_ = m_t;
}

void Camera::zoom(const float amount)
{
	glm::mat4 m_t = glm::scale( this->viewMatrix_, glm::vec3( amount ) );
	this->viewMatrix_ = m_t;
}

void Camera::rotate(const float yaw, const float pitch, const float roll)
{
	float r_yaw	= glm::radians( yaw );
	float r_pitch	= glm::radians( pitch );
	float r_roll	= glm::radians( roll );

	glm::mat4 m_t = glm::yawPitchRoll( r_yaw, r_pitch, r_roll );
	
	this->viewMatrix_ = m_t * viewMatrix_;
}
