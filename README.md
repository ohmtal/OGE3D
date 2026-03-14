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
###Notes:
- if you are not using bash: replace -j$(nproc) with -j[number of processors] -j8 for example.
- for dedicated build add **-DTORQUE_DEDICATED=ON** => cmake -S . -B Projects/Demo -DTORQUE_APP_NAME=Demo -DTORQUE_DEDICATED=ON


---

## core / shaders / tools 

I moved them to extra repoitories to be able to use them in different projects and 
update them when changes are made in a central repo. 

 - [core](https://github.com/ohmtal/core)
 - [shaders](https://github.com/ohmtal/shaders)
 - [tools](https://github.com/ohmtal/tools)
