#ifndef __CAMERA_CPP
#define __CAMERA_CPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
	enum CameraBehavior
	{
		CAMERA_FIRST_PERSON,
		CAMERA_SPECTATOR,
		CAMERA_FLIGHT,
		CAMERA_ORBIT
	};

public:
	Camera();
	~Camera();
	

//--------------perspective͸��ͶӰ--------------------
	void setPerspective( const float fovy, const float aspectRatio, const float near=0.1f, const float far=1000.0f );
	
//-----------------------------------------------------
//--------------��ͼ�任-------------------------------
	void lookAt( const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up );
	void lookAt( const glm::vec3& target );

	//��һ���ƶ���ȷ��������;
	void move( const glm::vec3& direction, const glm::vec3& amount );
	void move( const float dx, const float dy, const float dz );
	//�ڶ����ƶ������ݳ��ٶȣ�ʱ�䣬���ٶ�,ţ�ٵڶ�����;
	void updatePosition( const glm::vec3& direction, float elapsedTimeSec );

	void zoom( const float amount );

	void rotate( const float yaw, const float pitch, const float roll );
	void rotateSmoothly(const float yaw, const float pitch, const float roll );
	void resetRoll();

//---------------get----------------------
	const glm::mat4& getProjectionMatrix() const { return this->projectionMatrix_; }
	const glm::mat4& getViewMatrix() const { return this->viewMatrix_; }

	const glm::vec3& getAcceleration() const { return this->acceletation_; }

	Camera::CameraBehavior Camera::getBehavior() const { return this->camera_behavior_; }
//----------------set---------------------
	void setBehavior( CameraBehavior newBehavior );
	void setAcceleration( const glm::vec3& acceleration );
	void setCurrentVelocity( const glm::vec3& currentVelocity );
	void setCurrentVelocity( float x, float y, float z );
	void setOrbitMaxZoom( float orbitMaxZoom );
	void setOrbitMinZoom( float orbitMinZoom );

	
private:
	void rotateFirstPerson( float yaw, float pitch );
	void rotateFlight(		float yaw, float pitch, float roll );
	void rotateOrbit(		float yaw, float pitch, float roll );

	void updateVelocity( const glm::vec3& direction, float elapsedTimeSec );
	void updateViewMatrix();

private:

	static const float DEFAULT_ROTATION_SPEED;
	static const float DEFAULT_FOVX;
	static const float DEFAULT_ZNEAR;
	static const float DEFAULT_ZFAR;
	static const float DEFAULT_ORBIT_MIN_ZOOM;
	static const float DEFAULT_ORBIT_MAX_ZOOM;
	static const float DEFAULT_ORBIT_OFFSET_DISTANCE;
	static const glm::vec3 WORLD_XAXIS;
	static const glm::vec3 WORLD_YAXIS;
	static const glm::vec3 WORLD_ZAXIS;

private:

//��ͼ;
	glm::vec3 eye_;
	glm::vec3 eye_saved_;
	glm::vec3 target_;
	glm::vec3 target_y_axis_;
	//glm::vec3 up_;
	glm::mat4 viewMatrix_;

	glm::quat orientation_;
	glm::quat orientation_saved_;

	float pitch_accum_degrees_;
	float pitch_accum_degrees_saved_;
	float rotate_speed_;


	glm::vec3 x_axis_;
	glm::vec3 y_axis_;
	glm::vec3 z_axis_;

	glm::vec3 view_dir_;

//ͶӰ--perspective;
	float fovy_;
	float aspect_;
	float znear_;
	float zfar_;
	glm::mat4 projectionMatrix_;

//����������;
	CameraBehavior camera_behavior_;	//enum;

	glm::vec3 acceletation_;
	bool prefer_targetYaxis_orbiting_;	//orbitģʽ������Y����ת;
	glm::vec3 velocity_current_;
	glm::vec3 velocity_;


	float orbit_min_zoom_;
	float orbit_max_zoom_;
	float orbit_offset_distance_;
	
	float first_person_y_offset_;

};

#endif