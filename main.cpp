#include "memory.h"
#include "utils.h"
#include <iostream>
#include <Windows.h>

using namespace std;

namespace Config {
    const uint16_t AimRadius = 125;
    const uint16_t CONFIG_KEY = VK_SHIFT;
    const BOOL team_check = TRUE;
    const CHAR bone_ids[] = {3,4,5,6}; // target bones
}

// update these
namespace Offsets { 
    const uintptr_t dwLocalPlayerPawn = 0x186BDF8;
    const uintptr_t dwViewMatrix = 0x1A82740;
    const uintptr_t dwEntityList = 0x1A176C8;
 
    const uintptr_t m_hPlayerPawn = 0x80C;
    const uintptr_t m_iTeamNum = 0x3E3;
    const uintptr_t m_iHealth = 0x344;
    const uintptr_t m_pGameSceneNode = 0x328;
 
    const uintptr_t m_pBoneArray = 496;
}

class Player {
private:
    Memory& mem;
    uintptr_t address;
public:
    Player(Memory& memory, uintptr_t addr) : mem(memory), address(addr) {}

    uint16_t GetTeam() const {
        return mem.Read<uint16_t>(address + Offsets::m_iTeamNum);
    }

    uint16_t GetHealth() const {
        return mem.Read<uint16_t>(address + Offsets::m_iHealth);
    }

    uintptr_t GetBoneArrayPtr() const {
        const uintptr_t gameScene = mem.Read<uintptr_t>(address + Offsets::m_pGameSceneNode);
        return mem.Read<uintptr_t>(gameScene + Offsets::m_pBoneArray);
    }

    Vec3 GetBonePosition(int boneId) const {
        return mem.Read<Vec3>(GetBoneArrayPtr() + boneId * 32);
    }

    uintptr_t Address() const {
        return address;
    }
};

int main() {
    cout << "[+] Searching for cs2.exe\n";
    Memory mem("cs2.exe");

    if (!mem.IsValid()) {
        cout << "[!] Couldn't find the cs2.exe process.\n";
        return 1;
    }
    
    cout << "[+] Found cs2.exe\n";

    const uintptr_t client = mem.GetModuleAddress("client.dll");

    if (!client) {
        cout << "[!] Couldn't find client.dll\n";
        return 1;
    }
    
    cout << "[+] client.dll @ 0x" << hex << client << dec << "\n";

    Vec2 SCREEN_CENTER = screenSize;
    SCREEN_CENTER.x /=2;
    SCREEN_CENTER.y /=2;

    cout << "[+] Aim radius: " << Config::AimRadius << "\n";

    cout << "[+] Team check: " << Config::team_check << "\n";

    cout << "[+] Targetting " << sizeof(Config::bone_ids) << " bones\n";

    cout << "\n[+] Hold Config::CONFIG_KEY to enable.\n[+] https://github.com/im-razvan/CS2_Aimbot\n";

    while(true) {
        Sleep(1);
        // ^ removing this will break the mouse movement

        if((GetKeyState(Config::CONFIG_KEY) & 0x8000) == 0) {
            continue;
        }

        const uintptr_t localPlayerAddr = mem.Read<uintptr_t>(client + Offsets::dwLocalPlayerPawn);
        if(!localPlayerAddr) continue;

        Player localPlayer(mem, localPlayerAddr);
        const uint16_t localPlayerTeam = localPlayer.GetTeam();

        const uintptr_t ent_list = mem.Read<uintptr_t>(client + Offsets::dwEntityList);
        if(!ent_list) continue;

        ViewMatrix_ ViewMatrix = mem.Read<ViewMatrix_>(client + Offsets::dwViewMatrix);

        float closestDistance = FLT_MAX;
        Vec2 closestPoint;

        for(int i=1; i<=64; i++) {
            const uintptr_t entry_ptr = mem.Read<uintptr_t>(ent_list + (8 * (i & 0x7FFF) >> 9) + 16);
            if(!entry_ptr) continue;
            const uintptr_t controller_ptr = mem.Read<uintptr_t>(entry_ptr + 120 * (i & 0x1FF));
            if(!controller_ptr) continue;
            const uintptr_t controller_pawn_ptr = mem.Read<uintptr_t>(controller_ptr + Offsets::m_hPlayerPawn);
            if(!controller_pawn_ptr) continue;
            const uintptr_t list_entry_ptr = mem.Read<uintptr_t>(ent_list + 0x8 * ((controller_pawn_ptr & 0x7FFF) >> 9) + 16);
            if(!list_entry_ptr) continue;

            const uintptr_t player_pawn = mem.Read<uintptr_t>(list_entry_ptr + 120 * (controller_pawn_ptr & 0x1FF));
            if(!player_pawn || player_pawn == localPlayer.Address()) continue;

            Player enemy(mem, player_pawn);
            
            if(Config::team_check && enemy.GetTeam() == localPlayerTeam) continue;
            if(!(enemy.GetHealth() > 0)) continue;

            for(int k = 0; k < sizeof(Config::bone_ids); k++) {
                const Vec3 playerBonePOS3D = enemy.GetBonePosition(Config::bone_ids[k]);
                Vec2 playerBoneW2sPOS;

                if(!WorldToScreen(ViewMatrix.Matrix, playerBonePOS3D, playerBoneW2sPOS)) break;
                float cdist = playerBoneW2sPOS.distance_to(SCREEN_CENTER);
                if(closestDistance > cdist) {
                    closestDistance = cdist;
                    closestPoint = playerBoneW2sPOS;
                }
            }
        }

        if(closestDistance > Config::AimRadius) continue;

        // cout << closestDistance <<endl;

        closestPoint.x -= SCREEN_CENTER.x;
        closestPoint.y -= SCREEN_CENTER.y;

        move_mouse(closestPoint);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH){
		AllocConsole();
		SetConsoleTitleA("Version 1.01");
		freopen("CONOUT$", "w", stdout);
		CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), 0, 0, 0);
    }

    return TRUE;
}