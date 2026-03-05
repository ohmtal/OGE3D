# 🕹 OGE3D

This is the modified fork of [Torque3D](https://github.com/TorqueGameEngines/Torque3D) 3.10 in made some years ago. 
I added some stuff, fixed bugs and now added to github. 
The goal is to make it stable and more compatible with the old Torque Game Engine (TGE). 

## Setup Basic Game on linux:

In OGE3D Folder:
```
git submodule update --init --recursive
cmake -S . -B Projects/Demo -DTORQUE_APP_NAME=Demo
cd Projects/Demo/
make -j$(nproc)
cd game
./Demo_Linux_Debug.bin
```
note: if you are not using bash: replace -j$(nproc) with -j[number of processors] -j8 for example.


``` KDevelop config example for Project Demo:
    Install Directory=/opt/OGE3D/Projects/Demo
    Build Directory Path=/opt/OGE3D/Projects/Demo
    Install Directory=/opt/OGE3D/Projects/Demo/temp
```

## current todo 
    
- [~] Demo/Playground project 
    - biggest Problem: all my projects have assets with a copyright so i cant use it here 
    
## todos in random order 

- [ ] cmake / project
    - [ ] change the cmake system so projects it can include it
    - [ ] Projects need to start the engine at the moment there is only the config header 
    - [ ] test and documentate it 

- [ ] updating to SDL3 
- [ ] updating to C++20 (evaluation first)
- [ ] updating the libs or using current versions. 
- [ ] Streamline code: 
    - [ ] do i really need oculus, hydra, physx and openvr ? 
    - [ ] I used a pre 4.0 version of 3.10 development version when i pulled it years ago and there a still fragments like the assets handling which can be removed. 
    - [ ] remove DirectX so I can concentrate on OpenGL and maybe later add Vulcan. 
    - [ ] optional: collada is deprecated. 
    - [ ] adding gltf (https://github.com/KhronosGroup/glTF-Sample-Models/tree/main)
- [ ] imgui would be cool but a lot of work to add the torque script bindings 
- [ ] the new Torque3D Torque Script code would be cool but last time (2023) i tried to port it i cancled. 
- [ ] evaluate emscripten support 
- [ ] evaluate android support 





## core / shaders / tools 

I moved them to extra repoitories to be able to use them in different projects and 
update them when changes are made in a central repo. 

 - [core](https://github.com/ohmtal/core)
 - [shaders](https://github.com/ohmtal/shaders)
 - [tools](https://github.com/ohmtal/tools)
