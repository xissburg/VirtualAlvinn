

#define GL_GLEXT_PROTOTYPES

#include "Alvinn.h"
#include <OpenGL/OpenGL.h>
#include <SDL/SDL_opengl.h>
#include <iostream>

#import <Cocoa/Cocoa.h>


Alvinn::Alvinn():
world(),
camType(World::CT_3RD),
screenWidth(800),
screenHeight(600),
camTextureWidth(32),
camTextureHeight(32),
joystick(NULL),
maxSamples(50000),
numSamples(0),
numInputs(camTextureWidth*camTextureHeight),
numOutputs(2),//steering and velocity
samplingRate(30),
sample(false),
ann(),
driver(D_HUMAN)
{
	world.SetScreenAspect((float)screenWidth/screenHeight);

	input = new fann_type*[maxSamples];
	output = new fann_type*[maxSamples];
	//pre-allocate items
	for(int i=0; i<maxSamples; ++i)
	{
		input[i] = new fann_type[numInputs];
		output[i] = new fann_type[numOutputs];
	}

	ann.create_standard(3, numInputs, 4, numOutputs);
	ann.set_training_algorithm(FANN::TRAIN_INCREMENTAL);
	ann.set_learning_rate(0.3f);
	ann.set_activation_function_hidden(FANN::SIGMOID_SYMMETRIC);
	ann.set_activation_function_output(FANN::SIGMOID_SYMMETRIC);
	ann.randomize_weights(-1.f, 1.f);
}

Alvinn::~Alvinn()
{
	if(joystick)
	{
		SDL_JoystickClose(joystick);
		joystick = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
}

void Alvinn::InitializeJoystick()
{
	if(SDL_NumJoysticks() > 0)
	{
		std::cout << "Found joystick: "<< SDL_JoystickName(0) << std::endl;
		joystick = SDL_JoystickOpen(0);
	}
}

void Alvinn::CheckErrors()
{
	GLenum gl_error = GL_NO_ERROR;

	while( (gl_error = glGetError( )) != GL_NO_ERROR ) 
		std::cout <<  "OpenGL error[" << gl_error << "]: " << gluErrorString(gl_error) << std::endl;

	char* sdl_error = SDL_GetError( );

	if( sdl_error[0] != '\0' ) {
		std::cout << "SDL error: " << sdl_error << std::endl;
		SDL_ClearError();
	}
}

int Alvinn::Initialize()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		std::cout << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
		return 0;
	}

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if(SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_OPENGL) == NULL)
	{
		std::cout << "Couldn't set SDL video mode: " << SDL_GetError() << std::endl;
		return 0;
	}

	SDL_WM_SetCaption("Virtual Alvinn", "Virtual Alvinn");

	InitializeJoystick();
    
    NSString *path = [[NSBundle mainBundle] pathForResource:@"groundTexture3" ofType:@"bmp"];
    
	world.InitGroundTexture([path cStringUsingEncoding:NSUTF8StringEncoding]);
    /*
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Couldn't initialize GLEW: " << glewGetErrorString(err) << std::endl;
		return 0;
	}
     */

	//initialize RTT stuff
	//create camera texture
	glGenTextures(1, &camTexture);
	glBindTexture(GL_TEXTURE_2D, camTexture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, camTextureWidth, camTextureHeight,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
    
	//create render buffer to use as depth buffer
	glGenRenderbuffersEXT(1, &depthRenderBuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthRenderBuffer);	
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, 
							 camTextureWidth, camTextureHeight);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	//create FBO
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
							  GL_TEXTURE_2D, camTexture, 0);//attach texture to color
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
								 GL_RENDERBUFFER_EXT, depthRenderBuffer);//attach render buffer to depth
	
	// check FBO status
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		std::cout << "FBO doesn't seem ok..." <<  std::endl;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	return 1;
}

void Alvinn::TrainANN()
{
    if(driver == D_HUMAN)
    {
        std::cout << "ANN is going to be trained, please wait..." << std::endl;
        td.set_train_data(numSamples, numInputs, input, numOutputs, output);
        td.shuffle_train_data();
        ann.train_on_data(td, 100, 5, 0.f);
        std::cout << std::endl << "ANN is trained." << std::endl;
    }
}

