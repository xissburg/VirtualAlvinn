
#include "World.h"
#include <iostream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SDL.h>


World::World():
	dynamicsWorld(0),
	collisionShapes(),
	overlappingPairCache(0),
	dispatcher(0),
	constraintSolver(0),
	collisionConfiguration(0),
	indexVertexArrays(0),
	vertices(0),
	clock(),
	groundTexture(0),

	screenAspect(1.f),
	cameraHeight(4.f),
	minCameraDistance(3.f),
	maxCameraDistance(10.f),
	cameraPosition(-100.f, 30.f, 100.f),

	cameraType(CT_3RD),
	idle(false)

{
	shapeDrawer = new GL_ShapeDrawer();
	shapeDrawer->enableTexture(true);

	btCollisionShape* groundShape = new btBoxShape(btVector3(50,3,50));
	collisionShapes.push_back(groundShape);
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	overlappingPairCache = new btAxisSweep3(worldMin,worldMax);
	constraintSolver = new btSequentialImpulseConstraintSolver();

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, constraintSolver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-10,0));

	btTransform tr;
	tr.setIdentity();

	//triangle mesh
	int i;

	const float TRIANGLE_SIZE=20.f;

	//create a triangle-mesh ground
	int vertStride = sizeof(btVector3);
	int indexStride = 3*sizeof(int);

	const int NUVERTS_X = 21;
	const int NUVERTS_Y = 21;
	const int totalVerts = NUVERTS_X*NUVERTS_Y;
	
	const int totalTriangles = 2*(NUVERTS_X-1)*(NUVERTS_Y-1);

	vertices = new btVector3[totalVerts];
	int*	gIndices = new int[totalTriangles*3];

	
	//build terrain mesh
	for ( i=0;i<NUVERTS_X;i++)
	{
		for (int j=0;j<NUVERTS_Y;j++)
		{
			float wl = .2f;
			//height set to zero, but can also use curved landscape, just uncomment out the code
			float height = 20.f*sinf(float(i)*wl)*cosf(float(j)*wl);
			vertices[i+j*NUVERTS_X].setValue(
				(i-NUVERTS_X*0.5f)*TRIANGLE_SIZE,
				height,
				(j-NUVERTS_Y*0.5f)*TRIANGLE_SIZE);

		}
	}

	int index=0;
	for ( i=0;i<NUVERTS_X-1;i++)
	{
		for (int j=0;j<NUVERTS_Y-1;j++)
		{
			gIndices[index++] = j*NUVERTS_X+i;
			gIndices[index++] = j*NUVERTS_X+i+1;
			gIndices[index++] = (j+1)*NUVERTS_X+i+1;

			gIndices[index++] = j*NUVERTS_X+i;
			gIndices[index++] = (j+1)*NUVERTS_X+i+1;
			gIndices[index++] = (j+1)*NUVERTS_X+i;
		}
	}
	
	indexVertexArrays = new btTriangleIndexVertexArray(totalTriangles,
		gIndices,
		indexStride,
		totalVerts,(btScalar*) &vertices[0].x(),vertStride);

	bool useQuantizedAabbCompression = true;
	groundShape = new btBvhTriangleMeshShape(indexVertexArrays,useQuantizedAabbCompression);
	
	tr.setOrigin(btVector3(0,-4.5f,0));

	collisionShapes.push_back(groundShape);

	//create ground object
	groundRigidBody = CreateRigidBody(0,tr,groundShape);
	dynamicsWorld->addRigidBody(groundRigidBody);

	float wallLength = (NUVERTS_X-1)*TRIANGLE_SIZE;
	float wallHeight = 40.f;
	float wallWidth = 5.f;

	btCollisionShape* wall = new btBoxShape(btVector3(wallLength/2, wallHeight/2, wallWidth/2));
	collisionShapes.push_back(wall);
	tr.setOrigin(btVector3(-TRIANGLE_SIZE/2.f, 0.f, wallLength/2-wallWidth/2-TRIANGLE_SIZE/2));
	dynamicsWorld->addRigidBody(CreateRigidBody(0.f, tr, wall));

	wall = new btBoxShape(btVector3(wallLength/2, wallHeight/2, wallWidth/2));
	collisionShapes.push_back(wall);
	tr.setOrigin(btVector3(-TRIANGLE_SIZE/2.f, 0.f, -wallLength/2-wallWidth/2-TRIANGLE_SIZE/2));
	dynamicsWorld->addRigidBody(CreateRigidBody(0.f, tr, wall));

	wall = new btBoxShape(btVector3(wallWidth/2, wallHeight/2, wallLength/2));
	collisionShapes.push_back(wall);
	tr.setOrigin(btVector3(wallLength/2-wallWidth/2-TRIANGLE_SIZE/2, 0.f, -TRIANGLE_SIZE/2.f));
	dynamicsWorld->addRigidBody(CreateRigidBody(0.f, tr, wall));

	wall = new btBoxShape(btVector3(wallWidth/2, wallHeight/2, wallLength/2));
	collisionShapes.push_back(wall);
	tr.setOrigin(btVector3(-wallLength/2-wallWidth/2-TRIANGLE_SIZE/2, 0.f, -TRIANGLE_SIZE/2.f));
	dynamicsWorld->addRigidBody(CreateRigidBody(0.f, tr, wall));

	//create vehicle
	vehicle = new Vehicle(dynamicsWorld);
	
	cameraDistance = 26.f;
}

