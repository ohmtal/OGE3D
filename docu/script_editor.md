# Torque Script Editor
Using imGui. 


- [X] Scan all script files ~~in background using a thread~~ (config array: *.cs,*.hfl,*.mis,*.gui,*.glsl,*.hlsl)
- [X] Multiple edit windows using docking 
- [ ] File ~~Browser window~~ Tree
    - [X] load
    - [ ] delete 
    - [ ] favorites (saved as console variable tab separated? )
- [ ] dirty => do you want to save....
- [ ] Actions
    - [ ] File:
        - new
        - *load => click on file browser :P 
        - [ ] save => ctrl+s done calling:  bool ScriptEditor::saveEditor(EditorState& editorState, String fullPath) 
        - save as 
    - [ ] Edit
        - copy 
        - paste 
        - Undo
        - Redo 
        - Find
        - Find in Files
    - [ ] Reload (hot)
- [ ] Syntax highlighting
- [ ] nice to have but maybe a bit too much for now: 
    - [ ] Code snippet Templates
    - [ ] Completion by detecting type and read the object properties
    - [ ] Debugger
