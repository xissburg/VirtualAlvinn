# Virtual Alvinn

ALVINN (Autonomous Landing Vehicle In a Neural Network) is a project developed in 1996 by Dean A. Pormeleau et al where the aim was to make an autonomous car driven by a neural network solely based on the images captured by a camera mounted on the top of the vehicle. They were pretty successful, achieving a record for a vehicle driven on the road by a machine for several kilometers and their system was capable of driving the vehicle in different kinds of roads (pavement, dirt, etc).

![VirtualAlvinn](http://xissburg.com/images/VirtualAlvinn0.png)

Virtual Alvinn is an implementation of the same idea in a 3D virtual world. For the physics simulation the Bullet Physics engine is employed, with its Raycast Vehicle. For gamepad input support SDL is used. The FANN (Fast Artificial Neural Network) library is employed to implement the ANN. OpenGl is used for graphics.

The image of the camera is draw in an off-screen buffer using a Framebuffer Object (FBO). In capture mode the contents of this FBO is stored in an array. In training mode all the data that was captured is given to FANN for it to train the ANN. In ANN mode the ANN drives the vehicle.

# Building and running

Everything this project needs to build is included. However, you may get the following error in the Xcode's output when trying to run the project:
```
dyld: Library not loaded: @executable_path/../Frameworks/SDL.framework/Versions/A/SDL
  Referenced from: /Users/xissburg/Library/Developer/Xcode/DerivedData/VirtualAlvinn-albzrxerrqqgdxahsvcjjylywjad/Build/Products/Debug/VirtualAlvinn.app/Contents/MacOS/VirtualAlvinn
  Reason: image not found
```

That means you don't have the SDL.framework installed in your system. To install it, simply copy the SDL.framework folder to /Library/Frameworks. You can do that by running the following command while in the root directory of this project:

`sudo cp -rf Frameworks/SDL.framework /Library/Frameworks/`

# How to play with it

It is highly recommend that you use a gamepad with an analog stick to control the vehicle, because the ANN matches the steering angle with the image from the virtual camera. Anyway, you can also control it with the keyboard arrow keys.

## Gamepad

The following image contains the gamepad configuration for my gamepad (Logitech Cordless Rumblepad 2). I don't know how it will match other gamepads.

![Logitech Cordless Rumblepad 2](http://xissburg.com/images/GamepadConfig.jpg)

## Keyboard

To control the vehicle and stuff using a keyboard tap the following keys:

* Arrows Left/Right: Steering Left/Right
* Arrows Up/Down: Throttle/Reverse
* Space: Braking
* Return/Enter: Reset
* 1, 2: Camera
* C: Toggle image capture
* T: Train ANN
* D: Toggle driver (Human/ANN)
* Esc: Quit