World::~World()
{
	for (int i = dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}

	//delete collision shapes
	for (int j=0; j<collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		delete shape;
	}

	delete indexVertexArrays;
	delete vertices;
	
	delete vehicle;

	delete dynamicsWorld;

	delete constraintSolver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;

	delete shapeDrawer;
}

void World::InitGroundTexture(std::string texFilename)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        fprintf(stdout, "Current working dir: %s\n", cwd);
    else
        perror("getcwd() error");
    
	SDL_Surface* surface = SDL_LoadBMP(texFilename.c_str());

	if(surface == NULL)
	{
		std::cout << "Failed to load " << texFilename<< std::endl;
		return;
	}

	Uint8 nOfColors = surface->format->BytesPerPixel;
	GLenum textureFormat = GL_RGBA;

    if (nOfColors == 4)     // contains an alpha channel
    {
        if (surface->format->Rmask == 0x000000ff)
                textureFormat = GL_RGBA;
        else
                textureFormat = GL_BGRA_EXT;
    } 
	else if (nOfColors == 3)     // no alpha channel
    {
        if (surface->format->Rmask == 0x000000ff)
                textureFormat = GL_RGB;
        else
                textureFormat = GL_BGR_EXT;
    }
        
	glGenTextures( 1, &groundTexture );
	glBindTexture( GL_TEXTURE_2D, groundTexture );
 
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, 0x84FE, 16);//hardcoded anisotropy
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, nOfColors, surface->w, surface->h, 
						textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
	/*glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      textureFormat, GL_UNSIGNED_BYTE, surface->pixels );*/
	glBindTexture(GL_TEXTURE_2D, 0);

}

void World::SetupGL()
{
	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//light_position is NOT default value
	GLfloat light_position0[] = { 1.0f, 10.0f, 1.0f, 0.0f };
	GLfloat light_position1[] = { -1.0f, -10.0f, -1.0f, 0.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.57f,0.8f,0.93f,0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void World::UpdateCamera1st()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glFrustum (-screenAspect, screenAspect, -1.0, 1.0, 1.0, 10000.0);
	gluPerspective(120.0, screenAspect, 1.0, 10000.0);

	btTransform chassisWorldTrans = vehicle->GetTransform();
	btQuaternion chassisQuat = chassisWorldTrans.getRotation();
	btQuaternion qDirFront = chassisQuat * btQuaternion(0.f, 0.f, 1.f, 0.f) * chassisQuat.inverse();
	btQuaternion qDirUp = chassisQuat * btQuaternion(0.f, 1.f, 0.f, 0.f) * chassisQuat.inverse();
	btVector3 camDirFront(qDirFront[0], qDirFront[1], qDirFront[2]);
	btVector3 camDirUp(qDirUp[0], qDirUp[1], qDirUp[2]);


	btVector3 position = chassisWorldTrans.getOrigin() + camDirFront*2.3f + camDirUp*2.f;

	btVector3 targetPosition = position + camDirFront - camDirUp*0.9f;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    gluLookAt(position[0], position[1], position[2],
              targetPosition[0], targetPosition[1], targetPosition[2],
              camDirUp[0], camDirUp[1], camDirUp[2]);
}



void World::UpdateCamera3rd()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	btTransform chassisWorldTrans = vehicle->GetTransform();

	//look at the vehicle
	cameraTargetPosition = chassisWorldTrans.getOrigin();

	//interpolate the camera height
	cameraPosition[1] = (15.f*cameraPosition[1] + cameraTargetPosition[1] + cameraHeight)/16.f;

	btVector3 camToObject = cameraTargetPosition - cameraPosition;

	//keep distance between min and max distance
	float cameraDistance = camToObject.length();
	float correctionFactor = 0.f;

	if (cameraDistance < minCameraDistance)
	{
		correctionFactor = 0.2f*(minCameraDistance-cameraDistance)/cameraDistance;
	}
	if (cameraDistance > maxCameraDistance)
	{
		correctionFactor = 0.2f*(maxCameraDistance-cameraDistance)/cameraDistance;
	}

	cameraPosition -= correctionFactor*camToObject;
	
    glFrustum (-screenAspect, screenAspect, -1.0, 1.0, 1.0, 10000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2],
              cameraTargetPosition[0], cameraTargetPosition[1], cameraTargetPosition[2],
              0.f, 1.f, 0.f);
}

