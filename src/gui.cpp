#include "gui.h"
#include "drawMotionPlanner.h"
#include "util.h"
#include <tinyxml.h>


const GLColor bodyColor(0.1,0.1,0.1);
const GLColor selectedLinkColor(1.0,1.0,0.5);
const double sweptvolumeScale = 0.98;
const double sweptVolume_q_spacing = 0.01;
GLColor sweptvolumeColor(0.7,0.0,0.9,0.5);

ForceFieldBackend::ForceFieldBackend(RobotWorld *world)
    : SimTestBackend(world)
{
  drawForceField = 0;
  drawRobotExtras = 1;
  drawIKextras = 0;
  drawPath = 1;
  drawPlannerTree = 1;
  drawPlannerStartGoal = 1;

  drawAxes = 1;
  drawAxesLabels = 0;

  MapButtonToggle("draw_planner_tree",&drawPlannerTree);
  MapButtonToggle("draw_path",&drawPath);
  MapButtonToggle("draw_path_start_goal",&drawPlannerStartGoal);
  MapButtonToggle("draw_fancy_coordinate_axes",&drawAxes);
  MapButtonToggle("draw_fancy_coordinate_axes_labels",&drawAxesLabels);



  _mats.clear();
}


void ForceFieldBackend::Start()
{
  BaseT::Start();


  //disable higher drawing functions
  //drawBBs,drawPoser,drawDesired,drawEstimated,drawContacts,drawWrenches,drawExpanded,drawTime,doLogging
  drawPoser = 0;

  //settings["desired"]["color"][0] = 1;
  //settings["desired"]["color"][1] = 0;
  //settings["desired"]["color"][2] = 0;
  //settings["desired"]["color"][3] = 0.5;
  //camera.dist = 4;
  //viewport.n = 0.1;
  //viewport.f = 100;
  //viewport.setLensAngle(DtoR(90.0));
  //DisplayCameraTarget();
  //Camera::CameraController_Orbit camera;
  //Camera::Viewport viewport;
  show_frames_per_second = true;

}

//############################################################################
//############################################################################


void ForceFieldBackend::RenderWorld()
{
  DEBUG_GL_ERRORS()
  drawDesired=0;

  BaseT::RenderWorld();

  glEnable(GL_BLEND);
  allWidgets.Enable(&allRobotWidgets,drawPoser==1);
  allWidgets.DrawGL(viewport);
  vector<ViewRobot> viewRobots = world->robotViews;

  Robot *robot = world->robots[0];
  ViewRobot *viewRobot = &viewRobots[0];

  //############################################################################
  // Visualize
  //
  // drawrobotextras      : COM, skeleton
  // drawikextras         : contact links, contact directions
  // drawforcefield       : a flow/force field on R^3
  // drawpath             : swept volume along path
  // drawplannerstartgoal : start/goal configuration of motion planner
  // drawplannertree      : Cspace tree visualized as COM tree in W
  // drawaxes             : fancy coordinate axes
  // drawaxeslabels       : labelling of the coordinate axes [needs fixing]
  //############################################################################

  if(drawRobotExtras) GLDraw::drawRobotExtras(viewRobot);
  if(drawIKextras) GLDraw::drawIKextras(viewRobot, robot, _constraints, _linksInCollision, selectedLinkColor);
  if(drawForceField) GLDraw::drawUniformForceField();
  if(drawPath) GLDraw::drawPathSweptVolume(robot, _mats, _appearanceStack);
  if(drawPlannerStartGoal) GLDraw::drawPlannerStartGoal(robot, planner_p_init, planner_p_goal);
  if(drawPlannerTree) GLDraw::drawPlannerTree(_stree);
  if(drawAxes) drawCoordWidget(1); //void drawCoordWidget(float len,float axisWidth=0.05,float arrowLen=0.2,float arrowWidth=0.1);
  if(drawAxesLabels) GLDraw::drawAxesLabels(viewport);

}//RenderWorld

