#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp


class GameWorld;
class StudentWorld;

const int OVERLAP_DIST = 10;    //if centers are 10 pixels apart, objects overlap
const int PENELOPE_SPEED = 4;
const int CITIZEN_SPEED = 2;
const int ZOMBIE_SPEED = 1;

class Actor: public GraphObject
{
public:
    Actor (int imageID, double startX, double startY, StudentWorld * sw, Direction dir = 0,
           int depth = 0, double size = 1.0)
    :   GraphObject(imageID, startX, startY, dir, depth, size),
    m_alive(true), m_sw(sw), m_active(true)
    { 
    }
    virtual ~Actor() { }
    
    virtual void doSomething() = 0;     //actions during each tick
    void doNothing(){   }   
    virtual void damage();
    virtual void infect(){    }
    virtual void exit(){    }
    virtual bool blocksMovers() const{
        return false;   //default implementation
    }
    virtual bool canExit() const {
        return false;
    }
    virtual bool canBeDamaged() const {
        return false;
    }
    virtual bool canFall() const {
        return false;
    }
    virtual bool canBeInfected() const {
        return false;
    }
    virtual bool blocksFlames() const {
        return false;
    }
    virtual bool triggersLandmine() const {
        return false;
    }
    virtual bool scaresCitizen() const {
        return false;
    }
    virtual bool savesCitizens() const {
        return false;
    }
    virtual int deathSound() const {
        return SOUND_NONE;
    }
    virtual int deathPoints() const {
        return 0;
    }
    bool isAlive() const{
        return m_alive;
    }
    bool isActive() const{
        return m_active;
    }
    void setActive(bool a){
        m_active = a;
    }
    void setDead(){
        m_alive = false;
    }
    StudentWorld * getWorld() const{
        return m_sw;
    }
    bool overLapsWith(double x, double y) const;
    double distance(double x, double y) const;
    void determineNewPos(double& dest_x, double& dest_y, int numPix_x, int numPix_y, Direction dir);
private:
    bool m_alive;
    bool m_active;
    StudentWorld * m_sw;
};

class Person: public Actor
{
public:
    Person (int imageID, double startX, double startY, StudentWorld * sw, Direction dir = 0,
           int depth = 0, double size = 1.0)
    :   Actor(imageID, startX, startY, sw, dir, depth, size),
        m_infectionLevel(0), m_infected(false)
    {
    }
    virtual ~Person() { }
    
    virtual bool blocksMovers() const {
        return true;
    }
    virtual bool canFall() const {
        return true;    //only penelope, citizens, zombie can fall in pit
    }
    virtual bool canExit() const {
        return true;
    }
    virtual bool canBeDamaged() const {
        return true;
    }
    virtual bool canBeInfected() const {
        return true;
    }
    virtual bool triggersLandmine() const {
        return true;
    }
    bool isInfected() const {
        return m_infected;
    }
    int getInfectionLevel() const {
        return m_infectionLevel;
    }
    void incInfectionLevel(){
        m_infectionLevel++;
    }
    virtual void infect(){
        m_infected = true;
    }
    void disinfect(){
        m_infected = false;
        m_infectionLevel = 0;
    }
  
private:
    bool m_infected;
    int m_infectionLevel;
};

class Penelope: public Person
{
public:
    Penelope(double startX, double startY, StudentWorld * sw)
    :   Person(IID_PLAYER, startX, startY,  sw),
     m_flames(0), m_mines(0), m_vaccines(0)
    {
    }
    virtual ~Penelope() { }
    
    virtual int deathSound() const {
        return SOUND_PLAYER_DIE;
    }
    virtual void doSomething();
    virtual void damage();
    virtual void exit();
    
    virtual bool savesCitizens() const {
        return true;
    }
    int getFlames() const {
        return m_flames;
    }
    void addFlames() {
        m_flames += 5;
    }
    int getMines() const {
        return m_mines;
    }
    void addMines() {
        m_mines += 2;
    }
    int getVaccines() const {
        return m_vaccines;
    }
    void addVaccine() {
        m_vaccines++;
    }
    
private:
    int m_flames;
    int m_mines;
    int m_vaccines;
};


class Citizen: public Person
{
public:
    Citizen(double startX, double startY, StudentWorld * sw)
    :   Person(IID_CITIZEN, startX, startY,  sw) //, active(false)
    {
        setActive(false);
    }
    
    virtual void doSomething();
    
    virtual void damage();
    virtual void exit();
    virtual void infect();
    
    virtual int deathSound() const {
        return SOUND_CITIZEN_DIE;
    }
    virtual int deathPoints() const {
        return -1000;
    }
    
private:
    bool moveTowardsPenelope(double pen_x, double pen_y);
    bool moveAwayFromZombies(double dist_z);
    bool tryToMove(Direction dir);
    void zombify();
};


class Zombie: public Actor
{
public:
    Zombie(double startX, double startY, StudentWorld * sw)
    : Actor(IID_ZOMBIE, startX, startY, sw), m_planDist(0)
    {
        setActive(false);
    }
    
    virtual void doSomething();
    
    
    virtual void determineNewMovementPlan() = 0;
    
