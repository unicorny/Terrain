# Terrain Editor
A C++ based terrain viewer application, basically for play around with different Terrain Render Technics
SimpleTerrain use a evenly Grid, but with newer version GL_TRIANGLE_STRIP instead of GL_QUADS
SimpleTerrain now uses Multithreading for faster startup time
NTerrain use an adaptive LOD Grid, which more height differences in one place, more details
TODO: Update NTerrain for using also Multithreading for faster startup time

## Project Structure

- `src/` - Source code files
  - `GPU/` - GPU-related implementations
  - `Terrain/` - Core terrain generation and manipulation code
- `dependencies/` - External dependencies including DREngine and DRCore2

## Building the Project

This project uses CMake as its build system. To build the project:

1. Make sure you have CMake installed
2. Create a build directory: `mkdir build`
3. Navigate to build directory: `cd build`
4. Generate build files: `cmake ..`
5. Build the project: `cmake --build .`

## Dependencies

- DREngine - Custom rendering engine (included as submodule)
- DRCore2 - Custom core library (included as submodule)
- Other dependencies are managed through CMake on Windows

## Features

- Terrain generation
- 3D camera controls
- Move Forward with Num-Pad + Key
- Move Backward with Num-Pad - Key
- Move Left with a
- Move Right with d
- Move Up with w
- Move Down with s
- Rotate with Arrow-Keys
- Show Wireframe while pressing r

## You need a Data Folder next to binary
- need a terrainConfig.ini in Data folder with example content:
```ini
[Terrain]
Size=2048
Quads=512
Height=512
HeightMapPath=height.tga
```

- Size is terrain size
- Quads is quadrat count
- Height is terrain height
- HeightMapPath path to height map file relative to Data Folder.

### HeightMap
You can use all default image formats like bmp, tga (also compressed), jpg, gif, png
You can also use the format of this height map editor: https://hme.sourceforge.net/
You can also use ttp Files from David Scherfgens Terrain Editor from his Book "3D-Spiele-Programmierung mit DirectX9 und C++"

## License

This is a personal project. All rights reserved.
