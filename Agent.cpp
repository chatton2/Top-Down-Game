#include "Agent.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale, Grid* g, Ogre::Camera* c)
{
	cam = c;
	grid = g; //Pointer to grid
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
	//Create the boomerang object
	boomNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Boomerang",mBodyNode->getPosition());
	boomEnt = mSceneMgr->createEntity("Boomerang", "Fish.mesh"); 
	boomNode->attachObject(boomEnt);
	boomNode->setVisible(false); //Make it invisible
	boomEnt->setMaterialName("textures/tusk.jpg");//Make the boomerang white
	isThrown = false; //Set it to not be thrown
	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	setupAnimations();  // load the animation for this character

	// configure walking parameters
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;
	mKeyDirection = Ogre::Vector3::ZERO;
	mGoalDirection = Ogre::Vector3::ZERO;
	mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());
	left = 4;
	//Make the Orangutan White then make it orange
	mBodyEntity->setMaterialName("textures/orang.png");
	Ogre::MaterialPtr m_pMat = mBodyEntity->getSubEntity(0)->getMaterial();
	m_pMat->getTechnique(0)->getPass(0)->setAmbient(1, .5, 0);
	m_pMat->getTechnique(0)->getPass(0)->setDiffuse(1, 0,0,0);
	gameWin = false;
	gameLose = false;
}



Agent::~Agent(){
}

void 
Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

// update is called at every frame from GameApplication::addTime
void
Agent::update(Ogre::Real deltaTime) 
{
	this->updateAnimations(deltaTime);	// Update animation playback
	this->updateBody(deltaTime);
	//If the boomerang has been thrown, move it
	if(isThrown == true){
		this->updateBoomerang(deltaTime);
	}
	else{
		 boomNode->setPosition(mBodyNode->getPosition());
	}
}


void 
Agent::setupAnimations()
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

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void 
Agent::setBaseAnimation(AnimID id, bool reset)
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

	
void Agent::setTopAnimation(AnimID id, bool reset)
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
Agent::updateAnimations(Ogre::Real deltaTime)
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
Agent::fadeAnimations(Ogre::Real deltaTime)
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
Agent::getRCoord(){
	int rC = (this->mBodyNode->getPosition()[2] + (this->grid->getRows() * NODESIZE)/2.0 + (NODESIZE/2.0)) / NODESIZE;
	return rC;
}

int 
Agent::getCCoord(){
	int cC = (this->mBodyNode->getPosition()[0] + (this->grid->getColumns() * NODESIZE)/2.0 + (NODESIZE/2.0)) / NODESIZE;
	return cC;
}

//Move the Orangutan based on the keyboard movements
void 
Agent::updateBody(Ogre::Real deltaTime)
	{
		mGoalDirection = Ogre::Vector3::ZERO;   // we will calculate this

		if (mKeyDirection != Ogre::Vector3::ZERO && mBaseAnimID != ANIM_DANCE)
		{
			// calculate actually goal direction in world based on player's key directions
			mGoalDirection += mKeyDirection.z * cam->getOrientation().zAxis();
			mGoalDirection += mKeyDirection.x * cam->getOrientation().xAxis();
			mGoalDirection.y = 0;
			mGoalDirection.normalise();

			 Ogre::Quaternion toGoal = mBodyNode->getOrientation().zAxis().getRotationTo(mGoalDirection);

			// calculate how much the character has to turn to face goal direction
			 Ogre::Real yawToGoal = toGoal.getYaw().valueDegrees();
			// this is how much the character CAN turn this frame
			 Ogre::Real yawAtSpeed = yawToGoal / abs(yawToGoal) * deltaTime * 500.0f ;
			// reduce "turnability" if we're in midair
			if (mBaseAnimID == ANIM_JUMP_LOOP) yawAtSpeed *= 0.2f;

			// turn as much as we can, but not more than we need to
			if (yawToGoal < 0) yawToGoal =  std::min< Ogre::Real>(0, std::max< Ogre::Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, yawAtSpeed, 0);
			else if (yawToGoal > 0) yawToGoal = std::max< Ogre::Real>(0, std::min< Ogre::Real>(yawToGoal, yawAtSpeed)); //yawToGoal = Math::Clamp<Real>(yawToGoal, 0, yawAtSpeed);
			
			mBodyNode->yaw( Ogre::Degree(yawToGoal));

			// Check if the Orangutan is going to move into a wall
			Ogre::Vector3 checkVec = this->mBodyNode->getPosition();
			checkVec[1] = 0;
			Ogre::Vector3 checkDir = mGoalDirection;
			checkDir[1] = 0;
			if(isCollision(checkVec, checkDir, 1) == false){ 
				mBodyNode->translate(0, 0, deltaTime * 35 * mAnims[mBaseAnimID]->getWeight(),Ogre::Node::TS_LOCAL);
			}
		}

		if (mBaseAnimID == ANIM_JUMP_LOOP)
		{
			// if we're jumping, add a vertical offset too, and apply gravity
			mBodyNode->translate(0, mVerticalVelocity * deltaTime, 0,  Ogre::Node::TS_LOCAL);
			mVerticalVelocity -= -9.81 * deltaTime;
			
			 Ogre::Vector3 pos = mBodyNode->getPosition();
			if (pos.y <= height)
			{
				// if we've hit the ground, change to landing state
				pos.y = height;
				mBodyNode->setPosition(pos);
				setBaseAnimation(ANIM_JUMP_END, true);
				mTimer = 0;
			}
		}
	}

