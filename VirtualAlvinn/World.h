
#ifndef WORLD_H
#define WORLD_H

#include "btBulletDynamicsCommon.h"
#include "LinearMath/btQuickprof.h"
#include "GL_ShapeDrawer.h"
#include "Vehicle.h"
#include <SDL/SDL_opengl.h>

#include <string>


class World
{
public:
	enum CameraType{
		CT_1ST,
		CT_3RD
	};

private:
	btDynamicsWorld* dynamicsWorld;
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btBroadphaseInterface*	overlappingPairCache;
	btCollisionDispatcher*	dispatcher;
	btConstraintSolver*	constraintSolver;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btTriangleIndexVertexArray*	indexVertexArrays;
	btRigidBody* groundRigidBody;
	

	btVector3*	vertices;
	
	btClock	clock;

	Vehicle* vehicle;

	float	screenAspect;
	float	cameraHeight;
	float	cameraDistance;
	float	minCameraDistance;
	float	maxCameraDistance;
	float   ele;
	float   azi;
	btVector3 cameraPosition;
	btVector3 cameraTargetPosition;

	GL_ShapeDrawer*	shapeDrawer;

	void SetupGL();
	void UpdateCamera3rd();
	void UpdateCamera1st();
	unsigned long MicroDeltaTime();

	CameraType cameraType;
	

	bool idle;

public:
	World();
	~World();
GLuint groundTexture;
	void Render();
	void Update();
	void InitGroundTexture(std::string texFilename);

	Vehicle* GetVehicle() const { return vehicle; }

	float GetScreenAspect() { return screenAspect; }
	void SetScreenAspect(float aspect) { screenAspect = aspect; }

	void SetCamera(CameraType cam) { cameraType = cam; }

	static btRigidBody* CreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape);
};

#endif