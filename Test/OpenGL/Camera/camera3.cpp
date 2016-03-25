#include <algorithm>
#include "camera3.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp >
#include <glm/trigonometric.hpp>

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


bool closeEnough( float f1, float f2 )
{
	return fabs( f1 - f2 ) < 1e-10;
}

Camera::Camera()
{
    m_behavior = CAMERA_BEHAVIOR_ORBIT;
    m_preferTargetYAxisOrbiting = true;
    
    m_accumPitchDegrees = 0.0f;
    m_savedAccumPitchDegrees = 0.0f;
    
    m_rotationSpeed = DEFAULT_ROTATION_SPEED;
    m_fovx = DEFAULT_FOVX;
    m_aspectRatio = 0.0f;
    m_znear = DEFAULT_ZNEAR;
    m_zfar = DEFAULT_ZFAR;   
    
    m_orbitMinZoom = DEFAULT_ORBIT_MIN_ZOOM;
    m_orbitMaxZoom = DEFAULT_ORBIT_MAX_ZOOM;
    m_orbitOffsetDistance = DEFAULT_ORBIT_OFFSET_DISTANCE;
        
	m_eye			=glm::vec3(0.0f, 0.0f, 0.0f);
	m_savedEye		=glm::vec3(0.0f, 0.0f, 0.0f);
	m_target		=glm::vec3(0.0f, 0.0f, 0.0f);
	m_xAxis			=glm::vec3(1.0f, 0.0f, 0.0f);
	m_yAxis			=glm::vec3(0.0f, 1.0f, 0.0f);
    m_targetYAxis	=glm::vec3(0.0f, 1.0f, 0.0f);
    m_zAxis			=glm::vec3(0.0f, 0.0f, 1.0f);
    m_viewDir		=glm::vec3(0.0f, 0.0f, -1.0f);
        
    m_acceleration	  = glm::vec3(0.0f, 0.0f, 0.0f);
    m_currentVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    m_velocity		  = glm::vec3(0.0f, 0.0f, 0.0f);

    m_orientation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f);
    m_savedOrientation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f);

	m_viewMatrix	= glm::mat4(1.0f);
	m_projMatrix	= glm::mat4(1.0f);
	m_viewProjMatrix= glm::mat4(1.0f);
}

Camera::~Camera()
{
}

void Camera::lookAt(const glm::vec3 &target)
{
    lookAt(m_eye, target, m_yAxis);
}

void Camera::lookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up)
{
    m_eye = eye;
    m_target = target;

    m_zAxis = eye - target;
    m_zAxis =glm::normalize( m_zAxis );

    m_viewDir = -m_zAxis;

    m_xAxis = glm::normalize( glm::cross( up, m_zAxis ) );

    m_yAxis = glm::normalize( glm::cross( m_zAxis, m_xAxis ) );

    
    m_viewMatrix[0][0] = m_xAxis.x;
    m_viewMatrix[1][0] = m_xAxis.y;
    m_viewMatrix[2][0] = m_xAxis.z;
    m_viewMatrix[3][0] = -glm::dot(m_xAxis, eye);

    m_viewMatrix[0][1] = m_yAxis.x;
    m_viewMatrix[1][1] = m_yAxis.y;
    m_viewMatrix[2][1] = m_yAxis.z;
    m_viewMatrix[3][1] = -glm::dot(m_yAxis, eye);

    m_viewMatrix[0][2] = m_zAxis.x;
    m_viewMatrix[1][2] = m_zAxis.y;
    m_viewMatrix[2][2] = m_zAxis.z;    
    m_viewMatrix[3][2] = -glm::dot(m_zAxis, eye);

    // Extract the pitch angle from the view matrix.
 	m_accumPitchDegrees = glm::degrees(  glm::asin( m_viewMatrix[1][2] ) );
    
    m_orientation =  glm::toQuat( m_viewMatrix );
	updateViewMatrix();
}