//Change the animation from walking to idle and vice versa
void Agent::doTheThing(){
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

//Sends a ray out to the direction to see if their will be a collision
bool Agent::isCollision(const Ogre::Vector3& position, const Ogre::Vector3& direction, int by) 
{
	mRaySceneQuery = mSceneMgr->createRayQuery(Ogre::Ray());//creates a ray
    Ogre::Ray ray(position, direction);//stes the rays position and direction
    mRaySceneQuery->setRay(ray);//sets the ray to the query
    Ogre::RaySceneQueryResult &result = mRaySceneQuery->execute(); //Get the results from the SceneQuery
    Ogre::RaySceneQueryResult::iterator itr;
    for (itr = result.begin(); itr != result.end(); itr++) {
        if (itr->movable->getName().compare(0,4,"Carl")==0 && itr->distance<(NODESIZE / by)){
            return true;
        }
    }
    return false;
}

//Move boomerang
void Agent::updateBoomerang(Ogre::Real deltaTime){
	if(gameLose == false){//Won't fire if game was lost
		Ogre::Vector3 pos = boomNode->getPosition();
		timeKeep = timeKeep + deltaTime; //keeps track of how long boomerang's been out
		std::list<Flock*>::iterator it;
		bool hitFound = false;//If the boomerang hit an ogre
		for(it = flockList.begin(); it!= flockList.end(); it++){//If boomerang intersected an Ogre
			if(boomEnt->getWorldBoundingBox().intersects((*it)->mBodyEntity->getWorldBoundingBox())){
				(*it)->keepFlock = false;//Stop it's moving
				(*it)->mBodyNode->setVisible(false);//Make it invisible
				(*it)->mBodyNode->translate(0,100,0);//Move it upwards
				isThrown = false;//Make boomerang come back
				boomNode->setVisible(false);
				hitFound = true;
				left = left - 1;
				if(left == 0){//If there are no Ogre's left
					gameWin = true;
				}
				break;
			}
		}
		//If no collisions
		if(throwBack == false && hitFound == false){
			throwStart = ((throwStart) + (deltaTime));
			pos = pos + (throwStart * deltaTime); // velocity
			pos = pos + 0.5  * deltaTime * deltaTime; // acceleration
			pos[1] = 0;
			Ogre::Vector3 test = boomNode->getPosition();
			test[1] = 0;
			if(isCollision(test, pos, 15000)){
				isThrown = false;
				boomNode->setVisible(false);
			}
			else{
				pos[1] = this->boomNode->getPosition()[1];
				this->boomNode->setPosition(pos);
				this->boomNode->rotate(Ogre::Quaternion(1,0,1,0));
				if(timeKeep > 1.5){
					throwBack = true;
				}
			}
		}
		else if(hitFound == false){
			throwStart = mBodyNode->getPosition() - boomNode->getPosition();
			throwStart = (throwStart + (deltaTime));
			pos = pos + (throwStart * deltaTime); // velocity
			pos = pos + .5 * deltaTime * deltaTime; // acceleration
			pos[1] = 0;
			Ogre::Vector3 test = boomNode->getPosition();
			test[1] = 0;
			if(isCollision(test, pos, 15000)){
				isThrown = false;
				boomNode->setVisible(false);
			}
			else{
				pos[1] = this->boomNode->getPosition()[1];
				this->boomNode->setPosition(pos);
				this->boomNode->rotate(-Ogre::Quaternion(1,0,1,0));
				if((boomNode->getPosition() - mBodyNode->getPosition()).length() < 10){
					isThrown = false;
					boomNode->setVisible(false);
				}
			}
		}
	}
}