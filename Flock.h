#ifndef FLOCK_H
#define FLOCK_H

#include "BaseApplication.h"
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <list>
#include <map>
#include <utility>


#include "Grid.h"
class Grid;
class GridNode;

#include "Agent.h"
class Agent;


class Flock
{
private:	
	
	float scale;						// scale of character from original model


	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

	Ogre::AnimationState* mAnims[13];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[13];						// which animations are fading in
	bool mFadingOut[13];					// which animations are fading out
	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping
	OIS::Keyboard* mKeyboard;

	void setupAnimations();					// load this character's animations
	void fadeAnimations(Ogre::Real deltaTime);				// blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime);			// update the animation frame

	// for locomotion
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking

	//////////////////////////////////////////////
	// Lecture 4
	bool procedural;						// Is this character performing a procedural animation
    //////////////////////////////////////////////
public:
	Flock(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale, Grid* g, Ogre::Camera* c);
	~Flock();
	void setPosition(float x, float y, float z);

	void update(Ogre::Real deltaTime);		// update the agent
	
	void setBaseAnimation(AnimID id, bool reset = false);	// choose animation to display
	void setTopAnimation(AnimID id, bool reset = false);
	int getRCoord();	//Calculate the Agent's grid coordinates using it's position
	int getCCoord();
	double routeSoFar;
	Grid* grid;
	Ogre::SceneNode* mBodyNode;	
	float height;						// height the character should be moved up
	Ogre::Vector3 mKeyDirection;      // player's local intended direction based on WASD keys
	Ogre::Vector3 mGoalDirection;
	Ogre::Camera* cam;
	void doTheThing(); //Changes from walking to idle
	bool isCollision(const Ogre::Vector3& position, const Ogre::Vector3& direction);
	Ogre::RaySceneQuery* mRaySceneQuery;
	Ogre::Entity* mBodyEntity;
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph	
	Ogre::Vector3 separate(double radius, std::list<Flock*> agents);//Calculate the separation velocity
	Ogre::Vector3 alignment(double radius, std::list<Flock*> agents);//Calculate the alignment velocity
	Ogre::Vector3 cohesion(double radius, std::list<Flock*> agents);//Calculate the cohesion velocity
	void flockMove(Ogre::Vector3 sep, Ogre::Vector3 als, Ogre::Vector3 coh, Ogre::Real deltaTime);
	std::list<Flock*> flockList;			//List of other Agents' on Grid
	void stopTheWalk();						//Stops all of the Agents' walking
	void startTheWalk();					//Start an Agent's run animation
	Agent* player;	//Pointer to player
	Ogre::Vector3 getGoal(); //Gets flocks goal
	bool keepFlock; //Keep flocking
};

#endif