void Camera::move(float dx, float dy, float dz)
{
    // Moves the camera by dx world units to the left or right; dy
    // world units upwards or downwards; and dz world units forwards
    // or backwards.

    if (m_behavior == CAMERA_BEHAVIOR_ORBIT)
    {
        // Orbiting camera is always positioned relative to the
        // target position. See updateViewMatrix().
        return;
    }

    glm::vec3 eye = m_eye;
    glm::vec3 forwards;

    if (m_behavior == CAMERA_BEHAVIOR_FIRST_PERSON)
    {
        // Calculate the forwards direction. Can't just use the camera's local
        // z axis as doing so will cause the camera to move more slowly as the
        // camera's view approaches 90 degrees straight up and down.

        forwards = glm::cross( WORLD_YAXIS, m_xAxis );
        forwards = glm::normalize( forwards );
    }
    else
    {
        forwards = m_viewDir;
    }

    eye += m_xAxis * dx;
    eye += WORLD_YAXIS * dy;
    eye += forwards * dz;

    setPosition(eye);
}

void Camera::move(const glm::vec3 &direction, const glm::vec3 &amount)
{
    // Moves the camera by the specified amount of world units in the specified
    // direction in world space.

    if (m_behavior == CAMERA_BEHAVIOR_ORBIT)
    {
        // Orbiting camera is always positioned relative to the
        // target position. See updateViewMatrix().
        return;
    }

    m_eye.x += direction.x * amount.x;
    m_eye.y += direction.y * amount.y;
    m_eye.z += direction.z * amount.z;

    updateViewMatrix();
}

void Camera::perspective(float fovx, float aspect, float znear, float zfar)
{
    // Construct a projection matrix based on the horizontal field of view
    // 'fovx' rather than the more traditional vertical field of view 'fovy'.

    float e = 1.0f / glm::tan( glm::radians( fovx ) / 2.0f );
    float aspectInv = 1.0f / aspect;
    float fovy = 2.0f * atanf(aspectInv / e);
    float xScale = 1.0f / tanf(0.5f * fovy);
    float yScale = xScale / aspectInv;

    m_projMatrix[0][0] = xScale;
    m_projMatrix[0][1] = 0.0f;
    m_projMatrix[0][2] = 0.0f;
    m_projMatrix[0][3] = 0.0f;

    m_projMatrix[1][0] = 0.0f;
    m_projMatrix[1][1] = yScale;
    m_projMatrix[1][2] = 0.0f;
    m_projMatrix[1][3] = 0.0f;

    m_projMatrix[2][0] = 0.0f;
    m_projMatrix[2][1] = 0.0f;
    m_projMatrix[2][2] = (zfar + znear) / (znear - zfar);
    m_projMatrix[2][3] = -1.0f;

    m_projMatrix[3][0] = 0.0f;
    m_projMatrix[3][1] = 0.0f;
    m_projMatrix[3][2] = (2.0f * zfar * znear) / (znear - zfar);
    m_projMatrix[3][3] = 0.0f;

    m_viewProjMatrix = m_viewMatrix * m_projMatrix;
    
    m_fovx = fovx;
    m_aspectRatio = aspect;
    m_znear = znear;
    m_zfar = zfar;
}

void Camera::rotate(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Rotates the camera based on its current behavior.
    // Note that not all behaviors support rolling.

    pitchDegrees = -pitchDegrees;
    headingDegrees = -headingDegrees;
    rollDegrees = -rollDegrees;

    switch (m_behavior)
    {
    default:
        break;

    case CAMERA_BEHAVIOR_FIRST_PERSON:
    case CAMERA_BEHAVIOR_SPECTATOR:
        rotateFirstPerson(headingDegrees, pitchDegrees);
        break;

    case CAMERA_BEHAVIOR_FLIGHT:
        rotateFlight(headingDegrees, pitchDegrees, rollDegrees);
        break;

    case CAMERA_BEHAVIOR_ORBIT:
        rotateOrbit(headingDegrees, pitchDegrees, rollDegrees);
        break;
    }

    updateViewMatrix();
}

void Camera::rotateSmoothly(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // This method applies a scaling factor to the rotation angles prior to
    // using these rotation angles to rotate the camera. This method is usually
    // called when the camera is being rotated using an input device (such as a
    // mouse or a joystick). 

    headingDegrees *= m_rotationSpeed;
    pitchDegrees *= m_rotationSpeed;
    rollDegrees *= m_rotationSpeed;

    rotate(headingDegrees, pitchDegrees, rollDegrees);
}

