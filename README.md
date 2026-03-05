# 🕹 OGE3D

This is the modified fork of [Torque3D](https://github.com/TorqueGameEngines/Torque3D) 3.10 in made some years ago. I added some stuff, fixed bugs and now added to github. 
The goal is to make it more compatible with the old Torque Game Engine (TGE). 
I also think about: 


## current todo 
- [ ] cmake
    - [ ] change the cmake system so i can work with KDevelop more comfortable 
    - [ ] test and documentate it 
- [ ] Demo/Playground project 
    
## todos in random order 

- [ ] updating to SDL3 
- [ ] updating to C++20 (evaluation first)
- [ ] updating the libs or using current versions. 
- [ ] Streamline code: 
    - [ ] do i really need oculus, hydra, physx and openvr ? 
    - [ ] I used a pre 4.0 version of 3.10 development version when i pulled it years ago and there a still fragments like the assets handling which can be removed. 
    - [ ] remove DirectX so I can concentrate on OpenGL and maybe later add Vulcan. 
    - [ ] optional: removing the old collada and adding gltf 
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
