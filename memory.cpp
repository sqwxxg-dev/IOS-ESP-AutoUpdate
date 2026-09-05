#include "memory.hpp"

bool b_HookEnemyCords = false;
bool b_HookLocalCords = false;

HMODULE p_UnityPlayer = 0;

BYTE StoreEnemyData[89] = { 0xF2, 0x0F, 0x10, 0x4A, 0x10, 0x52, 0x50, 0x53, 0x8B, 0x05, 0xEE, 0xFF, 0xFF, 0xFF, 0x83, 0xF8, 0x0A, 0x0F, 0x8C, 0x08, 0x0, 0x0, 0x0, 0x31, 0xC0, 0x89, 0x05, 0xDD, 0xFF, 0xFF, 0xFF, 0x48, 0x6B, 0xD8, 0x20, 0x48, 0x8D, 0x15, 0x92, 0xFE, 0xFF, 0xFF, 0x48, 0x01, 0xD3, 0x0F, 0x11, 0x03, 0xF2, 0x0F, 0x11, 0x4B, 0x10, 0x48, 0x89, 0x4B, 0x18, 0x8B, 0x05, 0xBD, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x89, 0x05, 0xB5, 0xFF, 0xFF, 0xFF, 0x5B, 0x58, 0x5A, 0x0F, 0x11, 0x81, 0xF0, 0x01, 0x0, 0x0, 0xF2, 0x0F, 0x11, 0x89, 0x0, 0x02, 0x0, 0x0 };
BYTE OrigEnemyData[20] = { 0x0F, 0x11, 0x81, 0xF0, 0x01, 0x0, 0x0, 0xF2, 0x0F, 0x10, 0x4A, 0x10, 0xF2, 0x0F, 0x11, 0x89, 0x0, 0x02, 0x0, 0x0 };
BYTE StoreLocalData[49] = { 0x50, 0x0F, 0x11, 0x05, 0xD8, 0xFF, 0xFF, 0xFF, 0xF2, 0x0F, 0x10, 0x4F, 0x18, 0xF2, 0x0F, 0x11, 0x0D, 0xDB, 0xFF, 0xFF, 0xFF, 0x49, 0x8B, 0xC6, 0x48, 0x89, 0x05, 0xD9, 0xFF, 0xFF, 0xFF, 0x58, 0x41, 0x0F, 0x11, 0x86, 0xF0, 0x01, 0x0, 0x0, 0xF2, 0x41, 0x0F, 0x11, 0x8E, 0x0, 0x02, 0x0, 0x0 };
BYTE OrigLocalData[22] = { 0x41, 0x0F, 0x11, 0x86, 0xF0, 0x01, 0x0, 0x0, 0xF2, 0x0F, 0x10, 0x4F, 0x18, 0xF2, 0x41, 0x0F, 0x11, 0x8E, 0x0, 0x02, 0x0, 0x0 };

