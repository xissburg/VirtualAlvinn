# Virtual Alvinn

ALVINN (Autonomous Landing Vehicle In a Neural Network) is a project developed in 1996 by Dean A. Pormeleau et al where the aim was to make an autonomous car driven by a neural network solely based on the images captured by a camera mounted on the top of the vehicle. They were pretty successful, achieving a record for a vehicle driven on the road by a computer for several kilometers and their system was capable of driving the vehicle in different kinds of roads (pavement, dirt, etc).

![VirtualAlvinn](http://xissburg.com/images/VirtualAlvinn0.png)

Virtual Alvinn is an implementation of the same idea in a 3D virtual world. For the physics simulation the Bullet Physics engine is employed, with its Raycast Vehicle. For gamepad input support SDL is used. The FANN (Fast Artificial Neural Network) library is employed to implement the ANN. OpenGl is used for graphics.

The image of the camera is draw in an off-screen buffer using a Framebuffer Object (FBO), which is exactly the image in the little square on the bottom left corner of the screen. In capture mode the contents of this FBO is stored in an array. In training mode all the data that was captured is given to FANN for it to train the ANN. In ANN mode the ANN drives the vehicle.

# Building and running

Everything this project needs to build is included. However, you may get the following error in the Xcode's output when trying to run the project:

dyld: Library not loaded: @executablepath/../Frameworks/SDL.framework/Versions/A/SDL
  Referenced from: /Users/xissburg/Library/Developer/Xcode/DerivedData/VirtualAlvinn-albzrxerrqqgdxahsvcjjylywjad/Build/Products/Debug/VirtualAlvinn.app/Contents/MacOS/VirtualAlvinn
  Reason: image not found

That means you don't have the SDL.framework installed in your system. To install it, simply copy the SDL.framework folder to /Library/Frameworks. You can do that by running the following command while in the root directory of this project:

`sudo cp -rf Frameworks/SDL.framework /Library/Frameworks/`

# How to play with it

It is highly recommend that you use a gamepad with an analog stick to control the vehicle, because the ANN matches the steering angle with the image from the virtual camera. Anyway, you can also control it with the keyboard arrow keys.

First, drive a little to get used to that car. It is quite unstable in high speeds then, be careful or you may hurt yourself. Also, make sure you are not drunk before driving it. Place your car on the road and press the _Toggle Image Capture_ button to start capturing and storing the images along with the car state (steering angle and speed), and keep driving. Watch the console for useful information, like how many samples you have already taken. When you're done, press the _Toggle Image Capture_ button again to stop capturing and press the _Train ANN_ button to train the neural network. Watch the console for the training status. When done, put you car back on the road and press the _Toggle Driver_ button to put the ANN to work. The wheels should turn blue to indicate the ANN is driving. If the ANN goes crazy and get stuck, press the _Toggle Driver_ button again to take control of the car, put it back to the center of the road, and press the  _Toggle Driver_ button again to allow the ANN to drive.

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
