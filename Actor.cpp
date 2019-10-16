#include "Actor.h"
#include "StudentWorld.h"
#include <cmath>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

/////////ACTOR MEMBER FUNCTIONS///////

void Actor::damage(){
    setDead();
    getWorld()->playSound(deathSound());
    getWorld()->increaseScore(deathPoints());
}

bool Actor::overLapsWith(double x, double y) const {
    //calculate distance w distance formula
    double dist = distance(x, y);
    return dist <= OVERLAP_DIST;
}

double Actor::distance(double x, double y) const {
    double dx = getX() - x;
    double dy = getY() - y;
    double dist = sqrt(dx * dx + dy * dy);  //distance formula
    return dist;
}

void Actor::determineNewPos(double& dest_x, double& dest_y, int numPix_x, int numPix_y, Direction dir){
    //start at original coords
    dest_x = getX();
    dest_y = getY();
    //adjust accordining to direction
    switch(dir){
        case up:    dest_y += numPix_y;    break;
        case down:  dest_y -= numPix_y;    break;
        case left:  dest_x -= numPix_x;    break;
        case right: dest_x += numPix_x;    break;
    }
}


///////////PENELOPE MEMBER FUNCTIONS///////

void Penelope::doSomething(){
    if (!isAlive())
        return;     //dead penelope can't do anything :(
    if (isInfected()){
        incInfectionLevel();     //increases each tick
        if (getInfectionLevel() == 500){
            //P becomes zombie
            damage();  //kill her!!!
            return;
        }
        
    }
    int key;
    if (getWorld()->getKey(key)){   //valid key has been pressed since last tick
        int dest_x = getX();
        int dest_y = getY();
        switch(key){
            //face correct direction and determine where penelope should move
            case KEY_PRESS_UP:      setDirection(up);     dest_y += PENELOPE_SPEED;    break;
            case KEY_PRESS_DOWN:    setDirection(down);   dest_y -= PENELOPE_SPEED;    break;
            case KEY_PRESS_LEFT:    setDirection(left);   dest_x -= PENELOPE_SPEED;    break;
            case KEY_PRESS_RIGHT:   setDirection(right);  dest_x += PENELOPE_SPEED;    break;
            case KEY_PRESS_ENTER:
                if (getVaccines() >= 1){
                    disinfect();    //set infected status to false and count to 0
                    m_vaccines--;   //lose a vaccine
                }
                return;
            case KEY_PRESS_SPACE:
                if (m_flames >= 1){
                    m_flames--;  //lose a flamethrower
                    getWorld()->playSound(SOUND_PLAYER_FIRE);
                    //add flame objects to level
                    Direction dir = getDirection();
                    for (int i = 1; i < 4; i++){
                        double dest_x, dest_y;
                        //calculate positions for 3 new flames!
                        determineNewPos(dest_x, dest_y, i*SPRITE_WIDTH, i*SPRITE_HEIGHT, dir);
                        //attempt to put flame at calculated position
                        if (!getWorld()->createFlame(dest_x, dest_y, dir))
                            break;  //could not create new flame -- exit/wall in the way
                    }
                }
                return;
            case KEY_PRESS_TAB:
                if (m_mines >= 1){
                    getWorld()->createLandmine();
                    m_mines--;  //lose a landmine
                }
                return;
            default:
                return;     //do not try to move anywhere
        }
        if (getWorld()->isValidNewPos(dest_x, dest_y, this)) //check for anything blocking P
            moveTo(dest_x, dest_y);
    }
}

void Penelope::damage(){
    Actor::damage();
    getWorld()->decLives();
}

void Penelope::exit(){
    getWorld()->penelopeExitIfPossible();
}


///////////CITIZEN MEMBER FUNCTIONS///////

void Citizen::doSomething(){
    if (!isAlive())
        return;
    if (isInfected()){
        incInfectionLevel();     //increases each tick
        if (getInfectionLevel() == 500){
            //citizen becomes zombie
            zombify();
            return;
        }
    }
    setActive(!isActive());   //switches every tick
    if (!isActive())
        return; //paralysed

    
    //MOVEMENT TIME
    
    //calculate distance from citizen to penelope and to nearest zombie
    double pen_x, pen_y, dist_p;
    getWorld()->minDist(getX(), getY(), &Actor::savesCitizens, pen_x, pen_y, dist_p);
    
    double zombie_x, zombie_y, dist_z;
    bool zExists = getWorld()->minDist(getX(), getY(), &Actor::scaresCitizen,
                                       zombie_x, zombie_y, dist_z);
    
    //if closer to P and no zombies exist AND distance is less than 80 px
    if ((dist_p < dist_z || !zExists) && dist_p <= 80){
        if (moveTowardsPenelope(pen_x, pen_y) )
            return;     //do nothing more after moving
    }
    
    //run away from zombies!
    if (zExists && dist_z <= 80){
        moveAwayFromZombies(dist_z);
    }
}

