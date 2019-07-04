#include "elements/plannerdata_vertex_annotated.h"
#include "QuotientGraphSparse.h"
#include <ompl/base/spaces/SO2StateSpace.h>
#include <ompl/base/spaces/SO3StateSpace.h>
#include <ompl/util/Time.h>
#include <queue>

using namespace og;
using namespace ob;

template <class T>
MotionExplorer<T>::MotionExplorer(std::vector<ob::SpaceInformationPtr> &siVec, std::string type)
  : ob::Planner(siVec.back(), type), siVec_(siVec)
{
    T::resetCounter();
    for (unsigned int k = 0; k < siVec_.size(); k++)
    {
        og::Quotient *parent = nullptr;
        if (k > 0)
            parent = quotientSpaces_.back();

        T *ss = new T(siVec_.at(k), parent);
        quotientSpaces_.push_back(ss);
        quotientSpaces_.back()->SetLevel(k);
    }
    if (DEBUG)
        std::cout << "Created hierarchy with " << siVec_.size() << " levels." << std::endl;
}

template <class T>
MotionExplorer<T>::~MotionExplorer()
{
}

template <class T>
void MotionExplorer<T>::setup()
{
    Planner::setup();
    for (unsigned int k = 0; k < quotientSpaces_.size(); k++)
    {
        quotientSpaces_.at(k)->setup();
    }
}

template <class T>
void MotionExplorer<T>::clear()
{
    Planner::clear();

    for (unsigned int k = 0; k < quotientSpaces_.size(); k++)
    {
        quotientSpaces_.at(k)->clear();
    }

    pdef_->clearSolutionPaths();
    for (unsigned int k = 0; k < pdefVec_.size(); k++)
    {
        pdefVec_.at(k)->clearSolutionPaths();
    }
}

template <class T>
ob::PlannerStatus MotionExplorer<T>::solve(const ob::PlannerTerminationCondition &ptc)
{
  while (!ptc())
  {
    og::Quotient *jQuotient = quotientSpaces_.at(0);
    jQuotient->Grow();
  }

  return ob::PlannerStatus::TIMEOUT;
}

template <class T>
void MotionExplorer<T>::setProblemDefinition(std::vector<ob::ProblemDefinitionPtr> &pdef_)
{
    if (siVec_.size() != pdef_.size())
    {
        OMPL_ERROR("Number of ProblemDefinitionPtr is %d but we have %d SpaceInformationPtr.", pdef_.size(),
                   siVec_.size());
        exit(0);
    }
    pdefVec_ = pdef_;
    ob::Planner::setProblemDefinition(pdefVec_.back());
    for (unsigned int k = 0; k < pdefVec_.size(); k++)
    {
        quotientSpaces_.at(k)->setProblemDefinition(pdefVec_.at(k));
    }
}

template <class T>
void MotionExplorer<T>::setProblemDefinition(const ob::ProblemDefinitionPtr &pdef)
{
    if (siVec_.size() == 1)
    {
        this->Planner::setProblemDefinition(pdef);
    }
    else
    {
        OMPL_ERROR("You need to provide a ProblemDefinitionPtr for each SpaceInformationPtr.");
        exit(0);
    }
}

template <class T>
void MotionExplorer<T>::getPlannerData(ob::PlannerData &data) const
{
    unsigned int Nvertices = data.numVertices();
    if (Nvertices > 0)
    {
        std::cout << "cannot get planner data if plannerdata is already populated" << std::endl;
        std::cout << "PlannerData has " << Nvertices << " vertices." << std::endl;
        exit(0);
    }

    unsigned int K = quotientSpaces_.size();

    for (unsigned int k = 0; k < K; k++)
    {
        og::Quotient *Qk = quotientSpaces_.at(k);
        static_cast<QuotientGraphSparse*>(Qk)->enumerateAllPaths();
        Qk->getPlannerData(data);

        // label all new vertices
        unsigned int ctr = 0;
        for (unsigned int vidx = Nvertices; vidx < data.numVertices(); vidx++)
        {
            PlannerDataVertexAnnotated &v = *static_cast<PlannerDataVertexAnnotated *>(&data.getVertex(vidx));
            v.SetLevel(k);
            v.SetMaxLevel(K);

            ob::State *s_lift = Qk->getSpaceInformation()->cloneState(v.getState());
            v.setQuotientState(s_lift);

            for (unsigned int m = k + 1; m < quotientSpaces_.size(); m++)
            {
                og::Quotient *Qm = quotientSpaces_.at(m);

                if (Qm->GetX1() != nullptr)
                {
                    ob::State *s_X1 = Qm->GetX1()->allocState();
                    ob::State *s_Q1 = Qm->getSpaceInformation()->allocState();
                    if (Qm->GetX1()->getStateSpace()->getType() == ob::STATE_SPACE_SO3)
                    {
                        static_cast<ob::SO3StateSpace::StateType *>(s_X1)->setIdentity();
                    }
                    if (Qm->GetX1()->getStateSpace()->getType() == ob::STATE_SPACE_SO2)
                    {
                        static_cast<ob::SO2StateSpace::StateType *>(s_X1)->setIdentity();
                    }
                    Qm->MergeStates(s_lift, s_X1, s_Q1);
                    s_lift = Qm->getSpaceInformation()->cloneState(s_Q1);

                    Qm->GetX1()->freeState(s_X1);
                    Qm->GetQ1()->freeState(s_Q1);
                }
            }
            v.setState(s_lift);
            ctr++;
        }
        Nvertices = data.numVertices();
    }
    std::cout << "Created PlannerData with " << data.numVertices() << " vertices." << std::endl;
}