void Alvinn::ToggleCapture()
{
    if(driver == D_HUMAN)
    {
        sample = !sample;
        std::cout << (sample? "Capturing...": "Stopped capturing.") << std::endl;
    }
}

void Alvinn::ToggleDriver()
{
    if(driver == D_HUMAN)
    {
        driver = D_ANN;
        world.GetVehicle()->SetWheelColor(btVector3(0.27f,1.f,1.f));
    }
    else
    {
        driver = D_HUMAN;
        world.GetVehicle()->SetWheelColor(btVector3(1.f,0.f,0.f));
    }
    
    world.GetVehicle()->SetThrottle(0.f);
    world.GetVehicle()->SetBraking(0.f);
    world.GetVehicle()->SetTargetSteering(0.f);
}

int Alvinn::HandleEvent(const SDL_Event& event)
{
	switch(event.type)
	{
		//Keyboard
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym)
			{
				case SDLK_UP:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetThrottle(1.f);
					break;

				case SDLK_DOWN:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetThrottle(-1.f);
					break;

				case SDLK_LEFT:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetTargetSteering(1.f);
					break;

				case SDLK_RIGHT:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetTargetSteering(-1.f);
					break;

				case SDLK_SPACE:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetBraking(1.f);
                    
                default:;
			}
			break;

		case SDL_KEYUP:
			switch(event.key.keysym.sym)
			{
				case SDLK_UP:
				case SDLK_DOWN:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetThrottle(0.f);
					break;

				case SDLK_LEFT:
					if(driver == D_HUMAN && world.GetVehicle()->GetTargetSteering() > 0.f)
						world.GetVehicle()->SetTargetSteering(0.f);
					break;

				case SDLK_RIGHT:
					if(driver == D_HUMAN && world.GetVehicle()->GetTargetSteering() < 0.f)
						world.GetVehicle()->SetTargetSteering(0.f);
					break;

				case SDLK_SPACE:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetBraking(0.f);
					break;

				case SDLK_RETURN:
					world.GetVehicle()->Reset();
					break;

				case SDLK_1:
					camType = World::CT_1ST;
					break;

				case SDLK_2:
					camType = World::CT_3RD;
					break;
                    
                case SDLK_c:
                    ToggleCapture();
                    break;
                    
                case SDLK_t:
                    TrainANN();
                    break;
                    
                case SDLK_d:
                    ToggleDriver();
                    break;
			
				case SDLK_ESCAPE:
					return 0;
                    
                default:;
			}
			break;

		case SDL_JOYAXISMOTION:
			switch(event.jaxis.axis)
			{
				case 0:
					if(driver == D_HUMAN)
					{
						if(abs(event.jaxis.value) > 100)
							world.GetVehicle()->SetTargetSteering(-event.jaxis.value/(0.5f*(1<<16)));
						else
							world.GetVehicle()->SetTargetSteering(0.f);
					}
					break;

				case 3:
					world.GetVehicle()->SetThrottle(-event.jaxis.value/(0.5f*(1<<16)));
					break;

				case 2:
					if(driver == D_HUMAN)
					{
						if(abs(event.jaxis.value) > 100)
							world.GetVehicle()->SetBraking(abs(event.jaxis.value)/(0.5f*(1<<16)));
						else
							world.GetVehicle()->SetBraking(0.f);
					}
					break;
                    
                default:;
			}
			break;

		case SDL_JOYBUTTONDOWN:
			switch(event.jbutton.button)
			{
				case 1:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetThrottle(1.f);
				break;

				case 2:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetBraking(1.f);
				break;

				case 8:
					if(driver == D_HUMAN)
						world.GetVehicle()->Reset();
				break;
                    
                default:;
			}
			break;

		case SDL_JOYBUTTONUP:
			switch(event.jbutton.button)
			{
				case 1:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetThrottle(0.f);
				break;

				case 2:
					if(driver == D_HUMAN)
						world.GetVehicle()->SetBraking(0.f);
				break;

				case 3:
					if(camType == World::CT_1ST)
						camType = World::CT_3RD;
					else
						camType = World::CT_1ST;
				break;

				//toggle sampling
				case 4:
					ToggleCapture();
				break;
				
				//train ANN
				case 6:
					TrainANN();
				break;
				
				//toggle driver (human | ANN)
				case 7:
					ToggleDriver();
				break;
                    
                default:;
			}
			break;

        default:;
	}

	return 1;
}

