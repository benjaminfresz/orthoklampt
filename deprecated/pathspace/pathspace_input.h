#pragma once
#include "planner/strategy/strategy_input.h"
#include "planner/planner_input.h"
#include "planner/cspace/cspace_input.h"
#include <KrisLibrary/robotics/RobotKinematics3D.h> //Config

class PathSpaceInput{

  public:
    PathSpaceInput();
    PathSpaceInput(const PlannerInput&, int level_);

    Config q_init;
    Config q_goal;
    Config dq_init;
    Config dq_goal;

    Config qMin;
    Config qMax;

    uint robot_idx;
    uint robot_inner_idx;
    uint robot_outer_idx;

    int freeFloating;

    Config se3min;
    Config se3max;

    std::string name_algorithm;
    std::string name_sampler;

    std::string type;
    uint level;

    double epsilon_goalregion;
    double max_planning_time;
    double timestep_min;
    double timestep_max;

    bool enableSufficiency{false};
    bool fixedBase{false};
    bool kinodynamic{false};
    Config uMin;
    Config uMax;

    const CSpaceInput& GetCSpaceInput();
    const StrategyInput& GetStrategyInput();
    PathSpaceInput* GetNextLayer() const;
    void SetNextLayer(PathSpaceInput* next);
    friend std::ostream& operator<< (std::ostream&, const PathSpaceInput&);

  private:
    CSpaceInput* cin;
    StrategyInput* sin;
    PathSpaceInput* next_layer;
};