    virtual int deathSound() const {
        return SOUND_ZOMBIE_DIE;
    }
    int getPlanDist() const {
        return m_planDist;
    }
    void setPlanDist(int d) {
        m_planDist = d;
    }
    virtual bool blocksMovers() const {
        return true;
    }
    virtual bool canFall() const {
        return true;    //only penelope, citizens, zombie can fall in pit
    }
    virtual bool canBeDamaged() const {
        return true;
    }
    virtual bool scaresCitizen() const {
        return true;
    }
    virtual bool triggersLandmine() const {
        return true;
    }
private:
    int m_planDist;
    
    bool attemptToVomit();
};



class DumbZombie: public Zombie
{
public:
   DumbZombie(double startX, double startY, StudentWorld * sw)
    : Zombie(startX, startY, sw)
    {
        
    }
    
    virtual int deathPoints() const {
        return 1000;
    }
    virtual void determineNewMovementPlan();
    virtual void damage();
    
private:
};


class SmartZombie: public Zombie {
public:
    SmartZombie(double startX, double startY, StudentWorld * sw)
    :   Zombie(startX, startY, sw)
    {
        
    }
    virtual int deathPoints() const {
        return 2000;
    }
    virtual void determineNewMovementPlan();
    
private:
};



class Wall: public Actor
{
public:
    Wall(double startX, double startY, StudentWorld * sw)
    :   Actor(IID_WALL, startX, startY, sw)
    {
    }
    virtual ~Wall() { }
    
    virtual void doSomething(){
        //walls do nothing :|
    }
    virtual bool blocksMovers() const{
        return true;
    }
    virtual bool blocksFlames() const {
        return true;
    }
    
private:
    
};

class Exit: public Actor
{
public:
    Exit(double startX, double startY, StudentWorld * sw)
    :   Actor(IID_EXIT, startX, startY, sw, right, 1)
    {
    }
    virtual ~Exit() { }
    
    virtual void doSomething();
    
    virtual bool blocksFlames() const {
        return true;
    }
    
private:
    
};

class Pit: public Actor
{
public:
    Pit(double startX, double startY, StudentWorld * sw)
    :   Actor(IID_PIT, startX, startY, sw)
    {
    }
    virtual ~Pit() { }
    
    virtual void doSomething();
    
private:
};

class Projectile: public Actor
{
public:
    Projectile(int imageID, double startX, double startY, StudentWorld * sw,
               Direction dir, int depth, int ticks)
    :   Actor(imageID, startX, startY, sw, dir, depth), ticksLeft(ticks)
    {  
    }
    virtual ~Projectile() { }
    bool hasTicksLeft();
    
private:
    int ticksLeft;
};

class Flame: public Projectile
{
public:
    Flame(double startX, double startY, StudentWorld * sw, Direction dir)
    :   Projectile(IID_FLAME, startX, startY, sw, dir, 0, 3)    //0 depth, 3 ticks
    { 
    }
    virtual ~Flame() { }
    
    virtual void doSomething();
    virtual bool triggersLandmine() const {
        return true;
    }
    
private:
};


class Landmine: public Projectile
{
public:
    Landmine(double startX, double startY, StudentWorld * sw)
    :   Projectile(IID_LANDMINE, startX, startY, sw, right, 1, 30)     //depth 1, 30 ticks
    {
        setActive(false);
    }
    virtual ~Landmine() { }
    
    virtual void doSomething();
    
    virtual int deathSound() const {
        return SOUND_LANDMINE_EXPLODE;
    }
    
private:
};



class Vomit: public Projectile
{
public:
    Vomit(double startX, double startY, StudentWorld * sw, Direction dir)
    :   Projectile(IID_VOMIT, startX, startY, sw, dir, 0, 3)   //0 depth, 3 ticks
    {
    }
    virtual ~Vomit() { }
    
    virtual void doSomething();
    
private:
};



class Goodie: public Actor
{
public:
    
    enum GoodieType {
        vaccine, gasCan, landmine
    };
    
    Goodie(int imageID, double startX, double startY, StudentWorld * sw)
    :   Actor(imageID, startX, startY, sw, 0, 1)
    {
    }
    virtual ~Goodie() { }
    
    virtual void doSomething();
    virtual bool canBeDamaged() const {
        return true;
    }
    virtual GoodieType getType() = 0;
    virtual int deathSound() const {
        return SOUND_GOT_GOODIE;
    }
    virtual int deathPoints() const {
        return 50;
    }
    
private:
};


class VaccineGoodie: public Goodie
{
public:
    VaccineGoodie(double startX, double startY, StudentWorld * sw)
    :   Goodie(IID_VACCINE_GOODIE, startX, startY, sw)
    {
    }
    virtual ~VaccineGoodie() { }

    virtual GoodieType getType(){
        return vaccine;
    }
    
private:
};


class GasGoodie: public Goodie
{
public:
    GasGoodie(double startX, double startY, StudentWorld * sw)
    :   Goodie(IID_GAS_CAN_GOODIE, startX, startY, sw)
    {
    }
    virtual ~GasGoodie() { }
    
    virtual GoodieType getType(){
        return gasCan;
    }
    
private:
};


class LandmineGoodie: public Goodie
{
public:
    LandmineGoodie(double startX, double startY, StudentWorld * sw)
    :   Goodie(IID_LANDMINE_GOODIE, startX, startY, sw)
    {
    }
    virtual ~LandmineGoodie() { }
    
    virtual GoodieType getType(){
        return landmine;
    }
    
private:
};




#endif // ACTOR_H_
