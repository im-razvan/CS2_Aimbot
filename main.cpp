#include "memory.h"
#include "utils.h"
#include <thread>
#include <iostream>

using namespace std;

namespace Config {
    const uint16_t AimRadius = 125;
    const uint16_t CONFIG_KEY = VK_SHIFT;
}

namespace Offsets {
    const uintptr_t dwLocalPlayerPawn = 0x1869D88;
    const uintptr_t dwViewMatrix = 0x1A80870;
    const uintptr_t dwEntityList = 0x1A157C8;

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
        cin.get();
        return 1;
    }
    
    cout << "[+] Found cs2.exe\n";

    const uintptr_t client = mem.GetModuleAddress("client.dll");

    if (!client) {
        cout << "[!] Couldn't find client.dll\n";
        cin.get();
        return 1;
    }
    
    cout << "[+] client.dll @ 0x" << hex << client << dec << "\n";

    Vec2 SCREEN_CENTER = screenSize;
    SCREEN_CENTER.x /=2;
    SCREEN_CENTER.y /=2;

    cout << "[+] Aim radius: " << Config::AimRadius << "\n"; 

    cout << "\n[+] Hold Config::CONFIG_KEY to enable.\n[+] Made by im-razvan\n";

    while(true) {
        this_thread::sleep_for(chrono::milliseconds(1));
        // ^ removing this will break the mouse movement

        if((GetKeyState(Config::CONFIG_KEY) & 0x8000) == 0) {
            continue;
        }

        const uintptr_t localPlayerAddr = mem.Read<uintptr_t>(client + Offsets::dwLocalPlayerPawn);
        if(!localPlayerAddr) continue;

        Player localPlayer(mem, localPlayerAddr);
        const uint16_t localPlayerTeam = localPlayer.GetTeam();

        ViewMatrix_ ViewMatrix = mem.Read<ViewMatrix_>(client + Offsets::dwViewMatrix);

        float closestDistance = FLT_MAX;
        Vec2 closestPoint;

        const uintptr_t ent_list = mem.Read<uintptr_t>(client + Offsets::dwEntityList);
        if(!ent_list) continue;

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
            
            if(enemy.GetTeam() == localPlayerTeam) continue; // Comment this line to disable team check
            if(!(enemy.GetHealth() > 0)) continue;

            for(int boneid = 1; boneid <= 6; boneid++) { // first 6 bones should be okay
                const Vec3 playerBonePOS3D = enemy.GetBonePosition(boneid);
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