void ForceFieldBackend::Load()
{
}
bool ForceFieldBackend::Save()
{

   // string _robotname;
   // SerializedTree _stree;
   // //swept volume
   // Config planner_p_init, planner_p_goal;
   // std::vector<std::vector<Matrix4> > _mats;
   // vector<GLDraw::GeometryAppearance> _appearanceStack;
  std::string date = util::GetCurrentTimeString();
  string out = "../data/gui/state_"+date+".xml";
  std::cout << "saving data to "<<out << std::endl;

  TiXmlElement node("GUI");
  {
    TiXmlElement c("robot");

    //###################################################################
    {
      TiXmlElement cc("name");
      stringstream ss;
      ss<<_robotname;
      TiXmlText text(ss.str().c_str());
      cc.InsertEndChild(text);
      c.InsertEndChild(cc);
    }
    {
      TiXmlElement cc("config");
      stringstream ss;
      ss<<planner_p_init;
      ss<<planner_p_goal;

      TiXmlText text(ss.str().c_str());
      cc.InsertEndChild(text);
      c.InsertEndChild(cc);
    }
    //###################################################################
    node.InsertEndChild(c);
  }
  return true;

}

//############################################################################
//############################################################################

#include <KrisLibrary/graph/Tree.h>

void ForceFieldBackend::VisualizePlannerTree(const SerializedTree &tree)
{
//typedef std::pair<Vector, std::vector<Vector> > SerializedTreeNode;
//typedef std::vector< SerializedTreeNode > SerializedTree;

  _stree = tree;
  drawPlannerTree=1;

}
void ForceFieldBackend::VisualizeStartGoal(const Config &p_init, const Config &p_goal)
{
  drawPlannerStartGoal = 1;
  planner_p_init = p_init;
  planner_p_goal = p_goal;
}
void ForceFieldBackend::VisualizePathSweptVolumeAtPosition(const Config &q)
{
  Robot *robot = world->robots[0];
  robot->UpdateConfig(q);
  std::vector<Matrix4> mats_config;

  for(size_t i=0;i<robot->links.size();i++) {
    Matrix4 mat = robot->links[i].T_World;
    mats_config.push_back(mat);
  }
  _mats.push_back(mats_config);
}

void ForceFieldBackend::VisualizePathSweptVolume(const MultiPath &path)
{
  Robot *robot = world->robots[0];

  Config qt;
  path.Evaluate(0, qt);

  double dstep = 0.01;
  double d = 0;

  while(d <= 1)
  {
    d+=dstep;
    Config qtn;
    path.Evaluate(d, qtn);
    if((qt-qtn).norm() >= sweptVolume_q_spacing)
    {
      VisualizePathSweptVolumeAtPosition(qtn);
      qt = qtn;
    }
  }
  //for(double d = 0; d <= path.Duration()+dstep; d+=dstep)
  //{
  //  while 
  //  VisualizePathSweptVolumeAtPosition(path, d);
  //}

  if(DEBUG) std::cout << "[SweptVolume] #waypoints " << _mats.size() << std::endl;
  _appearanceStack.clear();
  _appearanceStack.resize(robot->links.size());

  for(size_t i=0;i<robot->links.size();i++) {
    GLDraw::GeometryAppearance& a = *robot->geomManagers[i].Appearance();
    //a.SetColor(sweptvolumeColor);
    _appearanceStack[i]=a;
  }
  if(DEBUG) std::cout << "[SweptVolume] #geometries " << _appearanceStack.size() << std::endl;
  drawPath = 1;

}







