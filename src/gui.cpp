#include "gui.h"
#include <GL/freeglut.h>
#include "drawMotionPlanner.h"

const GLColor bodyColor(0.1,0.1,0.1);
const GLColor selectedLinkColor(1.0,1.0,0.5);
const double sweptvolumeScale = 0.98;
const double sweptVolume_q_spacing = 0.01;
GLColor sweptvolumeColor(0.7,0.0,0.9,0.5);

ForceFieldBackend::ForceFieldBackend(RobotWorld *world)
    : SimTestBackend(world)
{
  drawForceField = false;
  drawRobotExtras = true;
  drawIKextras = false;
  drawAxesLabels = false;
  drawPath = false;
  drawPlannerTree = false;
  drawPlannerStartGoal = false;
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
  this->drawDesired=false;

  BaseT::RenderWorld();

  glEnable(GL_BLEND);
  allWidgets.Enable(&allRobotWidgets,drawPoser==1);
  allWidgets.DrawGL(viewport);

  vector<ViewRobot> viewRobots = world->robotViews;

  Robot *robot = world->robots[0];
  ViewRobot *viewRobot = &viewRobots[0];

  //############################################################################
  // Robot extras: COM, skeleton
  //############################################################################

  if(drawRobotExtras){
    GLDraw::drawRobotExtras(viewRobot);
  }

  //############################################################################
  // IK extras: contact links, contact directions
  //############################################################################

  if(drawIKextras){
    GLDraw::drawIKextras(viewRobot, robot, _constraints, _linksInCollision, selectedLinkColor);
  }

  //############################################################################
  // visualize a flow/force field on R^3
  //############################################################################

  if(drawForceField){
    GLDraw::drawUniformForceField();
  }

  //############################################################################
  // Visualize swept volume along path
  //############################################################################
  if(drawPath){
    GLDraw::drawPathSweptVolume(robot, _mats, _appearanceStack);
  }

  if(drawPlannerStartGoal){

    GLColor colorInit(0,1,0);
    GLColor colorGoal(1,0,0);
    double scale = 1.01;

    Robot *robot = world->robots[0];
    
    robot->UpdateConfig(planner_p_goal);
    for(uint j=0;j<robot->links.size();j++) {
      if(robot->IsGeometryEmpty(j)) continue;
      Matrix4 mat = robot->links[j].T_World;
      glPushMatrix();
      glMultMatrix(mat);
      glScalef(scale, scale, scale);
      GLDraw::GeometryAppearance& a = *robot->geomManagers[j].Appearance();
      a.SetColor(colorGoal);
      a.DrawGL();
      glPopMatrix();
    }

    robot->UpdateConfig(planner_p_init);
    for(uint j=0;j<robot->links.size();j++) {
      if(robot->IsGeometryEmpty(j)) continue;
      Matrix4 mat = robot->links[j].T_World;
      glPushMatrix();
      glMultMatrix(mat);
      glScalef(scale, scale, scale);
      GLDraw::GeometryAppearance& a = *robot->geomManagers[j].Appearance();
      a.SetColor(colorInit);
      a.DrawGL();
      glPopMatrix();
    }
  }
  if(drawPlannerTree){
    uint nearestNode = 0;
    double bestD = dInf;
    for(uint i = 0; i < _stree.size(); i++){
      SerializedTreeNode node = _stree.at(i);
      double d = node.cost_to_goal;
      if(d<bestD){
        bestD = d;
        nearestNode = i;
      }
    }
    for(uint i = 0; i < _stree.size(); i++){
      //std::cout << "Tree GUI:" << tree.at(0).first << std::endl;
      SerializedTreeNode node = _stree.at(i);
      Vector3 pos(node.position(0),node.position(1),node.position(2));
      Vector3 rot(node.position(3),node.position(4),node.position(5));

      std::vector<Vector3> dirs = node.directions;

      glDisable(GL_LIGHTING);
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      
      //compute color from cost_to_goal
      double d = node.cost_to_goal;
      double shade = exp(-d*d/0.5); //\in [0,1]
      GLColor color(shade,0,1.0-shade);

      color.setCurrentGL();

      glPushMatrix();
      glTranslate(pos);
      //glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,cForce);

      if(i==nearestNode){
        GLColor cNearest(1,1,0);
        cNearest.setCurrentGL();
        glPointSize(15.0);                                     
        //std::cout << "Nearest Node " << node.position << " d=" <<bestD << std::endl;
      }else{
        glPointSize(5.0);                                     
      }
      drawPoint(Vector3(0,0,0));
      //double linewidth = 0.01;
      //for(uint j = 0; j < dirs.size(); j++){
      //  Vector3 dir(dirs.at(j)[0], dirs.at(j)[1], dirs.at(j)[2]);
      //  drawCylinder(dir,linewidth);
      //}

      glPushMatrix();

      glPopMatrix();
      glPopMatrix();
      glEnable(GL_LIGHTING);
    }

  }
  //Draw fancy coordinate axes
  //void drawCoordWidget(float len,float axisWidth=0.05,float arrowLen=0.2,float arrowWidth=0.1);
  drawCoordWidget(1);

  //glRasterPos2i(100, 120);
  //glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
  //glPushMatrix();
  //glTranslatef(1,0,0);
  ////glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*)"some text");
  //glutStrokeString(GLUT_STROKE_ROMAN, (unsigned char*) "Xl");
  //glPopMatrix();

  if(drawAxesLabels)
  {
    //TODO: does not support scale
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((double)viewport.x, (double)viewport.y,
        (double)viewport.w, (double)viewport.h,
        -1000., 1000.);
    glTranslated(0., 0., 0.);
    glMatrixMode(GL_MODELVIEW);

    double l = 0.5;
    double o = 0 ;

    double cx = 0;
    double cy = 0;
    double xx, xy, yx, yy , zx, zy;

    float fvViewMatrix[ 16 ];
    glGetFloatv( GL_MODELVIEW_MATRIX, fvViewMatrix );
    glLoadIdentity();

    xx = l * fvViewMatrix[0];
    xy = l * fvViewMatrix[1];
    yx = l * fvViewMatrix[4];
    yy = l * fvViewMatrix[5];
    zx = l * fvViewMatrix[8];
    zy = l * fvViewMatrix[9];

    double lineWidth = 0.1;
    //double lineWidth = 0.1;
    glLineWidth(lineWidth);
    //glColor4ubv(color);

    glBegin(GL_LINES);
    glVertex2d(cx, cy);
    glVertex2d(cx + xx, cy + xy);
    glVertex2d(cx, cy);
    glVertex2d(cx + yx, cy + yy);
    glVertex2d(cx, cy);
    glVertex2d(cx + zx, cy + zy);
    glEnd();
    glRasterPos2d(cx + xx + o, cy + xy + o);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*) "X");
    glRasterPos2d(cx + yx + o, cy + yy + o);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*) "Y");
    glRasterPos2d(cx + zx + o, cy + zy + o);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*) "Z");
  }


}//RenderWorld

//############################################################################
//############################################################################


#include <KrisLibrary/graph/Tree.h>

void ForceFieldBackend::VisualizePlannerTree(const SerializedTree &tree)
{
//typedef std::pair<Vector, std::vector<Vector> > SerializedTreeNode;
//typedef std::vector< SerializedTreeNode > SerializedTree;

  _stree = tree;
  drawPlannerTree=true;

}
void ForceFieldBackend::VisualizeStartGoal(const Config &p_init, const Config &p_goal)
{
  drawPlannerStartGoal = true;
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
  drawPath = true;

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
    std::cout << d << qtn << std::endl;
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
  drawPath = true;
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
  drawPath = true;
}
void ForceFieldBackend::SetIKConstraints( vector<IKGoal> constraints, string robotname){
  _constraints = constraints;
  _robotname = robotname;
  drawIKextras = true;
}
void ForceFieldBackend::SetIKCollisions( vector<int> linksInCollision )
{
  _linksInCollision = linksInCollision;
  drawIKextras = true;
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
