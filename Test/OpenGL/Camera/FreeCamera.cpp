#include "FreeCamera.h"
#include <glm/gtx/euler_angles.hpp>

CFreeCamera::CFreeCamera()
{
	translation =glm::vec3(0); 
	speed = 0.5f; // 0.5 m/s
}


CFreeCamera::~CFreeCamera(void)
{
}
 
void CFreeCamera::update() {
	glm::mat4 R = glm::yawPitchRoll(yaw_,pitch_,roll_); 
	position_+=translation;

	//set this when no movement decay is needed
	//translation=glm::vec3(0); 

	look_ = glm::vec3(R*glm::vec4(0,0,1,0));	
	up_   = glm::vec3(R*glm::vec4(0,1,0,0));
	right_ = glm::cross(look_, up_);

	glm::vec3 tgt  = position_+look_;
	view_matrix = glm::lookAt(position_, tgt, up_); 
}




void CFreeCamera::Walk(const float dt) {
	translation += (look_*speed*dt);
	update();
}

void CFreeCamera::Strafe(const float dt) {
	translation += (right_*speed*dt);
	update();
}

void CFreeCamera::Lift(const float dt) {
	translation += (up_*speed*dt);
	update();
}
 
void CFreeCamera::SetTranslation(const glm::vec3& t) {
	translation = t;
	update();
}

glm::vec3 CFreeCamera::GetTranslation() const {
	return translation;
}

void CFreeCamera::SetSpeed(const float s) {
	speed = s;
}

const float CFreeCamera::GetSpeed() const {
	return speed;
}