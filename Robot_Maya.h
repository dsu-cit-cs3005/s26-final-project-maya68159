#pragma once

#include "RobotBase.h"
#include <vector>

class Robot_Maya : public RobotBase {
    private:
        bool has_target = false;
        int target_row = -1;
        int target_col = -1;
        int preferred_dir = 3;
        std::vector<RadarObj> m_radar_results;
        int tick = 0;
    public:
        Robot_Maya();

        void get_radar_direction(int& radar_direction) override;
        void process_radar_results(const std::vector<RadarObj>& radar_results) override;
        bool get_shot_location(int& shot_row, int& shot_col) override;
        void get_move_direction(int& direction, int& distance) override;
};

extern "C" {
    RobotBase* create_robot();
    const char* robot_summary();
}