#ifndef __CAMERA3_CPP
#define __CAMERA3_CPP

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

//-----------------------------------------------------------------------------
// A general purpose 6DoF (six degrees of freedom) quaternion based camera.
//
// This camera class supports 4 different behaviors:
// first person mode, spectator mode, flight mode, and orbit mode.
//
// First person mode only allows 5DOF (x axis movement, y axis movement, z axis
// movement, yaw, and pitch) and movement is always parallel to the world x-z
// (ground) plane.
//
// Spectator mode is similar to first person mode only movement is along the
// direction the camera is pointing.
// 
// Flight mode supports 6DoF. This is the camera class' default behavior.
//
// Orbit mode rotates the camera around a target position. This mode can be
// used to simulate a third person camera. Orbit mode supports 2 modes of
// operation: orbiting about the target's Y axis, and free orbiting. The former
// mode only allows pitch and yaw. All yaw changes are relative to the target's
// local Y axis. This means that camera yaw changes won't be affected by any
// rolling. The latter mode allows the camera to freely orbit the target. The
// camera is free to pitch, yaw, and roll. All yaw changes are relative to the
// camera's orientation (in space orbiting the target).
//
// This camera class allows the camera to be moved in 2 ways: using fixed
// step world units, and using a supplied velocity and acceleration. The former
// simply moves the camera by the specified amount. To move the camera in this
// way call one of the move() methods. The other way to move the camera
// calculates the camera's displacement based on the supplied velocity,
// acceleration, and elapsed time. To move the camera in this way call the
// updatePosition() method.
//-----------------------------------------------------------------------------

class Camera
{
public:
    enum CameraBehavior
    {
        CAMERA_BEHAVIOR_FIRST_PERSON,
        CAMERA_BEHAVIOR_SPECTATOR,
        CAMERA_BEHAVIOR_FLIGHT,
        CAMERA_BEHAVIOR_ORBIT
    };

    Camera();
    ~Camera();

    void lookAt(const glm::vec3 &target);
    void lookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up);
    void move(float dx, float dy, float dz);
    void move(const glm::vec3 &direction, const glm::vec3 &amount);
    void perspective(float fovx, float aspect, float znear, float zfar);
    void rotate(float headingDegrees, float pitchDegrees, float rollDegrees);
    void rotateSmoothly(float headingDegrees, float pitchDegrees, float rollDegrees);
    void undoRoll();
    void updatePosition(const glm::vec3 &direction, float elapsedTimeSec);
    void zoom(float zoom, float minZoom, float maxZoom);

    // Getter methods.

    const glm::vec3 &getAcceleration() const;
    CameraBehavior getBehavior() const;
    const glm::vec3 &getCurrentVelocity() const;
    const glm::vec3 &getPosition() const;
    float getOrbitMinZoom() const;
    float getOrbitMaxZoom() const;
    float getOrbitOffsetDistance() const;
    float getOrbitPitchMaxDegrees() const;
    float getOrbitPitchMinDegrees() const;
    const glm::quat &getOrientation() const;
    float getRotationSpeed() const;
    const glm::mat4 &getProjectionMatrix() const;
    const glm::vec3 &getVelocity() const;
    const glm::vec3 &getViewDirection() const;
    const glm::mat4 &getViewMatrix() const;
    const glm::mat4 &getViewProjectionMatrix() const;
    const glm::vec3 &getXAxis() const;
    const glm::vec3 &getYAxis() const;
    const glm::vec3 &getZAxis() const;
    bool preferTargetYAxisOrbiting() const;
    
    // Setter methods.

    void setAcceleration(const glm::vec3 &acceleration);
    void setBehavior(CameraBehavior newBehavior);
    void setCurrentVelocity(const glm::vec3 &currentVelocity);
    void setCurrentVelocity(float x, float y, float z);
    void setOrbitMaxZoom(float orbitMaxZoom);
    void setOrbitMinZoom(float orbitMinZoom);
    void setOrbitOffsetDistance(float orbitOffsetDistance);
    void setOrbitPitchMaxDegrees(float orbitPitchMaxDegrees);
    void setOrbitPitchMinDegrees(float orbitPitchMinDegrees);
    void setOrientation(const glm::quat &newOrientation);
    void setPosition(const glm::vec3 &newEye);
    void setPreferTargetYAxisOrbiting(bool preferTargetYAxisOrbiting);
    void setRotationSpeed(float rotationSpeed);
    void setVelocity(const glm::vec3 &velocity);
    void setVelocity(float x, float y, float z);

private:
    void rotateFirstPerson(float headingDegrees, float pitchDegrees);
    void rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees);
    void rotateOrbit(float headingDegrees, float pitchDegrees, float rollDegrees);
    void updateVelocity(const glm::vec3 &direction, float elapsedTimeSec);
    void updateViewMatrix();

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

    CameraBehavior m_behavior;
    bool m_preferTargetYAxisOrbiting;
    float m_accumPitchDegrees;
    float m_savedAccumPitchDegrees;
    float m_rotationSpeed;
    float m_fovx;
    float m_aspectRatio;
    float m_znear;
    float m_zfar;
    float m_orbitMinZoom;
    float m_orbitMaxZoom;
    float m_orbitOffsetDistance;
    float m_firstPersonYOffset;
    glm::vec3 m_eye;
    glm::vec3 m_savedEye;
    glm::vec3 m_target;
    glm::vec3 m_targetYAxis;
    glm::vec3 m_xAxis;
    glm::vec3 m_yAxis;
    glm::vec3 m_zAxis;
    glm::vec3 m_viewDir;
    glm::vec3 m_acceleration;
    glm::vec3 m_currentVelocity;
    glm::vec3 m_velocity;
    glm::quat m_orientation;
    glm::quat m_savedOrientation;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projMatrix;
    glm::mat4 m_viewProjMatrix;
};

//-----------------------------------------------------------------------------

inline const glm::vec3 &Camera::getAcceleration() const
{ return m_acceleration; }

inline Camera::CameraBehavior Camera::getBehavior() const
{ return m_behavior; }

inline const glm::vec3 &Camera::getCurrentVelocity() const
{ return m_currentVelocity; }

inline const glm::vec3 &Camera::getPosition() const
{ return m_eye; }

inline float Camera::getOrbitMinZoom() const
{ return m_orbitMinZoom; }

inline float Camera::getOrbitMaxZoom() const
{ return m_orbitMaxZoom; }

inline float Camera::getOrbitOffsetDistance() const
{ return m_orbitOffsetDistance; }

inline const glm::quat &Camera::getOrientation() const
{ return m_orientation; }

inline float Camera::getRotationSpeed() const
{ return m_rotationSpeed; }

inline const glm::mat4 &Camera::getProjectionMatrix() const
{ return m_projMatrix; }

inline const glm::vec3 &Camera::getVelocity() const
{ return m_velocity; }

inline const glm::vec3 &Camera::getViewDirection() const
{ return m_viewDir; }

inline const glm::mat4 &Camera::getViewMatrix() const
{ return m_viewMatrix; }

inline const glm::mat4 &Camera::getViewProjectionMatrix() const
{ return m_viewProjMatrix; }

inline const glm::vec3 &Camera::getXAxis() const
{ return m_xAxis; }

inline const glm::vec3 &Camera::getYAxis() const
{ return m_yAxis; }

inline const glm::vec3 &Camera::getZAxis() const
{ return m_zAxis; }

inline bool Camera::preferTargetYAxisOrbiting() const
{ return m_preferTargetYAxisOrbiting; }

#endif