void Citizen::zombify(){
    setDead();
    getWorld()->playSound(SOUND_ZOMBIE_BORN);
    getWorld()->increaseScore(-1000);
    getWorld()->removeCitizen();
    getWorld()->createZombie(getX(), getY()); //add zombie in c's coords
}

bool Citizen::moveTowardsPenelope(double pen_x, double pen_y){
    //get directions towards penelope
    vector<Direction> dirs = getWorld()->findBestDirections(getX(), getY(), pen_x, pen_y);
    
    //on same row or col- only one useful direction
    if (dirs.size() == 1 && tryToMove(dirs[0]))
        return true; //successfully moved
    
    //not on same row/col -- try random direction
    int x = randInt(0, 1);
    
    if(tryToMove(dirs[x]))
        return true;     //successfully moved
    
    //try other possible direction
    if(tryToMove(dirs[x-1]))
        return true;     //successfully moved
    
    return false;   //did not move
}

bool Citizen::moveAwayFromZombies(double dist_z){
    //check all directions to find one that takes us farthest
    Direction dirArr [] = {up, down, left, right};
    double max_zDist = dist_z;  //current best distance
    Direction bestDir = getDirection();
    double test_x, test_y;
    for (int i = 0; i < 4; i++){
        determineNewPos(test_x, test_y, CITIZEN_SPEED, CITIZEN_SPEED, dirArr[i]);
        
        if (!getWorld()->isValidNewPos(test_x, test_y, this))
            continue; //ignore directions where they can't move
        
        //find distance from closest zombie
        double test_dist, zx, zy;
        getWorld()->minDist(test_x, test_y, &Actor::scaresCitizen, zx, zy, test_dist);
        
        //this direction would take them further from zombies
        if (test_dist > max_zDist){
            max_zDist = test_dist;
            bestDir = dirArr[i];
        }
    }
    
    if (max_zDist == dist_z)
        return false;   //did not move
    
    //otherwise move to best position
    tryToMove(bestDir);
    return true;    //successfully moved
}

bool Citizen::tryToMove(Direction dir){
    double dest_x, dest_y;
    determineNewPos(dest_x, dest_y, CITIZEN_SPEED, CITIZEN_SPEED, dir);
    if(getWorld()->isValidNewPos(dest_x, dest_y, this)){
        setDirection(dir);
        moveTo(dest_x, dest_y);
        return true;     //successfully moved
    }
    return false;   //could not move
}

void Citizen::damage(){
    Actor::damage();
    getWorld()->removeCitizen();
}

void Citizen::exit(){
    getWorld()->increaseScore(500);     //receive points for saving citizen!
    setDead();                          //remove from game
    getWorld()->playSound(SOUND_CITIZEN_SAVED);
    getWorld()->removeCitizen();
}

void Citizen::infect(){
    if (!isInfected())  //only play sound first time citizen is infected
        getWorld()->playSound(SOUND_CITIZEN_INFECTED);
    Person::infect();
}

///////////ZOMBIE MEMBER FUNCTIONS///////
void Zombie::doSomething(){
    if (!isAlive())
        return;
    setActive(!isActive());   //switches every tick
    if (!isActive())
        return; //paralysed
    
    if (attemptToVomit())
        return;     //do nothing else after vomiting
    
    if (getPlanDist() < 1){  //needs new plan
        determineNewMovementPlan();
    }
    //move to new position
    double dest_x, dest_y;
    determineNewPos(dest_x, dest_y, ZOMBIE_SPEED, ZOMBIE_SPEED, getDirection());
    
    if (getWorld()->isValidNewPos(dest_x, dest_y, this)){
        moveTo(dest_x, dest_y);
        setPlanDist(getPlanDist() - 1); //decrement plan distance
    }else
        setPlanDist(0);  //cannot move in this direction!! rechoose
}


bool Zombie::attemptToVomit(){
    double vx, vy;
    determineNewPos(vx, vy, SPRITE_WIDTH, SPRITE_HEIGHT, getDirection());
    //check if vomit would land on anyone
    bool couldVomit = getWorld()->overLapActionTrigger(&Actor::canBeInfected,
                                                       &Actor::doNothing, vx, vy);
    if (couldVomit){
        int x = randInt(1, 3);      //1 in 3 chance it will decide to vomit
        if (x == 1){
            getWorld()->createVomit(vx, vy, getDirection());
            getWorld()->playSound(SOUND_ZOMBIE_VOMIT);
            return true; //decided to vomit
        }
    }
    return false;   //did not vomit
}


///////////DUMBZOMBIE MEMBER FUNCTIONS///////


