
#include "Vehicle.h"
#include "World.h"

#define CUBE_HALF_EXTENTS 1


Vehicle::Vehicle(btDynamicsWorld* world):
	carChassis(0),
	collisionShapes(),
	tuning(),
	vehicleRayCaster(0),
	vehicle(0),
	wheelShape(0),

	dynamicsWorld(world),

	rightIndex(0),
	upIndex(1),
	forwardIndex(2),
	wheelDirectionCS0(0,-1,0),
	wheelAxleCS(-1,0,0),

	engineForce(0.f),
	brakingForce(0.f),

	maxEngineForce(2000.f),
	maxBrakingForce(100.f),

	steering(0.f),
	targetSteering(0.f),
	steeringIncrement(0.9f),
	steeringClamp(0.4f),
	wheelRadius(0.5f),
	wheelWidth(0.4f),
	wheelFriction(10.f),
	suspensionStiffness(20.f),
	suspensionDamping(2.3f),
	suspensionCompression(4.4f),
	rollInfluence(0.1f),

	wheelColor(1,0,0),

	suspensionRestLength(0.6f)
{
	btTransform tr;
	tr.setIdentity();

	btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f,0.5f,2.f));
	collisionShapes.push_back(chassisShape);

	btCompoundShape* compound = new btCompoundShape();
	collisionShapes.push_back(compound);
	btTransform localTrans;
	localTrans.setIdentity();
	localTrans.setOrigin(btVector3(0,0.7,0));//localTrans effectively shifts the center of mass with respect to the chassis

	compound->addChildShape(localTrans,chassisShape);

	tr.setOrigin(btVector3(0,0.f,0));

	carChassis = World::CreateRigidBody(1000,tr,compound);//chassisShape);
	dynamicsWorld->addRigidBody(carChassis);
	//m_carChassis->setDamping(0.2,0.2);
	
	wheelShape = new btCylinderShapeX(btVector3(wheelWidth,wheelRadius,wheelRadius));
	
	Reset();

	/// create vehicle
	vehicleRayCaster = new btDefaultVehicleRaycaster(dynamicsWorld);
	vehicle = new btRaycastVehicle(tuning, carChassis, vehicleRayCaster);
	
	///never deactivate the vehicle
	carChassis->setActivationState(DISABLE_DEACTIVATION);

	dynamicsWorld->addVehicle(vehicle);

	float connectionHeight = 0.5f;

	bool isFrontWheel=true;

	//choose coordinate system
	vehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

	btVector3 connectionPointCS0(CUBE_HALF_EXTENTS-(0.3f*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
	vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,tuning,isFrontWheel);

	connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3f*wheelWidth),connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
	vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,tuning,isFrontWheel);

	isFrontWheel = false;

	connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3f*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
	vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,tuning,isFrontWheel);

	connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3f*wheelWidth),connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
	vehicle->addWheel(connectionPointCS0,wheelDirectionCS0,wheelAxleCS,suspensionRestLength,wheelRadius,tuning,isFrontWheel);
	
	for (int i=0;i<vehicle->getNumWheels();i++)
	{
		btWheelInfo& wheel = vehicle->getWheelInfo(i);
		wheel.m_suspensionStiffness = suspensionStiffness;
		wheel.m_wheelsDampingRelaxation = suspensionDamping;
		wheel.m_wheelsDampingCompression = suspensionCompression;
		wheel.m_frictionSlip = wheelFriction;
		wheel.m_rollInfluence = rollInfluence;
	}
	
}

Vehicle::~Vehicle()
{
	//delete collision shapes
	for (int j=0;j<collisionShapes.size();j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		delete shape;
	}

	delete vehicleRayCaster;

	delete vehicle;

	delete wheelShape;
}

btTransform Vehicle::GetTransform()
{
	btTransform t;
	carChassis->getMotionState()->getWorldTransform(t);
	return t;
}


void Vehicle::Reset()
{
	steering = 0.f;
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(0.f, 0.f, 0.f));
	carChassis->setCenterOfMassTransform(tr);
	carChassis->setLinearVelocity(btVector3(0,0,0));
	carChassis->setAngularVelocity(btVector3(0,0,0));
	dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(carChassis->getBroadphaseHandle(), dynamicsWorld->getDispatcher());

	if (vehicle)
	{
		vehicle->resetSuspension();
		for (int i=0;i<vehicle->getNumWheels();i++)
		{
			//synchronize the wheels with the (interpolated) chassis worldtransform
			vehicle->updateWheelTransform(i,true);
		}
	}
}

void Vehicle::Render(GL_ShapeDrawer* shapeDrawer)
{
	//render wheels
	btScalar m[16];

	btVector3 worldBoundsMin,worldBoundsMax;
	dynamicsWorld->getBroadphase()->getBroadphaseAabb(worldBoundsMin,worldBoundsMax);

	for (int i=0;i<vehicle->getNumWheels();i++)
	{
		//synchronize the wheels with the (interpolated) chassis worldtransform
		vehicle->updateWheelTransform(i,true);
		//draw wheels (cylinders)
		vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
		shapeDrawer->drawOpenGL(m, wheelShape, wheelColor, 0, worldBoundsMin, worldBoundsMax);
	}
}

void Vehicle::Update(float dt)
{
	const static float delta = 0.01f;
	//update steering
	float dif = targetSteering - steering;

	if(dif > delta)
	{
		steering += steeringIncrement*dt;
		steering = btMin(btMax(-1.f, steering), 1.f);//clamp
	}
	else if(dif < -delta)
	{
		steering -= steeringIncrement*dt;
		steering = btMin(btMax(-1.f, steering), 1.f);//clamp
	}

	vehicle->setSteeringValue(steering, W_FL);
	vehicle->setSteeringValue(steering, W_FR);
}

void Vehicle::SetTargetSteering(float steering)
{
	steering = btMin(btMax(-1.f, steering), 1.f);//clamp
	targetSteering = steering * steeringClamp;
}

float Vehicle::GetTargetSteering()
{
	return targetSteering/steeringClamp;
}

float Vehicle::GetSteering()
{
	return steering/steeringClamp;
}

void Vehicle::SetThrottle(float throttle)
{
	throttle = btMin(btMax(-1.f, throttle), 1.f);//clamp
	engineForce = maxEngineForce * throttle;

	vehicle->applyEngineForce(engineForce, W_RL);
	vehicle->applyEngineForce(engineForce, W_RR);
}

float Vehicle::GetThrottle()
{
	return engineForce;
}

void Vehicle::SetBraking(float braking)
{
	braking = btMin(btMax(0.f, braking), 1.f);//clamp
	brakingForce = maxBrakingForce * braking;

	vehicle->setBrake(brakingForce, W_RL);
	vehicle->setBrake(brakingForce, W_RR);
}

float Vehicle::GetBraking()
{
	return brakingForce;
}

float Vehicle::GetSpeed()
{
	return vehicle->getCurrentSpeedKmHour();
}