#pragma once
#include <functional>

enum class GameState {
    MAIN_MENU,   
    LOADING,     
    GAMEPLAY,    
    DEATH        
};


enum class MenuAction {
    NONE,
    START_GAME,
    SETTINGS,
    CREDITS,     QUIT
};
