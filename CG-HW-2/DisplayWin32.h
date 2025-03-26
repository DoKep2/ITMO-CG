//
// Created by sergo on 18.02.2025.
//

#pragma once

#include <windows.h>

class Game;

class DisplayWin32 {

private:
    Game* game;
public:
    int ClientHeight = 600;
    int ClientWidth = 800;
    HINSTANCE hInstance;
    HWND hWnd;
    WNDCLASSEX wc;
    // module

    DisplayWin32(LPCWSTR applicationName, HINSTANCE hInst, int screenWidth, int screenHeight, Game* g);
};