void Camera::undoRoll()
{
    // Undo any camera rolling by leveling the camera. When the camera is
    // orbiting this method will cause the camera to become level with the
    // orbit target.

    if (m_behavior == CAMERA_BEHAVIOR_ORBIT)
        lookAt(m_eye, m_target, m_targetYAxis);
    else
        lookAt(m_eye, m_eye + m_viewDir, WORLD_YAXIS);
}

void Camera::updatePosition(const glm::vec3 &direction, float elapsedTimeSec)
{
    // Moves the camera using Newton's second law of motion. Unit mass is
    // assumed here to somewhat simplify the calculations. The direction vector
    // is in the range [-1,1].


	float length2 = glm::dot( m_currentVelocity,m_currentVelocity );
    if ( length2 < 1e-10 )
    {
        // Only move the camera if the velocity vector is not of zero length.
        // Doing this guards against the camera slowly creeping around due to
        // floating point rounding errors.

        glm::vec3 displacement = (m_currentVelocity * elapsedTimeSec) + (0.5f * m_acceleration * elapsedTimeSec * elapsedTimeSec);

        // Floating point rounding errors will slowly accumulate and cause the
        // camera to move along each axis. To prevent any unintended movement
        // the displacement vector is clamped to zero for each direction that
        // the camera isn't moving in. Note that the updateVelocity() method
        // will slowly decelerate the camera's velocity back to a stationary
        // state when the camera is no longer moving along that direction. To
        // account for this the camera's current velocity is also checked.

        if (direction.x == 0.0f && closeEnough(m_currentVelocity.x, 0.0f))
            displacement.x = 0.0f;

        if (direction.y == 0.0f && closeEnough(m_currentVelocity.y, 0.0f))
            displacement.y = 0.0f;

        if (direction.z == 0.0f && closeEnough(m_currentVelocity.z, 0.0f))
            displacement.z = 0.0f;

        move( displacement.x, displacement.y, displacement.z );
    }

    // Continuously update the camera's velocity vector even if the camera
    // hasn't moved during this call. When the camera is no longer being moved
    // the camera is decelerating back to its stationary state.

    updateVelocity(direction, elapsedTimeSec);
}

void Camera::zoom(float zoom, float minZoom, float maxZoom)
{
    if (m_behavior == CAMERA_BEHAVIOR_ORBIT)
    {
        // Moves the camera closer to or further away from the orbit
        // target. The zoom amounts are in world units.

        m_orbitMaxZoom = maxZoom;
        m_orbitMinZoom = minZoom;

        glm::vec3 offset = m_eye - m_target;

        m_orbitOffsetDistance = glm::length2( offset );
        offset = glm::normalize( offset );
        m_orbitOffsetDistance += zoom;
        m_orbitOffsetDistance = std::min(std::max(m_orbitOffsetDistance, minZoom), maxZoom);

        offset *= m_orbitOffsetDistance;
        m_eye = offset + m_target;
        
        updateViewMatrix();
    }
    else
    {
        // For the other behaviors zoom is interpreted as changing the
        // horizontal field of view. The zoom amounts refer to the horizontal
        // field of view in degrees.

        zoom = std::min(std::max(zoom, minZoom), maxZoom);
        perspective(zoom, m_aspectRatio, m_znear, m_zfar);
    }
}

void Camera::setAcceleration(const glm::vec3 &acceleration)
{
    m_acceleration = acceleration;
}