void ForceFieldBackend::VisualizePathSweptVolume(const KinodynamicMilestonePath &path)
{
  Robot *robot = world->robots[0];

  Config qt;
  path.Eval(0, qt);

  double dstep = 0.01;
  double d = 0;

  while(d <= 1)
  {
    d+=dstep;
    Config qtn;
    path.Eval(d, qtn);
    //std::cout << d << qtn << std::endl;
    if((qt-qtn).norm() >= sweptVolume_q_spacing)
    {
      VisualizePathSweptVolumeAtPosition(qtn);
      qt = qtn;
    }
  }
  _appearanceStack.clear();
  _appearanceStack.resize(robot->links.size());

  for(size_t i=0;i<robot->links.size();i++) {
    GLDraw::GeometryAppearance& a = *robot->geomManagers[i].Appearance();
    _appearanceStack[i]=a;
  }
  drawPath = 1;
}
void ForceFieldBackend::VisualizePathSweptVolume(const std::vector<Config> &keyframes)
{
  Robot *robot = world->robots[0];

  for(int i = 0; i < keyframes.size(); i++)
  {
    VisualizePathSweptVolumeAtPosition(keyframes.at(i));
  }

  _appearanceStack.clear();
  _appearanceStack.resize(robot->links.size());

  for(size_t i=0;i<robot->links.size();i++) {
    GLDraw::GeometryAppearance& a = *robot->geomManagers[i].Appearance();
    _appearanceStack[i]=a;
  }
  drawPath = 1;
}
void ForceFieldBackend::SetIKConstraints( vector<IKGoal> constraints, string robotname){
  _constraints = constraints;
  _robotname = robotname;
  drawIKextras = 1;
}
void ForceFieldBackend::SetIKCollisions( vector<int> linksInCollision )
{
  _linksInCollision = linksInCollision;
  drawIKextras = 1;
}
bool ForceFieldBackend::OnCommand(const string& cmd,const string& args){
  BaseT::OnCommand(cmd,args);
  if(cmd=="advance") {
    //ODERobot *robot = sim.odesim.robot(0);
    //std::cout << "Force" << std::endl;
    //std::cout << robot->robot.name << std::endl;
    //Config q;
    //robot->GetConfig(q);
    //double px,py,pz;
    //double fx,fy,fz;
    //fx = 10.0;
    //fy = 0.0;
    //fz = 10.0;
    //px = 0.0;
    //py = 0.0;
    //pz = 0.0;
    //dBodyAddForceAtPos(robot->body(7),fx,fy,fz,px,py,pz);
  }
  return true;
}

GLUIForceFieldGUI::GLUIForceFieldGUI(GenericBackendBase* _backend,RobotWorld* _world)
    :BaseT(_backend,_world)
{
}
bool GLUIForceFieldGUI::Initialize()
{
  if(!BaseT::Initialize()) return false;
  
  GLUI_Panel* panel;
  GLUI_Checkbox* checkbox;

  panel = glui->add_rollout("Motion Planning");
  checkbox = glui->add_checkbox_to_panel(panel, "Draw Planning Tree");
  AddControl(checkbox,"draw_planner_tree");
  checkbox->set_int_val(1);

  checkbox = glui->add_checkbox_to_panel(panel, "Draw Swept Volume");
  AddControl(checkbox,"draw_path");
  checkbox->set_int_val(1);

  checkbox = glui->add_checkbox_to_panel(panel, "Draw Start Goal Config");
  AddControl(checkbox,"draw_path_start_goal");
  checkbox->set_int_val(1);

    //AddControl(glui->add_button_to_panel(panel,"Save state"),"save_state");



  panel = glui->add_rollout("Fancy Decorations");
  checkbox = glui->add_checkbox_to_panel(panel, "Draw Coordinate Axes");
  AddControl(checkbox,"draw_fancy_coordinate_axes");
  checkbox->set_int_val(1);

  checkbox = glui->add_checkbox_to_panel(panel, "Draw Coordinate Axes Labels[TODO]");
  AddControl(checkbox,"draw_fancy_coordinate_axes_labels");
  checkbox->set_int_val(0);

  UpdateGUI();
  return true;

}



