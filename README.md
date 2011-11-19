# Virtual Alvinn

ALVINN (Autonomous Landing Vehicle In a Neural Network) is a project developed in 1996 by Dean A. Pormeleau et al where the aim was to make an autonomous car driven by a neural network solely based on the images captured by a camera mounted on the top of the vehicle. They were pretty successful, achieving a record for a vehicle driven on the road by a machine for several kilometers and their system was capable of driving the vehicle in different kinds of roads (pavement, dirt, etc).

Virtual Alvinn is an implementation of the same idea in a 3D virtual world. For the physics simulation the Bullet Physics engine is employed, with its Raycast Vehicle. For gamepad input support SDL is used. The FANN (Fast Artificial Neural Network) library is employed to implement the ANN. OpenGl is used for graphics.

The image of the camera is draw in an off-screen buffer using a Framebuffer Object (FBO). In capture mode the contents of this FBO is stored in an array. In training mode all the data that was captured is given to FANN for it to train the ANN. In ANN mode the ANN drives the vehicle.