void DumbZombie::determineNewMovementPlan(){
    setPlanDist(randInt(3, 10));
    Direction d = randInt(0, 3) * 90;
    setDirection(d);
}

void DumbZombie::damage(){
    Actor::damage();
    //1 in 10 chance zombie will drop vaccine goodie when it dies
    int chance = randInt(1, 10);
    //cerr << x;
    if (chance == 1){
        //getWorld()->createVaccineGoodie(this);
        Direction newDir = randInt(0, 3) * 90;
        double dest_x, dest_y;
        determineNewPos(dest_x, dest_y, SPRITE_WIDTH, SPRITE_HEIGHT, newDir);
        
        //check if vaccine can go here-- can't overlap with any object in game
        bool overlap = getWorld()->overLapActionTrigger(&Actor::isAlive,
                                                        &Actor::doNothing, dest_x, dest_y);
        if (!overlap){
            getWorld()->createVaccineGoodie(dest_x, dest_y);
        }
    }
}

///////////SMART ZOMBIE MEMBER FUNCTIONS///////

void SmartZombie::determineNewMovementPlan(){
    setPlanDist(randInt(3, 10));
    Direction newDir = -1;
    //find closest person to zombie
    double person_x, person_y, person_dist;
    getWorld()->minDist(getX(), getY(), &Actor::canBeInfected, person_x, person_y, person_dist);
    
    if (person_dist > 80){  //not close enough to follow
        newDir = randInt(0, 3) * 90;    //up/down/left/right
    }else{
        
        //get directions that take us closer to closest person
        vector<Direction> dirs = getWorld()->findBestDirections(getX(), getY(), person_x, person_y);
        if (dirs.size() == 1)   //on same row or col- only one useful direction
            newDir = dirs[0];
        else{
            int x = randInt(0, 1);      //choose randomly from possible directions
            if (x < dirs.size())     //just in case!! should always be true
                newDir = dirs[x];
        }
    }
    setDirection(newDir);
}


///////////EXIT MEMBER FUNCTIONS/////////


void Exit::doSomething(){
    //check for citizens and penelope to exit level
    getWorld()->overLapActionTrigger(&Actor::canExit, &Actor::exit, getX(), getY());
}


///////////PIT MEMBER FUNCTIONS/////////


void Pit::doSomething(){
    getWorld()->overLapActionTrigger(&Actor::canFall, &Actor::damage, getX(), getY());
    //getWorld()->suckIntoPit(this);
}

    
///////////PROJECTILE MEMBER FUNCTIONS/////////

bool Projectile::hasTicksLeft(){
    ticksLeft--;   //decrement ticks left
    return ticksLeft > 0;
}

///////////FLAME MEMBER FUNCTIONS/////////

void Flame::doSomething(){
    if (!hasTicksLeft()){
        setDead();  //remove from game
    }
    if (!isAlive())
        return;
    getWorld()->overLapActionTrigger(&Actor::canBeDamaged, &Actor::damage, getX(), getY());
}
    
///////////VOMIT MEMBER FUNCTIONS/////////

void Vomit::doSomething(){
    if (!hasTicksLeft()){
        setDead();  //remove from game
    }
    if (!isAlive())
        return;
    
    getWorld()->overLapActionTrigger(&Actor::canBeInfected, &Actor::infect, getX(), getY());
}

/////////// GOODIE MEMBER FUNCTIONS/////////

void Goodie::doSomething(){
    if (!isAlive())
        return;
    //check if penelope is close enough to pick up
    getWorld()->pickUpGoodie(this, getType());
}


///////////LANDMINE MEMBER FUNCTIONS/////////

void Landmine::doSomething(){
    if (!isAlive())
        return;
    if (!isActive()){
        if (!hasTicksLeft())    //decrements ticks
            setActive(true);    //can now be detonated
        return;
    }
    bool landmineSteppedOn = getWorld()->overLapActionTrigger(&Actor::triggersLandmine,
                                                              &Actor::doNothing, getX(), getY());
    if (landmineSteppedOn){
        //getWorld()->detonateLandmine(this);
        damage();
        
        double sh = SPRITE_HEIGHT;
        double sw = SPRITE_WIDTH;
        
        //arrays to represent changes in pos from landmine to create new flames
        double dx[] = {sh, sh, sh, 0, 0, 0, -sh, -sh, -sh};
        double dy[] = {-sw, 0, sw, -sw, 0, sw, -sw, 0, sw};
        
        //create 9 flames surrounding landmine
        for (int i = 0; i < 9; i++)
            getWorld()->createFlame(getX() + dx[i], getY() + dy[i], up);
        
        //create pit at landmine's location
        getWorld()->createPit(getX(), getY());
    }
    
    
}



