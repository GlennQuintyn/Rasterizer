# Rasterizer

https://github.com/user-attachments/assets/e7515e24-3a49-4d8b-a7d7-e6dd22f0e3ba

___
The Cyan background is the hardware accelerated version using DirectX11
The light grey background one is the software (CPU only) implementation

The video showcases the smooth runtime transition between the 2.
It also shows the differnt culling modes and other visualizing modes mentioned below in the Extra Contols tab

___

### -= GENERAL CONTROLS =-
- Hold LMB: to rotate camera left/right or go forwad/backwards
- Hold RMB: to rotate camera left/right or go forwad/backwards
- W/S: to go forwad/backwards respcetivly (this is relative to the camera foward)
- A/D: to go left/right respcetivly (this is relative to the camera foward)
- Q/E: to go down/up relative to the camera foward

### -= EXTRA CONTROLS =-
- R  : to toggle between switch between Software or DirectX rasterizer
- F  : to change filtering mode (point, linear, anisotropic) **DirectX ONLY!!!**
- T  : to toggle transparency **DirectX ONLY!!!**
- C  : to toggle between toggle between cullmodes (front, back and no culling)
- +/-: to increase/decrease the FOV (clamped between 0-180)
- Z  : to toggle between visualizing modes (lambert phong, only diffuse sample, dept buffer, triangle vertex color interpolation) **SOFTWARE MODE ONLY!!!**
