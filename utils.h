#pragma once
#include <Windows.h>
#include <cmath>
#define FLT_MAX 3.402823466e+38F

struct ViewMatrix_{
    float Matrix[4][4];
};

struct Vec2{
    float x, y;
    float distance_to(Vec2 b){
        return sqrt((x - b.x)*(x - b.x) + (y - b.y)*(y - b.y));
    }
};

struct Vec3{
    float x, y, z;
};

void move_mouse(Vec2 position) {
    mouse_event(MOUSEEVENTF_MOVE, position.x, position.y, 0, 0);
}

Vec2 GetScreenRes() {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return {
        static_cast<float>(desktop.right),
        static_cast<float>(desktop.bottom)
    };
}

Vec2 screenSize = GetScreenRes();

bool WorldToScreen(float Matrix[4][4], const Vec3 worldPos, Vec2& screenPos)
{
	float View = 0.f;
	float SightX = screenSize.x / 2, SightY = screenSize.y / 2;
 
	View = Matrix[3][0] * worldPos.x + Matrix[3][1] * worldPos.y + Matrix[3][2] * worldPos.z + Matrix[3][3];
	
	if (View <= 0.01)
		return false;
 
	screenPos.x = SightX + (Matrix[0][0] * worldPos.x + Matrix[0][1] * worldPos.y + Matrix[0][2] * worldPos.z + Matrix[0][3]) / View * SightX;
	screenPos.y = SightY - (Matrix[1][0] * worldPos.x + Matrix[1][1] * worldPos.y + Matrix[1][2] * worldPos.z + Matrix[1][3]) / View * SightY;
	
	return true;
}