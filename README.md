# GPUFluid

Real-time 2D Eulerian fluid simulation using OpenGL compute shader. 

![GPUFluid2D](https://github.com/masatakesato/GPUFluid/blob/main/media/GPUFluid2D.mp4?raw=true)

## About

An experimental implementation of fluid interaction. 

## Features

- Real-time 2D Eulerian fluid simulation based on StableFluids

- Vorticity confinement

- MacCormack advection

## Running application

1. Download prebuilt application from [**here**](https://github.com/masatakesato/GPUFluid/releases/download/v0.0.1-alpha/GPUFluid-v.0.0.1-alpha.zip).

2. Extract zip file and execute GPUFluid2D.exe inside bin folder.

## Building application

### Prerequisities

Following system configurations are required to build application.

- Windows 10(or later) with Visual Studio Community 2017 installed

- freeglut*

- AntTweakBar* 

- FreeImage* 

* Installed under D:\Lib\ in my configuration.

### Setup

1. Clone this repositoty.

2. Create a new directory named "**external**" in the /GPUFluid directory.

3. Download **[OreOreLib](https://github.com/masatakesato/OreOreLib/releases/download/2022q1/oreore.zip)** and put extracted directory into "**external**".

4. Download **[GraphicsLib](https://github.com/masatakesato/GraphicsLib/releases/download/2022q1/graphics.zip)** and put extracted directory into "**external**".

5. Open VisualStudio project
   
   - /GPUFluid/dev/GPUFluid2D/GPUFluid2D/GPUFluid2D.vcxproj

6. Modify "**Additional Include Directory**" and "**Additional Library Directories**" settings related to following libraries
   
   - freeglut
   
   - AntTweakBar
   
   - FreeImage

7. Build project.
