
#ifndef ALVINN_H
#define ALVINN_H

#define GL_SGIX_fragment_lighting

#include <SDL.h>
#include <SDL_opengl.h>
#include "World.h"

#include <fann.h>
#include <fann_cpp.h>

class Alvinn
{
private:
	World world;
	World::CameraType camType;

	int screenWidth, screenHeight;
	int camTextureWidth, camTextureHeight;
	GLuint camTexture;
	GLuint depthRenderBuffer;
	GLuint fbo;

	FANN::neural_net ann;
	FANN::training_data td;

	enum DRIVER {
		D_HUMAN,
		D_ANN
	} driver;

	btClock clock;//clock to control samples per second
	int maxSamples;//maximum number of samples to capture (the size of the input and output arrays)
	int numSamples;//current number of samples captured
	int numInputs;//number of elements in each entry in the input array (camTextureWidth*camTextureHeight)
	int numOutputs;//number of elements in each entry in the output array (steering, throttle, brake)
	int samplingRate;//number of samples to capture per second
	bool sample;//whether to capture samples(true) or not(false)

	fann_type** input;
	fann_type** output;

	SDL_Joystick* joystick;

	void InitializeJoystick();//sets joystick to the first joystick found if any, NULL otherwise
	void CheckErrors();//verifies whether any sdl/opengl error ocurred during the main loop and output them
	int HandleEvent(const SDL_Event& event);
	void ANNDrive(fann_type* input);
	void RenderFBO();
	void RenderScreen();
	void RenderRobotCamera();
    void ToggleCapture();
    void TrainANN();
    void ToggleDriver();
public:
	Alvinn();//sets up basic config
	~Alvinn();
	int Initialize();//initialize stuff including SDL and OpenGL. Retuns 0 on error, other values otherwise
	void Run();
};

float Luminance(unsigned int abgr);

#endif