void Alvinn::ANNDrive(fann_type* input)
{
	fann_type* output = ann.run(input);
	Vehicle* vehicle = world.GetVehicle();
	vehicle->SetTargetSteering(output[0]);
	float speed = vehicle->GetSpeed();
	float target = output[1]*300.f;
	float dif = target - speed;
	const static float delta = 1.f;
	
	if(dif > delta)//needs moar speed
	{
		vehicle->SetThrottle(1.f);
		vehicle->SetBraking(0.f);
	}
	else if(dif < -delta)//slow it down
	{
		vehicle->SetThrottle(0.f);
		vehicle->SetBraking(1.f);
	}

	std::cout << "ANN - steering: " << output[0]
			  << ", speed: " << output[1] << std::endl;
}

void Alvinn::RenderFBO()
{
	glViewport(0, 0, camTextureWidth, camTextureHeight);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	world.SetCamera(World::CT_1ST);
	world.Render();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Alvinn::RenderScreen()
{
	glViewport(0, 0, screenWidth, screenHeight);
	world.SetCamera(camType);
	world.Render();
}

void Alvinn::RenderRobotCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, camTexture);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(1.f, 1.f, 1.f);
	glTexCoord2f (0.f, 0.f);
	glVertex2f (32.f, 32.f);

	glTexCoord2f (1.f, 0.f);
	glVertex2f (160.f, 32.f);

	glTexCoord2f (1.f,1.f);
	glVertex2f (160.f, 160.f);

	glTexCoord2f (0.f, 1.f);
	glVertex2f (32.f, 160.f);
	glEnd ();

	glPopMatrix();
}

void Alvinn::Run()
{
	bool run = true;
	unsigned int* pixels = new unsigned int[camTextureWidth*camTextureHeight];
	fann_type* fPixels = new fann_type[camTextureWidth*camTextureHeight];

	while(run)
	{
		SDL_Event event;

		while(SDL_PollEvent(&event))
		{
			run = HandleEvent(event) != 0;
		}

		if(driver == D_ANN)
			ANNDrive(fPixels);

		world.Update();

		RenderFBO();

		RenderScreen();

		RenderRobotCamera();
		

		//if ANN is driving make sure to get camera image into pixels array
		if(driver == D_ANN)
		{
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			for(int i=0; i<numInputs; ++i)
				fPixels[i] = Luminance(pixels[i]);
			
		}
		
		if(sample && numSamples < maxSamples && clock.getTimeMicroseconds()*1e-6 > 1.f/samplingRate)
		{
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			for(int i=0; i<numInputs; ++i)
				input[numSamples][i] = Luminance(pixels[i]);

			output[numSamples][0] = world.GetVehicle()->GetTargetSteering();
			output[numSamples][1] = world.GetVehicle()->GetSpeed()/300.f;

			++numSamples;
            
            if(numSamples % 1000 == 0)
                std::cout << "Current sample count: " << numSamples << std::endl;
			
			if(numSamples == maxSamples)
				std::cout << "Max samples reached" << std::endl;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);


		SDL_GL_SwapBuffers();

		CheckErrors();

		SDL_Delay(10);
	}

	delete[] pixels;
}

float Luminance(unsigned int abgr)
{
	float r = (abgr & 0xff)/255.f;
	float g = ((abgr>>8) & 0xff)/255.f;
	float b = ((abgr>>16) & 0xff)/255.f;

	return (0.3f*r + 0.59f*g + 0.11f*b);//luminance
}