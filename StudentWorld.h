#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <string>
#include <list>

using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    virtual ~StudentWorld();
    
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    bool isValidNewPos(double destX, double destY, Actor * a);
    bool overLapActionTrigger(bool (Actor::*condition)() const,
                              void (Actor::*action)(), double a_X, double a_Y);
    
    bool pickUpGoodie(Goodie * g, Goodie::GoodieType gt);
    
    void createLandmine();
    void createPit(double x, double y);
    void createZombie(double x, double y);
    bool createFlame(double x, double y, Direction dir);
    void createVomit(double x, double y, Direction dir);
    void createVaccineGoodie(double x, double y);
    
    bool minDist(double startX, double startY, bool (Actor::*condition)() const,
                 double& resultX, double& resultY, double& distance);
    vector<Direction> findBestDirections(double startX, double startY, double goalX, double goalY);
    void removeCitizen(){   //one less citizen to save
        m_numCitizensLeft--;
    }
    void penelopeExitIfPossible();

private:
    list<Actor * > m_actorList;
    Penelope * m_penelope;
    int m_numCitizensLeft;
    bool m_levelFinished;
    
    
};

#endif // STUDENTWORLD_H_
