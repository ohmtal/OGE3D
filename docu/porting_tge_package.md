# Porting TGE Package

## II. TGE 1.5.2 starter.fps 

TGE 1.4.2 was the Version i got from Garage Games for $100 in 2006?  with an free upgrade to TGE 1.52. 

Note: i have the Torque3D Interior (pro verion) source compiled in - i does not work so good but should load the Interiors 


- ./OhmtalGame_Linux_Debug.bin -game starter.fps -notools 
    -  console not picture
- let's do the fixes I did for sheep dog
    - 🔨 main.cs
        - loadDir("common"); => loadDir("core");
        - before initServer():
            - physicsInit();
            - sfxStartup(); 
    - 🔨 client/init.cs: 
        - initCanvas(...); => configureCanvas()
        - before setNetPort(0) add: loadMaterials();
    - 🔨 client/defaults.cs at top: exec( "core/scripts/client/defaults.cs" );
    - 🔨 server/init.cs: $Server::MissionFileSpec = "*/missions/*.mis"; => $Server::MissionFileSpec = $defaultGame @ "/data/missions/*.mis";
    - 🤘 in folder starter.fps:  find . -type f -exec sed -i 's/datablock TSShapeConstructor/singleton TSShapeConstructor/g' {} +
    
- ./OhmtalGame_Linux_Debug.bin -game starter.fps -notools
    - Main menu :)
    - Start mission finds the mission (🐞 names are not detected correctly)
    - load Mission 
    - 🐞 /opt/OGE3D/Engine/source/gfx/gfxTextureHandle.cpp(62,0): {Fatal} - Texture name is empty
        - 😔 Platform::debugBreak: triggering SIGSEGV for core dump
        - Interior problem ? ... too bad this message says nothing. 
        - I'll try a release build 
    - ./OhmtalGame_Linux.bin -game starter.fps -notools
        - **penultimate mission is StrongHold!!!!**
        - Waterblock is somewhere => Waterplane but we dont have the textures here 
        - Kork is a bit too transparent ;)
        -🐞 Something is missing can't access data or core folder
        

    

---


## I. SheepDog 2026-03-08

A small package i made years ago. Not sure it's standard TGE so 
this is only an example. I did this before with other packages,
but i want it todo step by step without modifing the sctipts first. 


- copied the folder into my project's game directory
- Testing: ./OhmtalGame_Linux_Debug.bin -game sheepdog: 
    - Segmentation fault (core dumped)
- 🦴./OhmtalGame_Linux_Debug.bin  -game sheepdog -notools
    - no crash and console is active
- Looking at sheepdog/main.cs
    - 🔨 loadDir("common"); => loadDir("core");
- 🦴./OhmtalGame_Linux_Debug.bin  -game sheepdog -notools
    - 🐞 **Failed to set light manager (dialog)**
        - this comes from function initLightingSystems() 
        - called by initBaseClient();
        - not here ... looking somewhere else 
- sheepdog/client/init.cs
    - 🔨 initCanvas("Auteria", true); => configureCanvas()
- Back to function onStart() in main.cs
    - 🔨 before initServer():
        - physicsInit();
        - sfxStartup();
- clients/defaults.cs
    - on top of the file:
        - exec( "core/scripts/client/defaults.cs" );
    - 🔨 Solved: **Failed to set light manager (dialog)**
        - Main Menu popup 
- 🐞 set window caption 
- 🐞 Option dialog does not match 
- 🐞 Startmission does not find any missions
    - function startMissionGui::onWake()
        - findFirstFile() invalid initial search directory: 'game:/*/missions'
        - findFirstFile() search directory not found: '*/missions/*.mis'
    - in server/init.cs (initServer)
        - 🔨 $Server::MissionFileSpec = "*/missions/*.mis"; => $Server::MissionFileSpec = $defaultGame @ "/data/missions/*.mis";
        

- 🥁 Map loaded :D 
    - 🐞 No animations 
        - 🔨 datablock TSShapeConstructor => singleton TSShapeConstructor ...
        - 🤘 find . -type f -exec sed -i 's/datablock TSShapeConstructor/singleton TSShapeConstructor/g' {} +
    - 🐞 class Sky does not exits 
    - 🐞 camera FOV not matching 
    - 🐞 /opt/OGE3D/Engine/source/scene/sceneObject.cpp(174,0): {Fatal} - SceneObject::~SceneObject - Object still linked in reference lists!

- 🕹️ there are still some 🐞 but it basically runs. not bad - all the work on OGE3D does help a bit to make it such fast. 


    
