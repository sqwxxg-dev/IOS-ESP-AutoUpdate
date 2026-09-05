#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <windows.h>
#include <DirectXMath.h>
#include <fstream>

extern bool b_HookEnemyCords;
extern bool b_HookLocalCords;
extern bool LocalCordshooked;
extern bool EnemyCordshooked;

extern HMODULE p_GameAssembly;
extern HMODULE p_UnityPlayer;

extern BYTE* LocalData;
extern BYTE* PlayerDataBuffer;

struct Vec2 {
    float x, y;
};
struct Vec3 {
    float x, y, z;

    Vec3 operator+(Vec3 d) {
        return { x + d.x, y + d.y, z + d.z };
    }
    Vec3 operator-(Vec3 d) {
        return { x - d.x, y - d.y, z - d.z };
    }
    Vec3 operator*(float d) {
        return { x * d, y * d, z * d };
    }
    bool operator!=(Vec3 d) {
        return { x != d.x || y != d.y || z != d.z };
    }

    float Length() const {
        return sqrt(x * x + y * y + z * z);
    }
};

struct Vec3d {
    double x, y, z;
    Vec3 ToFloat() const {
        return { (float)x, (float)y, (float)z };
    }
};
struct PlayerData {
    Vec3d Coords;
    uintptr_t baseaddr;
};
struct Vec4 {
    float x, y, z, w;
};

struct Matrix16 {
    float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
};

DWORD WINAPI CheatThread(LPVOID lpParam);

#endif 
