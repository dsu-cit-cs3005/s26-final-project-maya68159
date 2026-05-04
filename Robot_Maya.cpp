#include "Robot_Maya.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

Robot_Maya::Robot_Maya()
    : RobotBase(3, 4, railgun) 
{
    m_name = "MayaBot";
    m_character = 'Y'; // Changed from 'M' to avoid Mound confusion
}

void Robot_Maya::get_radar_direction(int& radar_direction) {
    tick++;
    radar_direction = 3; 

    if (tick % 4 == 0) radar_direction = 2;
    else if (tick % 6 == 0) radar_direction = 4;
}

void Robot_Maya::process_radar_results(const std::vector<RadarObj>& radar_results) {
    m_radar_results = radar_results;
    if (!radar_results.empty()) {
        has_target = true;
        target_row = radar_results[0].m_row;
        target_col = radar_results[0].m_col;
    } else {
        has_target = false;
    }
}

bool Robot_Maya::get_shot_location(int& shot_row, int& shot_col) {
    if (!has_target) return false;

    int r, c;
    get_current_location(r, c);

    int dr = std::abs(target_row - r);
    int dc = std::abs(target_col - c);
    int distance = dr + dc;

    int chance = std::max(1, 5 - distance / 2);
    int roll = std::rand() % 5;

    if (roll < chance) {
        shot_row = target_row;
        shot_col = target_col;
        return true;
    }
    return false;
}

void Robot_Maya::get_move_direction(int& direction, int& distance) {
    int roll = std::rand() % 10;

    if (roll < 6) {
        direction = preferred_dir;
        distance = 1;
    } else if (roll < 8) {
        direction = 4;
        distance = 1;
    } else {
        direction = 0;
        distance = 0;
    }
}

RobotBase* create_robot() {
    return new Robot_Maya();
}

const char* robot_summary() {
    return "Railgun scout; erratic movement, precise shots.";
}