#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include "Level.h"
#include <list>
#include <set>
#include <string>
#include <iostream> // defines the overloads of the << operator
#include <sstream>  // defines the type std::ostringstream
#include <iomanip>  // defines the manipulator setw
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_penelope(nullptr),
  m_numCitizensLeft(0), m_levelFinished(false)
{
}

StudentWorld::~StudentWorld(){
    cleanUp();
    cleanUp();
}

int StudentWorld::init()
{
    m_numCitizensLeft = 0;    //reinitialize value
    m_levelFinished = false;
    
    if (getLevel() == 100)   //no more levels
        return GWSTATUS_PLAYER_WON;
    
    //make string with level file name
    ostringstream levFile;
    levFile.fill('0');  // 01 or 02 instead of 1 or 2
    levFile << "level" << setw(2) << getLevel()<< ".txt";
    string levString = levFile.str();
    
    //create new level object
    Level * lev = new Level(assetPath());
    Level::LoadResult lr = lev->loadLevel(levString);
    switch (lr) {
        case Level::load_fail_file_not_found:
            return GWSTATUS_PLAYER_WON; //no more level files so player has completed & won game
            break;
        case Level::load_fail_bad_format:
            return GWSTATUS_LEVEL_ERROR;    //level not valid, game controller will handle
            break;
        default:    //level is ok, continue w init()
            break;
    }
    
    //Populate level with actors based on level file
    for (int level_x = 0; level_x < LEVEL_WIDTH; level_x++){
        for (int level_y = 0; level_y < LEVEL_HEIGHT; level_y++){
            Level::MazeEntry me = lev->getContentsOf(level_x, level_y);
            double startX = level_x * SPRITE_WIDTH;    //convert to correct coordinates
            double startY = level_y * SPRITE_HEIGHT;
            switch(me){
                case Level::player:
                    m_penelope = new Penelope(startX, startY, this);  //place P in maze
                    m_actorList.push_front(m_penelope);     //FIRST ONE IN LIST
                    break;
                case Level::wall:
                {
                    Actor * w = new Wall(startX, startY, this);
                    m_actorList.push_back(w);   //add to actor list
                    break;
                }
                case Level::exit:
                {
                    Actor * e = new Exit(startX, startY, this);
                    m_actorList.push_back(e);  
                    break;
                }
                case Level::pit:
                {
                    Actor * p = new Pit(startX, startY, this);
                    m_actorList.push_back(p);
                    break;
                }
                case Level::citizen:
                {
                    Actor * c = new Citizen(startX, startY, this);
                    m_actorList.push_back(c);
                    m_numCitizensLeft++;
                    break;
                }
                case Level::vaccine_goodie:
                {
                    Actor * vg = new VaccineGoodie(startX, startY, this);
                    m_actorList.push_back(vg);
                    break;
                }
                case Level::landmine_goodie:
                {
                    Actor * lg = new LandmineGoodie(startX, startY, this);
                    m_actorList.push_back(lg);
                    break;
                }
                case Level::gas_can_goodie:
                {
                    Actor * gcg = new GasGoodie(startX, startY, this);
                    m_actorList.push_back(gcg);
                    break;
                }
                case Level::dumb_zombie:
                {
                    Actor * dz = new DumbZombie(startX, startY, this);
                    m_actorList.push_back(dz);
                    break;
                }
                case Level::smart_zombie:
                {
                    Actor * sz = new SmartZombie(startX, startY, this);
                    m_actorList.push_back(sz);
                    break;
                }
                default:    //handle other objects later!
                    break;
            }
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    //Ask all actors to do something
    
    for (auto it = m_actorList.begin(); it != m_actorList.end(); it++){
        Actor * a = *it;
        if (! a->isAlive()) //only living actors can act in game
            continue;
        a->doSomething();
        if(!m_penelope->isAlive())  //check for P's death
            return GWSTATUS_PLAYER_DIED;
        if(m_levelFinished){        //check if P exited
            playSound(SOUND_LEVEL_FINISHED);
            return GWSTATUS_FINISHED_LEVEL;
        }
    }
    
    //remove any dead actors from list
    for (auto it = m_actorList.begin(); it != m_actorList.end(); ){
        Actor * a = *it;
        if(!(a->isAlive())){
            delete a;   //reclaim memory
            it = m_actorList.erase(it); //remove from list
        }else
            it++;   //only go to next one if actor not deleted
    }

    
    //update display text
    ostringstream gameStats;
    if (getScore()>= 0)
        gameStats.fill('0');
    gameStats << "Score: " << setw(6) << getScore() << "  ";
    gameStats << "Level: " << getLevel() << "  ";
    gameStats << "Lives: " << getLives() << "  ";
    gameStats << "Vaccines: " << m_penelope->getVaccines() << "  ";
    gameStats << "Flames: " << m_penelope->getFlames() << "  ";
    gameStats << "Mines: " << m_penelope->getMines() << "  ";       
    gameStats << "Infected: " << m_penelope->getInfectionLevel() << "  ";
    
    setGameStatText(gameStats.str());   //display to screen

    //keep going
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{

    if(m_actorList.size() > 0){    //delete any actors in the game
        for (list<Actor * >::iterator it = m_actorList.begin(); it != m_actorList.end(); it++)
            delete *it;     //delete object pointed to by Actor pointer
        m_actorList.clear();   //empty list
    }
    
     m_penelope = nullptr;   //no dangling pointer
    
}


bool StudentWorld::isValidNewPos(double destX, double destY, Actor * a) {
    for(auto it = m_actorList.begin(); it != m_actorList.end(); it++){
        Actor * a_other = *it;
        if (a_other == a)   //don't check against self
            continue;
        if(a_other->blocksMovers()){
            //calculate distance between lower left corners
            double dx = abs(destX - a_other->getX());
            double dy = abs(destY - a_other->getY());
            
            if (dx < SPRITE_WIDTH && dy < SPRITE_HEIGHT)
                return false;    //bounding boxes intersect-- can't move to this postion
        }
    }
    return true;    //no actors are in the way of the mover
}

bool StudentWorld::overLapActionTrigger(bool (Actor::*condition)() const,
                                        void (Actor::*action)(), double a_X, double a_Y){
    bool actionPerformed = false;   //keep track of whether something happens
    
    for(auto it = m_actorList.begin(); it != m_actorList.end(); it++){
        Actor * a_other = *it;
        //if (a_X == a_other->getX() && a_Y == a_other->getY()) //don't check against self
            //continue;
        if(!a_other->isAlive())
            continue;   //only check for living objects
        if((a_other->*condition)() && a_other->overLapsWith(a_X, a_Y)){  //check condition and overlap
            (a_other->*action)();   //perform action
            actionPerformed = true;
        }
    }
    return actionPerformed;
}



bool StudentWorld::pickUpGoodie(Goodie * g, Goodie::GoodieType gt){
    if (m_penelope->overLapsWith(g->getX(), g->getY())){
        g->damage();
        switch(gt){
            case Goodie::vaccine:
                m_penelope->addVaccine();
                break;
            case Goodie::gasCan:
                m_penelope->addFlames();
                break;
            case Goodie::landmine:
                m_penelope->addMines();
                break;
        }
        return true;
    }
    return false;   //penelope is not close enough to pick up goodie
}



bool StudentWorld::createFlame(double dest_x, double dest_y, Direction dir){
    
    bool flameBlocked = overLapActionTrigger(&Actor::blocksFlames,
                                             &Actor::doNothing, dest_x, dest_y);
    if (flameBlocked)
        return false;   //can't create a flame here!
    
    //nothing blocking so add flame!
    Flame * f = new Flame(dest_x, dest_y, this, dir);
    m_actorList.push_back(f);
    return true;        //successful creation
}


void StudentWorld::createLandmine(){
    Actor * l = new Landmine(m_penelope->getX(), m_penelope->getY(), this);
    m_actorList.push_back(l);
}

void StudentWorld::createPit(double x, double y){
    Actor * p = new Pit(x, y, this);
    m_actorList.push_back(p);
}


void StudentWorld::createZombie(double x, double y){
    Actor * z;
    int chance = randInt(1, 10);
    if (chance <= 7)     //70% chance dumb zombie
        z = new DumbZombie(x, y, this);
    else            //30% chance smart zombie
        z = new SmartZombie(x, y, this);
    m_actorList.push_back(z);
}

void StudentWorld::createVomit(double x, double y, Direction dir){
    Vomit * v = new Vomit(x, y, this, dir);
    m_actorList.push_back(v);
}


void StudentWorld::createVaccineGoodie(double x, double y){
    Actor * vg = new VaccineGoodie(x, y, this);
    m_actorList.push_back(vg);
    
}

bool StudentWorld::minDist(double startX, double startY, bool (Actor::*condition)() const,
                           double& resultX, double& resultY, double& dist)
{
    double minDist = VIEW_WIDTH;
    bool found = false;
    //find closest distance to mover
    for (auto it = m_actorList.begin(); it != m_actorList.end(); it++){
        Actor * a = *it;
        if (a->isAlive() && (a->*condition)()){
            double d = a->distance(startX, startY);
            if (d < minDist){
                minDist = d; //set as min if smaller
                resultX = a->getX();
                resultY = a->getY();
                found = true;
            }
        }
    }
    dist = minDist;
    return found;
}

vector<Direction> StudentWorld::findBestDirections(double startX, double startY, double goalX, double goalY){
    vector<Direction> dirs;      //set of directions c should go to follow P
    Direction d;
    if(goalY > startY){     d = Actor::up;      dirs.push_back(d);   }
    if(goalY < startY){     d = Actor::down;    dirs.push_back(d);   }
    if(goalX > startX){     d = Actor::right;   dirs.push_back(d);   }
    if(goalX < startX){     d = Actor::left;    dirs.push_back(d);   }
    
    return dirs;
}



void StudentWorld::penelopeExitIfPossible(){
    if (m_numCitizensLeft == 0)
        m_levelFinished = true;
}

