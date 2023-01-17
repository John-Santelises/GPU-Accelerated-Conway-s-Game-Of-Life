# GPU-Accelerated-Conways-Game-Of-Life
Simple graphical application that renders Conways Game of Life with a high volume of cells using the GPU. The motivation was to learn how to use GPGPU libraries, as well as gain more exposure to OpenGL.


[C++ AMP](https://learn.microsoft.com/en-us/cpp/parallel/amp/cpp-amp-overview?view=msvc-170) is used, allowing for massive levels of parrellism on the GPU. It runs ontop of DirectX which means this program may be restricted to Windows only. The program will render a cell per pixel on your screen using simple OpenGL draw calls. The program will randomly start by generating a game state for you and then immediately begin processing new generations.


Controls:
R: Start Over with a new game state
Q or ESC: Quit program



[Demo Video](https://drive.google.com/file/d/10anhL4t5h0M7cvmN7L8i9I5BQXgZLNJK/view)
**Warning: Video Compression doesn't allow you to see all of the pixel level details.**
*There was a game state reset at the 5 second mark in the demo video*

Demo was running on a 1920x1080 screen therefore there were 2,073,600 cells being processed per generation.

CPU: Ryzen 9 5900HX

GPU (Accelerator): NVIDIA GeForce RTX 3080 Laptop GPU

Results: Program was able to achieve ~4 to ~25 Generations Per Second (or Frames per Second) depending on how many cells were alive. More Alive Cells = More Draw Calls = Slower Performance.
