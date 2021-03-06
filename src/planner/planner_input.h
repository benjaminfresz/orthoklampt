#pragma once
#include "file_io.h"
#include <string>
#include <KrisLibrary/robotics/RobotKinematics3D.h> //Config

struct Layer{
  int level;
  int inner_index;
  int outer_index;
  double cspace_constant;
  Config q_init;
  Config q_goal;
  std::string type;
};

struct Stratification{
  std::vector<Layer> layers;
};

class CSpaceInput;
class StrategyInput;

class PlannerInput{
  public:
    //general input for any planner method (fixed)
    Config q_init;
    Config q_goal;
    Config dq_init;
    Config dq_goal;

    Config qMin;
    Config qMax;

    Config se3min;
    Config se3max;

    uint robot_idx;
    int freeFloating;

    std::string name_algorithm;
    std::string name_sampler;
    std::string environment_name;

    double epsilon_goalregion;
    double max_planning_time;
    double timestep_min;
    double timestep_max;

    bool smoothPath{false};
    double pathSpeed{1};
    double pathWidth{1};
    double pathBorderWidth{0.01};

    bool kinodynamic{false};
    Config uMin;
    Config uMax;

    std::vector<Stratification> stratifications;

    bool Load(const char* file);
    bool Load(TiXmlElement *node, int hierarchy = 0);
    void SetDefault();
    void ExtractHierarchy(TiXmlElement *node, int hierarchy_index);
    const CSpaceInput& GetCSpaceInput();
    const StrategyInput& GetStrategyInput();

    friend std::ostream& operator<< (std::ostream& out, const PlannerInput& pin) ;
  private:
    CSpaceInput* cin;
    StrategyInput* sin;
};

/// @brief multiple plannerinputs (to use several algorithms inside GUI)
struct PlannerMultiInput{
  std::vector<PlannerInput*> inputs;
  bool Load(const char* file);
  bool Load(TiXmlElement *node);
  std::vector<std::string> GetAlgorithms(bool kinodynamic);
};