void Camera::setBehavior(CameraBehavior newBehavior)
{
    // Switch to a new camera mode (i.e., behavior).
    // This method is complicated by the fact that it tries to save the current
    // behavior's state prior to making the switch to the new camera behavior.
    // Doing this allows seamless switching between camera behaviors.

    CameraBehavior prevBehavior = m_behavior;

    if (prevBehavior == newBehavior)
        return;

    m_behavior = newBehavior;

    switch (newBehavior)
    {
    case CAMERA_BEHAVIOR_FIRST_PERSON:
        switch (prevBehavior)
        {
        default:
            break;

        case CAMERA_BEHAVIOR_FLIGHT:
            m_eye.y = m_firstPersonYOffset;
            updateViewMatrix();
            break;

        case CAMERA_BEHAVIOR_SPECTATOR:
            m_eye.y = m_firstPersonYOffset;
            updateViewMatrix();
            break;

        case CAMERA_BEHAVIOR_ORBIT:
            m_eye.x = m_savedEye.x;
            m_eye.z = m_savedEye.z;
            m_eye.y = m_firstPersonYOffset;
            m_orientation = m_savedOrientation;
            m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
            break;
        }

        undoRoll();
        break;

    case CAMERA_BEHAVIOR_SPECTATOR:
        switch (prevBehavior)
        {
        default:
            break;

        case CAMERA_BEHAVIOR_FLIGHT:
            updateViewMatrix();
            break;

        case CAMERA_BEHAVIOR_ORBIT:
            m_eye = m_savedEye;
            m_orientation = m_savedOrientation;
            m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
            break;
        }

        undoRoll();
        break;
    
    case CAMERA_BEHAVIOR_FLIGHT:
        if (prevBehavior == CAMERA_BEHAVIOR_ORBIT)
        {
            m_eye = m_savedEye;
            m_orientation = m_savedOrientation;
            m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
        }
        else
        {
            m_savedEye = m_eye;
            updateViewMatrix();
        }
        break;
    
    case CAMERA_BEHAVIOR_ORBIT:
        if (prevBehavior == CAMERA_BEHAVIOR_FIRST_PERSON)
            m_firstPersonYOffset = m_eye.y;

        m_savedEye = m_eye;
        m_savedOrientation = m_orientation;
        m_savedAccumPitchDegrees = m_accumPitchDegrees;
        
        m_targetYAxis = m_yAxis;

        glm::vec3 newEye = m_eye + m_zAxis * m_orbitOffsetDistance;
        glm::vec3 newTarget = m_eye;
        
        lookAt(newEye, newTarget, m_targetYAxis);
        break;
    }
}

void Camera::setCurrentVelocity(const glm::vec3 &currentVelocity)
{
    m_currentVelocity = currentVelocity;
}

void Camera::setCurrentVelocity(float x, float y, float z)
{
    m_currentVelocity = glm::vec3(x, y, z);
}

void Camera::setOrbitMaxZoom(float orbitMaxZoom)
{
    m_orbitMaxZoom = orbitMaxZoom;
}

void Camera::setOrbitMinZoom(float orbitMinZoom)
{
    m_orbitMinZoom = orbitMinZoom;
}

void Camera::setOrbitOffsetDistance(float orbitOffsetDistance)
{
    m_orbitOffsetDistance = orbitOffsetDistance;
}

void Camera::setOrientation(const glm::quat &newOrientation)
{
    glm::mat4 m = glm::toMat4( newOrientation );

    // Store the pitch for this new orientation.
    // First person and spectator behaviors limit pitching to
    // 90 degrees straight up and down.

    m_accumPitchDegrees = glm::degrees( glm::asin( m[1][2] ) );

    // First person and spectator behaviors don't allow rolling.
    // Negate any rolling that might be encoded in the new orientation.

    m_orientation = newOrientation;

    if (m_behavior == CAMERA_BEHAVIOR_FIRST_PERSON || m_behavior == CAMERA_BEHAVIOR_SPECTATOR)
        lookAt(m_eye, m_eye + m_viewDir, WORLD_YAXIS);

    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3 &newEye)
{
    m_eye = newEye;
    updateViewMatrix();
}

void Camera::setPreferTargetYAxisOrbiting(bool preferTargetYAxisOrbiting)
{
    // Determines the behavior of Y axis rotations when the camera is
    // orbiting a target. When preferTargetYAxisOrbiting is true all
    // Y axis rotations are about the orbit target's local Y axis.
    // When preferTargetYAxisOrbiting is false then the camera's
    // local Y axis is used instead.

    m_preferTargetYAxisOrbiting = preferTargetYAxisOrbiting;

    if (m_preferTargetYAxisOrbiting)
        undoRoll();
}

void Camera::setRotationSpeed(float rotationSpeed)
{
    // This is just an arbitrary value used to scale rotations
    // when rotateSmoothly() is called.

    m_rotationSpeed = rotationSpeed;
}

