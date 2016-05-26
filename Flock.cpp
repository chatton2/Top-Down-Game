#include "Flock.h"

Flock::Flock(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale, Grid* g, Ogre::Camera* c)
{
	cam = c;
	grid = g;
	using namespace Ogre;
	routeSoFar = 0.0;
	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node
	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	setupAnimations();  // load the animation for this character

	// configure walking parameters
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;
	mKeyDirection = Ogre::Vector3::ZERO;
	mGoalDirection = Ogre::Vector3::ZERO;
	mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());
	//Randomly rotates flock
	double randAngle = rand() % (360);
	randAngle  = randAngle * (22/7) / 180; 
	Ogre::Quaternion rand(randAngle, 0,1,0);
	this -> mBodyNode -> rotate(rand);
	this->mDirection = this->mBodyNode->getPosition();
	//Start the flocking
	keepFlock = true;
}

Flock::~Flock(){
}

void 
Flock::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

// update is called at every frame from GameApplication::addTime
void
Flock::update(Ogre::Real deltaTime) 
{
	//Calculate flocking
	if(keepFlock == true){
		this->flockMove(this -> separate(50, this->flockList), this -> alignment(50, this->flockList), this -> cohesion(50, this->flockList), deltaTime);
	}

	this->updateAnimations(deltaTime);	// Update animation playback
}


void 
Flock::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 13; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	setBaseAnimation(ANIM_RUN_BASE);
	setTopAnimation(ANIM_RUN_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void 
Flock::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id; 

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

	
void Flock::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void 
Flock::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	
	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void 
Flock::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 13; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

//Calculate the Agent's grid coordinates using it's position
int 
Flock::getRCoord(){
	int rC = (this->mBodyNode->getPosition()[2] + (this->grid->getRows() * NODESIZE)/2.0 + (NODESIZE/2.0)) / NODESIZE;
	return rC;
}

int 
Flock::getCCoord(){
	int cC = (this->mBodyNode->getPosition()[0] + (this->grid->getColumns() * NODESIZE)/2.0 + (NODESIZE/2.0)) / NODESIZE;
	return cC;
}


void Flock::doTheThing(){
	if (mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_RUN_BASE)
		{
			setBaseAnimation(ANIM_IDLE_BASE);
			if (mTopAnimID == ANIM_RUN_TOP) setTopAnimation(ANIM_IDLE_TOP);
		}
	else if (!mKeyDirection.isZeroLength() && mBaseAnimID == ANIM_IDLE_BASE)
		{
			// start running if not already moving and the player wants to move
			setBaseAnimation(ANIM_RUN_BASE, true);
			if (mTopAnimID == ANIM_IDLE_TOP) setTopAnimation(ANIM_RUN_TOP, true);
		}

}

//Sends out ray to see if Ogre will run into wall
bool Flock::isCollision(const Ogre::Vector3& position, const Ogre::Vector3& direction) 
{
	mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());//creates a ray
    Ogre::Ray ray(position, direction);//stes the rays position and direction
    mRaySceneQuery->setRay(ray);//sets the ray to the query
    Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute(); //Get the results from the SceneQuery
    Ogre::RaySceneQueryResult::iterator itr;
    for (itr = result.begin(); itr != result.end(); itr++) {
        if (itr->movable->getName().compare(0,4,"Carl")==0 && itr->distance<NODESIZE){
            return true;
        }
    }
    return false;
}


//Hides the flock and keeps them from flocking
void
Flock::stopTheWalk(){
	std::list<Flock*>::iterator it = this->flockList.begin();
	for(it; it != flockList.end(); it++){
		(*it)->mBodyNode->setVisible(false);
		(*it)->mBodyNode->translate(0,100,0);
		(*it)->keepFlock = false;
	}
}

//Start the Agents' walking animation
void
Flock::startTheWalk(){
	std::list<Flock*>::iterator it = this->flockList.begin();
	for(it; it != flockList.end(); it++){
		this->setBaseAnimation(ANIM_RUN_BASE,true);//Set running animations
		this->setTopAnimation(ANIM_RUN_TOP,true);
	}
}

