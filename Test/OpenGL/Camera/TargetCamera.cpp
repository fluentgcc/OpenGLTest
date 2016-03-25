#define _USE_MATH_DEFINES
#include <cmath>
#include "TargetCamera.h"
#include <iostream>

#include <glm/gtx/euler_angles.hpp>

CTargetCamera::CTargetCamera( void )
{  
	right_ = glm::vec3(1,0,0);
	up_ = glm::vec3(0,1,0);
	look_ = glm::vec3(0,0,-1);
	minRy_ = -60;
	maxRy_ = 60;
	minDistance_ = 1;
	maxDistance_ = 10;
}


CTargetCamera::~CTargetCamera( void )
{
}
 
void CTargetCamera::update() 
{
	 
	glm::mat4 R = glm::yawPitchRoll(yaw_,pitch_,0.0f);
	glm::vec3 T = glm::vec3(0,0,distance_);
	T = glm::vec3(R*glm::vec4(T,0.0f));
	position_ = target_ + T;
	look_ = glm::normalize(target_-position_);
	up_ = glm::vec3(R*glm::vec4(UP,0.0f));
	right_ = glm::cross(look_, up_);
	view_matrix = glm::lookAt(position_, target_, up_); 
}

void CTargetCamera::setTarget( const glm::vec3 tgt ) 
{
	target_ = tgt; 
	distance_ = glm::distance(position_, target_);
	distance_ = std::max(minDistance_, std::min(distance_, maxDistance_));
	/*V = glm::lookAt(position, target, up);

	m_yaw = 0;
	m_pitch = 0;

	if(V[0][0] < 0)
		m_yaw = glm::degrees((float)(M_PI - asinf(-V[2][0])) );
	else
		m_yaw = glm::degrees(asinf(-V[2][0]));

	m_pitch = glm::degrees(asinf(-V[1][2]));  
	*/
}

const glm::vec3 CTargetCamera::getTarget() const 
{
	return target_;
} 

void CTargetCamera::rotate( const float yaw, const float pitch, const float roll ) 
{
 	float p = std::min( std::max(pitch, minRy_), maxRy_);
	CAbstractCamera::rotate(yaw, p, roll); 
}
 
void CTargetCamera::pan(const float dx, const float dy) {
	glm::vec3 X = right_*dx;
	glm::vec3 Y = up_*dy;
	position_ += X + Y;
	  target_ += X + Y;
	update();
}

 
void CTargetCamera::zoom(const float amount) { 
	position_ += look_ * amount;
	distance_ = glm::distance(position_, target_); 
	distance_ = std::max(minDistance_, std::min(distance_, maxDistance_));
	update();
}
 
void CTargetCamera::move(const float dx, const float dy) {
	glm::vec3 X = right_*dx;
	glm::vec3 Y = look_*dy;
	position_ += X + Y;
	  target_ += X + Y;
	update();
}