void Camera::setVelocity(const glm::vec3 &velocity)
{
    m_velocity = velocity;
}

void Camera::setVelocity(float x, float y, float z)
{
    m_velocity = glm::vec3(x, y, z);
}

void Camera::rotateFirstPerson(float headingDegrees, float pitchDegrees)
{
    // Implements the rotation logic for the first person style and
    // spectator style camera behaviors. Roll is ignored.

    m_accumPitchDegrees += pitchDegrees;

    if (m_accumPitchDegrees > 90.0f)
    {
        pitchDegrees = 90.0f - (m_accumPitchDegrees - pitchDegrees);
        m_accumPitchDegrees = 90.0f;
    }

    if (m_accumPitchDegrees < -90.0f)
    {
        pitchDegrees = -90.0f - (m_accumPitchDegrees - pitchDegrees);
        m_accumPitchDegrees = -90.0f;
    }
   
    glm::quat rot;

    // Rotate camera about the world y axis.
    // Note the order the quaternions are multiplied. That is important!
    if (headingDegrees != 0.0f)
    {
        //rot.fromAxisAngle(WORLD_YAXIS, headingDegrees);
		float halfTheta = glm::radians( headingDegrees ) * 0.5f;
		float s = sinf(halfTheta);
		rot = glm::quat( glm::cos( halfTheta ),  WORLD_YAXIS.x * s,  WORLD_YAXIS.y * s, WORLD_YAXIS.z * s );

        m_orientation = rot * m_orientation;
    }

    // Rotate camera about its local x axis.
    // Note the order the quaternions are multiplied. That is important!
    if (pitchDegrees != 0.0f)
    {
        //rot.fromAxisAngle(WORLD_XAXIS, pitchDegrees);

		float halfTheta = glm::radians( pitchDegrees ) * 0.5f;
		float s = sinf(halfTheta);
		rot = glm::quat( glm::cos( halfTheta ),  WORLD_XAXIS.x * s,  WORLD_XAXIS.y * s, WORLD_XAXIS.z * s );

        m_orientation = m_orientation * rot;
    }
}

void Camera::rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Implements the rotation logic for the flight style camera behavior.

    m_accumPitchDegrees += pitchDegrees;

    if (m_accumPitchDegrees > 360.0f)
        m_accumPitchDegrees -= 360.0f;

    if (m_accumPitchDegrees < -360.0f)
        m_accumPitchDegrees += 360.0f;
   
    glm::quat rot;

   // rot.fromHeadPitchRoll(headingDegrees, pitchDegrees, rollDegrees);
	
	glm::mat4 m = glm::yawPitchRoll( glm::radians( headingDegrees ) , glm::radians( pitchDegrees) ,glm::radians( rollDegrees ) );
	rot = glm::toQuat( m );

    m_orientation *= rot;
}

void Camera::rotateOrbit(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Implements the rotation logic for the orbit style camera behavior.
    // Roll is ignored for target Y axis orbiting.
    //
    // Briefly here's how this orbit camera implementation works. Switching to
    // the orbit camera behavior via the setBehavior() method will set the
    // camera's orientation to match the orbit target's orientation. Calls to
    // rotateOrbit() will rotate this orientation. To turn this into a third
    // person style view the updateViewMatrix() method will move the camera
    // position back 'm_orbitOffsetDistance' world units along the camera's
    // local z axis from the orbit target's world position.
    
    glm::quat rot;

    if (m_preferTargetYAxisOrbiting)
    {
        if (headingDegrees != 0.0f)
        {
            //rot.fromAxisAngle(m_targetYAxis, headingDegrees);

			float halfTheta = glm::radians( headingDegrees ) * 0.5f;
			float s = sinf(halfTheta);
			rot = glm::quat( glm::cos( halfTheta ),  m_targetYAxis.x * s,  m_targetYAxis.y * s, m_targetYAxis.z * s );

            m_orientation = rot * m_orientation;
        }

        if (pitchDegrees != 0.0f)
        {
            //rot.fromAxisAngle(WORLD_XAXIS, pitchDegrees);

			float halfTheta = glm::radians( pitchDegrees ) * 0.5f;
			float s = sinf(halfTheta);
			rot = glm::quat( glm::cos( halfTheta ),  WORLD_XAXIS.x * s,  WORLD_XAXIS.y * s, WORLD_XAXIS.z * s );

            m_orientation = m_orientation * rot;
        }
    }
    else
    {
        //rot.fromHeadPitchRoll(headingDegrees, pitchDegrees, rollDegrees);

		glm::mat4 m = glm::yawPitchRoll( glm::radians( headingDegrees ) , glm::radians( pitchDegrees) ,glm::radians( rollDegrees ) );
		rot = glm::toQuat( m );
        m_orientation *= rot;
    }
}

