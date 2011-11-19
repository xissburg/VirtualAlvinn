
#ifndef VEHICLE_H
#define VEHICLE_H

#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Vehicle/btRaycastVehicle.h"
#include "GL_ShapeDrawer.h"

class Vehicle
{
public:
	enum Wheel {
		W_FL = 0,//front left..
		W_FR = 1,
		W_RL = 2,
		W_RR = 3
	};
private:
	btRigidBody* carChassis;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	
	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster*	vehicleRayCaster;
	btRaycastVehicle* vehicle;
	btCollisionShape* wheelShape;

	btDynamicsWorld* dynamicsWorld;

	int rightIndex;
	int upIndex;
	int forwardIndex;
	btVector3 wheelDirectionCS0;
	btVector3 wheelAxleCS;

	float	engineForce;
	float	brakingForce;

	float	maxEngineForce;//this should be engine/velocity dependent
	float	maxBrakingForce;

	float	steering;
	float	targetSteering;
	float	steeringIncrement;
	float	steeringClamp;
	float	wheelRadius;
	float	wheelWidth;
	float	wheelFriction;
	float	suspensionStiffness;
	float	suspensionDamping;
	float	suspensionCompression;
	float	rollInfluence;

	btVector3 wheelColor;

	btScalar suspensionRestLength;
	
public:
	explicit Vehicle(btDynamicsWorld* world);
	~Vehicle();
	btTransform GetTransform();
	void Reset();
	void Render(GL_ShapeDrawer* shapeDrawer);
	void Update(float dt);

	void SetTargetSteering(float steering);//steering is in [-1,1]
	float GetTargetSteering();
	float GetSteering();

	void SetThrottle(float throttle);//throttle is in [0,1]
	float GetThrottle();

	void SetBraking(float braking);//braking is in [0,1]
	float GetBraking();
	
	float GetSpeed();

	void SetWheelColor(const btVector3& color) { wheelColor = color; };

	
};

#endif