BYTE* OldLocalCordsFunc = 0;
BYTE* NewLocalCordsFunc = 0;
BYTE* LocalData = 0;
bool LocalCordshooked = false;
void LocalCordsHookUnhook(BYTE* OrigCodeArray, SIZE_T OrigCodeSize, BYTE* NewCodeArray, SIZE_T NewCodeSize, bool state) {
    if (p_UnityPlayer == NULL) return;
    OldLocalCordsFunc = (BYTE*)p_UnityPlayer + 0x140BE5B;
    if (!LocalCordshooked && state) {
        NewLocalCordsFunc = (BYTE*)VirtualAlloc(nullptr, 128, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (OldLocalCordsFunc == NULL || NewLocalCordsFunc == NULL) return;
        LocalData = NewLocalCordsFunc;
        BYTE* NEW = NewLocalCordsFunc + 0x20;
        for (int NCA = 0; NCA < NewCodeSize; NCA++) {
            *(NEW + NCA) = NewCodeArray[NCA];
        }
        NEW += NewCodeSize;
        *(uint16_t*)&NEW[0] = 0x25FF;
        *(uint32_t*)&NEW[2] = 0x00000000;
        *(uint64_t*)&NEW[6] = (uint64_t)(OldLocalCordsFunc + OrigCodeSize); //JMP

        DWORD oldProtectT;
        BYTE* TRG = OldLocalCordsFunc;
        VirtualProtect((LPVOID)OldLocalCordsFunc, OrigCodeSize, PAGE_EXECUTE_READWRITE, &oldProtectT);
        *(uint16_t*)&TRG[0] = 0x25FF;
        *(uint32_t*)&TRG[2] = 0x00000000;
        *(uint64_t*)&TRG[6] = (uint64_t)(NewLocalCordsFunc + 0x20); //JMP
        for (int nop = 14; nop < OrigCodeSize; nop++) {
            *(TRG + nop) = 0x90;
        }
        VirtualProtect((LPVOID)OldLocalCordsFunc, OrigCodeSize, oldProtectT, &oldProtectT);
        LocalCordshooked = true;
        return;
    }
    if (LocalCordshooked && !state) {
        if (OldLocalCordsFunc == NULL) return;
        DWORD oldProtectF;
        BYTE* TRG2 = OldLocalCordsFunc;
        VirtualProtect((LPVOID)OldLocalCordsFunc, OrigCodeSize, PAGE_EXECUTE_READWRITE, &oldProtectF);
        for (int ORG = 0; ORG < OrigCodeSize; ORG++) { *(TRG2 + ORG) = OrigCodeArray[ORG]; } //WRITE ORIG BYTES
        VirtualProtect((LPVOID)OldLocalCordsFunc, OrigCodeSize, oldProtectF, &oldProtectF);
        VirtualFree(NewLocalCordsFunc, 0, MEM_RELEASE);
        NewLocalCordsFunc = NULL;
        LocalCordshooked = false;
        return;
    }
}

BYTE* OldEnemyCordsFunc = 0;
BYTE* NewEnemyCordsFunc = 0;
BYTE* PlayerDataBuffer = 0;
bool EnemyCordshooked = false;
void EnemyCordsHookUnhook(BYTE* OrigCodeArray, SIZE_T OrigCodeSize, BYTE* NewCodeArray, SIZE_T NewCodeSize, bool state) {
    if (p_UnityPlayer == NULL) return;
    OldEnemyCordsFunc = (BYTE*)p_UnityPlayer + 0x140437C;
    if (!EnemyCordshooked && state) {
        NewEnemyCordsFunc = (BYTE*)VirtualAlloc(nullptr, 512, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (OldEnemyCordsFunc == NULL || NewEnemyCordsFunc == NULL) return;
        PlayerDataBuffer = NewEnemyCordsFunc;
        BYTE* NEW = NewEnemyCordsFunc + 0x144;
        for (int NCA = 0; NCA < NewCodeSize; NCA++) {
            *(NEW + NCA) = NewCodeArray[NCA];
        }
        NEW += NewCodeSize;
        *(uint16_t*)&NEW[0] = 0x25FF;
        *(uint32_t*)&NEW[2] = 0x00000000;
        *(uint64_t*)&NEW[6] = (uint64_t)(OldEnemyCordsFunc + OrigCodeSize); //JMP

        DWORD oldProtectT;
        BYTE* TRG = OldEnemyCordsFunc;
        VirtualProtect((LPVOID)OldEnemyCordsFunc, OrigCodeSize, PAGE_EXECUTE_READWRITE, &oldProtectT);
        *(uint16_t*)&TRG[0] = 0x25FF;
        *(uint32_t*)&TRG[2] = 0x00000000;
        *(uint64_t*)&TRG[6] = (uint64_t)(NewEnemyCordsFunc + 0x144); //JMP
        for (int nop = 14; nop < OrigCodeSize; nop++) {
            *(TRG + nop) = 0x90;
        }
        VirtualProtect((LPVOID)OldEnemyCordsFunc, OrigCodeSize, oldProtectT, &oldProtectT);
        EnemyCordshooked = true;
        return;
    }
    if (EnemyCordshooked && !state) {
        if (OldEnemyCordsFunc == NULL) return;
        DWORD oldProtectF;
        BYTE* TRG2 = OldEnemyCordsFunc;
        VirtualProtect((LPVOID)OldEnemyCordsFunc, OrigCodeSize, PAGE_EXECUTE_READWRITE, &oldProtectF);
        for (int ORG = 0; ORG < OrigCodeSize; ORG++) { *(TRG2 + ORG) = OrigCodeArray[ORG]; } //WRITE ORIG BYTES
        VirtualProtect((LPVOID)OldEnemyCordsFunc, OrigCodeSize, oldProtectF, &oldProtectF);
        VirtualFree(NewEnemyCordsFunc, 0, MEM_RELEASE);
        NewEnemyCordsFunc = NULL;
        EnemyCordshooked = false;
        return;
    }
}

DWORD WINAPI CheatThread(LPVOID lpParam) {

    p_UnityPlayer = GetModuleHandle("engine.dll");

    bool local_HookEnemyCords = b_HookEnemyCords;
    bool local_HookLocalCords = b_HookLocalCords;

    b_HookEnemyCords = true;
    b_HookLocalCords = true;

    while (true) {
        if (b_HookEnemyCords != local_HookEnemyCords) {
            local_HookEnemyCords = b_HookEnemyCords;
            EnemyCordsHookUnhook(OrigEnemyData, 20, StoreEnemyData, 89, b_HookEnemyCords);
        }
        if (b_HookLocalCords != local_HookLocalCords) {
            local_HookLocalCords = b_HookLocalCords;
            LocalCordsHookUnhook(OrigLocalData, 22, StoreLocalData, 49, b_HookLocalCords);
        }
        Sleep(10);
    }
    return 0;
}
