#pragma once
#include "abstractcamera.h"

class CTargetCamera :
	public CAbstractCamera
{
public:
	CTargetCamera(void);
	~CTargetCamera(void);

	void update();
	void rotate( const float yaw, const float pitch, const float roll );
	 
	void setTarget( const glm::vec3 tgt );
	const glm::vec3 getTarget() const;

	void pan(  const float dx, const float dy );
	void zoom( const float amount );
	void move( const float dx, const float dz );

protected:
	glm::vec3 target_;   
	 
	float minRy_, maxRy_;
	float distance_;
	float minDistance_, maxDistance_;

};

