#pragma once

#include <glm/gtc/matrix_transform.hpp>

class CAbstractCamera
{
public:
	CAbstractCamera(void);
	~CAbstractCamera(void);
	 
	void setPerspective(const float fovy, const float aspectRatio, const float near=0.1f, const float far=1000.0f);

	void lookAt( const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up );
	
	virtual void update() = 0;
	virtual void rotate(const float yaw, const float pitch, const float roll); 

	const glm::mat4 getViewMatrix() const;
	const glm::mat4 getProjectionMatrix() const;

	void setPosition(const glm::vec3& v);
	const glm::vec3 getPosition() const;
	 

	void setFOV(const float fov);
	const float getFOV() const;
	const float getAspectRatio() const; 
	
	//frustum points
	glm::vec3 farPts[4];
	glm::vec3 nearPts[4];

protected:	
	
	static glm::vec3 UP;

	float yaw_, pitch_, roll_, fov_, aspect_ratio_, z_near_, z_far_;

	glm::vec3 look_;
	glm::vec3 up_;
	glm::vec3 right_; 
	glm::vec3 position_;
	glm::mat4 view_matrix; //view matrix
	glm::mat4 projection_matrix; //projection matrix

	//Frsutum planes

};