void Camera::updateVelocity(const glm::vec3 &direction, float elapsedTimeSec)
{
    // Updates the camera's velocity based on the supplied movement direction
    // and the elapsed time (since this method was last called). The movement
    // direction is in the range [-1,1].

    if (direction.x != 0.0f)
    {
        // Camera is moving along the x axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.x += direction.x * m_acceleration.x * elapsedTimeSec;

        if (m_currentVelocity.x > m_velocity.x)
            m_currentVelocity.x = m_velocity.x;
        else if (m_currentVelocity.x < -m_velocity.x)
            m_currentVelocity.x = -m_velocity.x;
    }
    else
    {
        // Camera is no longer moving along the x axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.x > 0.0f)
        {
            if ((m_currentVelocity.x -= m_acceleration.x * elapsedTimeSec) < 0.0f)
                m_currentVelocity.x = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.x += m_acceleration.x * elapsedTimeSec) > 0.0f)
                m_currentVelocity.x = 0.0f;
        }
    }

    if (direction.y != 0.0f)
    {
        // Camera is moving along the y axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.y += direction.y * m_acceleration.y * elapsedTimeSec;
        
        if (m_currentVelocity.y > m_velocity.y)
            m_currentVelocity.y = m_velocity.y;
        else if (m_currentVelocity.y < -m_velocity.y)
            m_currentVelocity.y = -m_velocity.y;
    }
    else
    {
        // Camera is no longer moving along the y axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.y > 0.0f)
        {
            if ((m_currentVelocity.y -= m_acceleration.y * elapsedTimeSec) < 0.0f)
                m_currentVelocity.y = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.y += m_acceleration.y * elapsedTimeSec) > 0.0f)
                m_currentVelocity.y = 0.0f;
        }
    }

    if (direction.z != 0.0f)
    {
        // Camera is moving along the z axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.z += direction.z * m_acceleration.z * elapsedTimeSec;

        if (m_currentVelocity.z > m_velocity.z)
            m_currentVelocity.z = m_velocity.z;
        else if (m_currentVelocity.z < -m_velocity.z)
            m_currentVelocity.z = -m_velocity.z;
    }
    else
    {
        // Camera is no longer moving along the z axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.z > 0.0f)
        {
            if ((m_currentVelocity.z -= m_acceleration.z * elapsedTimeSec) < 0.0f)
                m_currentVelocity.z = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.z += m_acceleration.z * elapsedTimeSec) > 0.0f)
                m_currentVelocity.z = 0.0f;
        }
    }
}

void Camera::updateViewMatrix()
{
    // Reconstruct the view matrix.

    m_viewMatrix = glm::toMat4(m_orientation);

	m_xAxis = glm::vec3(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);
	m_yAxis = glm::vec3(m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]);
	m_zAxis = glm::vec3(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]);
    m_viewDir = -m_zAxis;

    if (m_behavior == CAMERA_BEHAVIOR_ORBIT)
    {
        // Calculate the new camera position based on the current
        // orientation. The camera must always maintain the same
        // distance from the target. Use the current offset vector
        // to determine the correct distance from the target.

        m_eye = m_target + m_zAxis * m_orbitOffsetDistance;
    }

    m_viewMatrix[3][0] = -glm::dot(m_xAxis, m_eye);
    m_viewMatrix[3][1] = -glm::dot(m_yAxis, m_eye);
    m_viewMatrix[3][2] = -glm::dot(m_zAxis, m_eye);
}