void World::Render()
{
	if(cameraType == CT_3RD)
		UpdateCamera3rd();
	else if(cameraType == CT_1ST)
		UpdateCamera1st();

	SetupGL();

	btScalar m[16];
	btMatrix3x3	rot;
	rot.setIdentity();
	const int numObjects = dynamicsWorld->getNumCollisionObjects();
	btVector3 wireColor(1,0,0);

	//render all objs
	for(int i=0; i<numObjects; i++)
	{
		btCollisionObject* colObj=dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body=btRigidBody::upcast(colObj);

		if(body&&body->getMotionState())
		{
			btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			rot = myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix(m);
			rot = colObj->getWorldTransform().getBasis();
		}
		btVector3 wireColor(1.f,1.0f,0.5f); //wants deactivation
		if(i&1) wireColor=btVector3(0.f,0.0f,1.f);
		///color differently for active, sleeping, wantsdeactivation states
		if (colObj->getActivationState() == 1) //active
		{
			if (i & 1)
			{
				wireColor += btVector3 (1.f,0.f,0.f);
			}
			else
			{			
				wireColor += btVector3 (.5f,0.f,0.f);
			}
		}
		if(colObj->getActivationState()==2) //ISLAND_SLEEPING
		{
			if(i&1)
			{
				wireColor += btVector3 (0.f,1.f, 0.f);
			}
			else
			{
				wireColor += btVector3 (0.f,0.5f,0.f);
			}
		}

		btVector3 aabbMin,aabbMax;
		dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin, aabbMax);
		
		aabbMin-=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);
		aabbMax+=btVector3(BT_LARGE_FLOAT,BT_LARGE_FLOAT,BT_LARGE_FLOAT);

		//set texture for terrain
		if(body == groundRigidBody)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, groundTexture);

			static const GLfloat	planex[]={0.1f,0.f,0.f,21.f};
			//static const GLfloat	planey[]={0,0.025,0,0};
			static const GLfloat	planez[]={0.f,0.f,0.1f,21.f};

			glTexGenfv(GL_S,GL_OBJECT_PLANE,planex);
			glTexGenfv(GL_T,GL_OBJECT_PLANE,planez);
			glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);

			wireColor = btVector3(1.f, 1.f, 1.f);
		}

		shapeDrawer->drawOpenGL(m, colObj->getCollisionShape(), wireColor, 0, aabbMin, aabbMax);

		if(body == groundRigidBody)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
		}
	}

	//draw wheels
	if(cameraType != CT_1ST)
		vehicle->Render(shapeDrawer);
}


void World::Update()
{
	float dt = MicroDeltaTime() * 1e-6f;
	
	if (dynamicsWorld)
	{
		//during idle mode, just run 1 simulation step maximum
		int maxSimSubSteps = idle ? 1 : 2;
		if (idle)
			dt = 1.f/420.f;

		vehicle->Update(dt);
		dynamicsWorld->stepSimulation(dt, maxSimSubSteps);
	}
}


unsigned long World::MicroDeltaTime()
{
	unsigned long dt = clock.getTimeMicroseconds();
	clock.reset();
	return dt;
}


btRigidBody* World::CreateRigidBody(float mass, const btTransform &startTransform, btCollisionShape *shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);
	btVector3 localInertia(0,0,0);

	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
	btRigidBody* body = new btRigidBody(cInfo);

	return body;
}