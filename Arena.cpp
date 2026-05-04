#include <dlfcn.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>
#include <unistd.h>
#include "RobotBase.h"

struct RobotData {
    RobotBase* ptr;
    void* handle;
    char icon;
};

int rows, cols, maxRounds, flameCount, pitCount, moundCount;
float sleepInterval;
bool live;
std::vector<RobotData> robots;
char grid[100][100]; // Fixed size for simplicity

void load_config(std::string filename) {
    std::ifstream file(filename);
    std::string line, key;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> key;
        if (key == "Arena_Size:") {
            char x; ss >> rows >> cols;
        } else if (key == "Max_Rounds:") ss >> maxRounds;
        else if (key == "Sleep_interval:") ss >> sleepInterval;
        else if (key == "Game_State_Live:") {
            std::string val; ss >> val; live = (val == "true");
        } else if (key == "Flamethrowers:") ss >> flameCount;
        else if (key == "Pits:") ss >> pitCount;
        else if (key == "Mounds:") ss >> moundCount;
    }
}

void setup_arena() {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) grid[i][j] = '.';

    auto place = [&](int count, char type) {
        for (int i = 0; i < count; i++) {
            int r = rand() % rows, c = rand() % cols;
            if (grid[r][c] == '.') grid[r][c] = type;
            else i--;
        }
    };
    place(flameCount, 'F');
    place(pitCount, 'P');
    place(moundCount, 'M');
}

void do_damage(RobotBase* source, RobotBase* victim) {
    int dmg = 0;
    WeaponType w = source->get_weapon();
    if (w == railgun) dmg = rand() % 11 + 10;
    else if (w == hammer) dmg = rand() % 11 + 50;
    else if (w == grenade) {
        dmg = rand() % 31 + 10;
        source->decrement_grenades();
    }
    else if (w == flamethrower) dmg = rand() % 21 + 30;

    float factor = 1.0 - (victim->get_armor() * 0.1);
    victim->take_damage((int)(dmg * factor));
    victim->reduce_armor(1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    srand(time(0));
    load_config(argv[1]);
    setup_arena();

    std::vector<std::string> robotFiles = {"Robot_Ratboy.cpp", "Robot_Flame_e_o.cpp"}; 
    char icons[] = {'@', '#', '$', '%', '&', '!'};

    for (size_t i = 0; i < robotFiles.size(); i++) {
        std::string so = "./Robot_" + std::to_string(i) + ".so";
        std::string cmd = "g++ -shared -fPIC -o " + so + " " + robotFiles[i] + " RobotBase.o -I. -std=c++20";
        if (system(cmd.c_str()) != 0) continue;

        void* h = dlopen(so.c_str(), RTLD_LAZY);
        if (!h) continue;
        auto create = (RobotBase* (*)())dlsym(h, "create_robot");
        auto summary = (const char* (*)())dlsym(h, "robot_summary");
        
        if (create && summary && strlen(summary()) <= 50) {
            RobotBase* r = create();
            r->set_boundaries(rows, cols);
            int startR, startC;
            do { startR = rand() % rows; startC = rand() % cols; } while (grid[startR][startC] != '.');
            r->move_to(startR, startC);
            robots.push_back({r, h, icons[i]});
        }
    }

    for (int currRound = 0; currRound < maxRounds; currRound++) {
        std::cout << "\n======= ROUND " << currRound << " =======\n";
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                bool botHere = false;
                for (auto& rb : robots) {
                    int rr, rc; rb.ptr->get_current_location(rr, rc);
                    if (rr == i && rc == j) {
                        std::cout << (rb.ptr->get_health() <= 0 ? 'X' : rb.icon) << " ";
                        botHere = true; break;
                    }
                }
                if (!botHere) std::cout << grid[i][j] << " ";
            }
            std::cout << "\n";
        }

        int aliveCount = 0;
        for (auto& rb : robots) if (rb.ptr->get_health() > 0) aliveCount++;
        if (aliveCount <= 1) break;

        for (auto& rb : robots) {
            if (rb.ptr->get_health() <= 0) continue;

            int rDir;
            rb.ptr->get_radar_direction(rDir);
            std::vector<RadarObj> scan;
            rb.ptr->process_radar_results(scan);

            int sr, sc;
            if (rb.ptr->get_shot_location(sr, sc)) {
                for (auto& target : robots) {
                    if (target.ptr == rb.ptr || target.ptr->get_health() <= 0) continue;
                    int tr, tc; target.ptr->get_current_location(tr, tc);
                    if (std::abs(tr - sr) <= 1 && std::abs(tc - sc) <= 1) do_damage(rb.ptr, target.ptr);
                }
            } else {
                int mDir, mDist;
                rb.ptr->get_move_direction(mDir, mDist);
                if (mDir > 0 && mDir < 9) {
                    int r, c; rb.ptr->get_current_location(r, c);
                    for (int s = 0; s < mDist; s++) {
                        int nr = r + directions[mDir].first;
                        int nc = c + directions[mDir].second;
                        if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) break;
                        if (grid[nr][nc] == 'M') break;
                        r = nr; c = nc;
                        if (grid[r][c] == 'P') { rb.ptr->disable_movement(); break; }
                        if (grid[r][c] == 'F') { rb.ptr->take_damage(rand() % 21 + 30); }
                    }
                    rb.ptr->move_to(r, c);
                }
            }
        }
        if (live) usleep(sleepInterval * 1000000);
    }

    for (auto& rb : robots) {
        delete rb.ptr;
        dlclose(rb.handle);
    }
    return 0;
}