void
Flock::flockMove(Ogre::Vector3 sep, Ogre::Vector3 als, Ogre::Vector3 coh, Ogre::Real deltaTime){
	//Get the vector to the goal
	Ogre::Vector3 hustle = (sep) + (.036 * als) + (.036 * coh) + (.036 * getGoal());
	//Calculate destination
	Ogre::Vector3 flo  = hustle + mBodyNode->getPosition(); 
	flo[1] = mBodyNode->getPosition()[1];
	hustle[1] = mBodyNode->getPosition()[1];
	//Roatate to next goal
    mDirection = flo -  mBodyNode->getPosition();
    mDistance = mDirection.normalise();
	Ogre::Vector3 too = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;//Turn the Agent facing the destination
	if ((1.0f + too.dotProduct(mDirection)) < 0.0001f) 
	{
		mBodyNode->yaw(Ogre::Degree(180));
	}
	else
	{
		Ogre::Quaternion quat = too.getRotationTo(mDirection);
		mBodyNode->rotate(quat);
	} 
	Ogre::Vector3 checkDir = flo - mBodyNode->getPosition();
	checkDir[1] = 0;
	Ogre::Vector3 checkVec = this->mBodyNode->getPosition();
	checkVec[1] = 0;
	//Checks for wall collisions
	if(isCollision(checkVec, checkDir) == false){
		mBodyNode->translate(Ogre::Vector3(hustle[0], 0, hustle[2]));//Move Agent
	}
	//If player hits Ogre player loses
	if(this->mBodyEntity->getWorldBoundingBox().intersects(player->mBodyEntity->getWorldBoundingBox())){
		if((this->mBodyNode->getPosition() - this->mBodyNode->getPosition()).length() < .0001){
			player->gameLose = true;
			player->mBodyNode->setVisible(false);
			player->boomNode->setVisible(false);
			stopTheWalk();
		}
	}
}

Ogre::Vector3 
Flock::separate(double radius, std::list<Flock*> agents){
	Ogre::Vector3 seps(0,0,0);
	std::list<Flock*>::iterator it = agents.begin();
	int radNum = 0;
	it = agents.begin();
	for(it; it != agents.end(); it++) {
		if(this != (*it)){//If current Agent not itself
			if(abs((*it)->getRCoord() - this->getRCoord()) <= 4 &&  abs((*it)->getCCoord() - this->getCCoord()) <= 4){
				seps = seps + (2 * (this->mBodyNode->getPosition() - (*it)->mBodyNode->getPosition()) / std::pow(this->mBodyNode->getPosition().distance((*it)->mBodyNode->getPosition()), 2));		
				//Gets the sum of the normalized distances
			}
		}
	}
	return (6 * seps);
}

Ogre::Vector3 
Flock::alignment(double radius, std::list<Flock*> agents){
	Ogre::Vector3 als(0,0,0);
	double weights = 1.0;
	std::list<Flock*>::iterator it = agents.begin();
	int radNum = 0;
	it = agents.begin();
	for(it; it != agents.end(); it++) {
		if(this != (*it)){
			if(abs((*it)->getRCoord() - this->getRCoord()) <= 4 &&  abs((*it)->getCCoord() - this->getCCoord()) <= 4){
				Ogre::Vector3 jj = (*it) -> mDirection;
				als = als + (jj * 2);
				radNum = radNum + 2;
			}
		}
	}
	if(radNum != 0){
		return (2 * als / radNum);
	}
	return als;
}

Ogre::Vector3 
Flock::cohesion(double radius, std::list<Flock*> agents){
	Ogre::Vector3 cm(0,0,0);
	double weights = 1.0;
	std::list<Flock*>::iterator it = agents.begin();
	int radNum = 0;
	it = agents.begin();
	for(it; it != agents.end(); it++) {
		if(this != (*it)){
			if(abs((*it)->getRCoord() - this->getRCoord()) <= 4 &&  abs((*it)->getCCoord() - this->getCCoord())  <= 4){
				Ogre::Vector3 jj = (*it)->mBodyNode->getPosition();
				cm = cm + (jj);
				radNum = radNum + 2;
			}
		}
	}
	if(radNum != 0){
		return (2 * ((cm / radNum) - this->mBodyNode->getPosition()));
	}
	return (cm);
}


//If boomerang's been thrown Ogre walks toward player.  Else Ogre walks away from player
Ogre::Vector3 
Flock::getGoal(){
	if(player->isThrown == true){
		return (player->mBodyNode->getPosition() - this -> mBodyNode ->getPosition());
	}
	else{
		return -(player->mBodyNode->getPosition() - this -> mBodyNode ->getPosition());
	}
}

