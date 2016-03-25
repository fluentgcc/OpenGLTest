#include "AbstractCamera.h"  
 

glm::vec3 CAbstractCamera::UP = glm::vec3(0,1,0);

CAbstractCamera::CAbstractCamera(void) 
{ 
	z_near_ = 0.1f;
	z_far_  = 1000;
}


CAbstractCamera::~CAbstractCamera(void)
{
}

void CAbstractCamera::setPerspective( const float fovy, const float aspRatio, const float nr, const float fr ) 
{
	projection_matrix = glm::perspective(fovy, aspRatio, nr, fr); 
	z_near_ = nr;
	z_far_ = fr;
	fov_ = fovy;
	aspect_ratio_ = aspRatio; 
} 

void CAbstractCamera::lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{

}

const glm::mat4 CAbstractCamera::getViewMatrix() const 
{
	return view_matrix;
}

const glm::mat4 CAbstractCamera::getProjectionMatrix() const 
{
	return projection_matrix;
}

const glm::vec3 CAbstractCamera::getPosition() const 
{
	return position_;
}

void CAbstractCamera::setPosition( const glm::vec3& p ) 
{
	position_ = p;
}
  
const float CAbstractCamera::getFOV() const 
{
	return fov_;
} 
void CAbstractCamera::setFOV( const float fovInDegrees ) 
{
	fov_ = fovInDegrees;
	projection_matrix = glm::perspective( fovInDegrees, aspect_ratio_, z_near_, z_far_ ); 
}
const float CAbstractCamera::getAspectRatio() const 
{
	return aspect_ratio_;
}

void CAbstractCamera::rotate( const float y, const float p, const float r ) 
{
	yaw_	=glm::radians( y );
	pitch_	=glm::radians( p );
	roll_	=glm::radians( r );
	update();
}