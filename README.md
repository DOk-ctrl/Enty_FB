# C++ Custom Game Engine

[![Reddit Post](https://img.shields.io/badge/Reddit-Discussion-FF4500?style=for-the-badge&logo=reddit&logoColor=white)](https://www.reddit.com/r/opengl/comments/1w0v7mj/what_i_learned_after_5_months_of_daily_work_on_my/)
[![Play on itch.io](https://img.shields.io/badge/itch.io-Play_Demo-FA5C5C?style=for-the-badge&logo=itch.io&logoColor=white)](https://dokich-crcr.itch.io/tentacle-shippuden-enty-furry-beast)

A 3D game engine built with **C++17**(20) and **OpenGL 4.6**, integrated with **Bullet Physics**, **miniaudio**, **tinygltf**, and **Dear ImGui**. 

The core architecture follows object-oriented design principles (specifically the Bridge and State patterns) for modular code separation. Debugging and runtime telemetry are handled by a custom `Logger` class. Reddit https://www.reddit.com/r/opengl/comments/1w0v7mj/what_i_learned_after_5_months_of_daily_work_on_my/  https://dokich-crcr.itch.io/tentacle-shippuden-enty-furry-beast

### Initial Release (April)
The initial build featured full mesh collision detection for static environments and skeletal structure models, exported via CPack:
<img width="1138" height="640" alt="April-ezgif com-gif-maker" src="https://github.com/user-attachments/assets/fd09237d-174d-44dc-8243-dca4b4337d7a" />
The second release version that I'm launched on itchio(reference) and made post on the Reddit(reference). Published on late August that included new feats:

### Key Features & Updates

* IBL (Image-Based Lighting): Environment lighting powered by skybox maps.
* Advanced Shader Pipeline: Texture blending, custom GLSL shaders, and Framebuffer post-processing effects (Blur).
* AI & Gameplay Systems: State-Machine architecture for NPCs with polymorphic base enemy classes.
* Skeletal Animation: Model-specific animations driven by the State pattern.
* Graphics Settings & UI: Integrated Dear ImGui settings (MSAA toggles, LOD management, Main Menu).
* Multithreading: Asynchronous asset loading and Intel TBB integration.
<img width="1138" height="640" alt="August-ezgif com-gif-maker" src="https://github.com/user-attachments/assets/1beca521-f33b-434a-b801-a1a54b170de1" />
