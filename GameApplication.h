#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "OIS.h"
#include "Agent.h"
class Agent;

#include "Grid.h"
class Grid;
class GridNode;
#include "Flock.h"
class Flock;

class GameApplication : public BaseApplication
{
private:
	Agent* agent; // store a pointer to the character
	std::list<Agent*> agentList; // Lecture 5: now a list of agents
	std::list<Flock*> flock; // List of Ogre Flock
	std::list<Ogre::ParticleSystem*> sparks; //List of Particle Systems

public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// Lecture 4: keyboard interaction
	// moved from base application
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg);
    bool keyReleased( const OIS::KeyEvent &arg );
	void injectKeyDown(const OIS::KeyEvent& evt);
	void injectKeyUp(const OIS::KeyEvent& arg);
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	Grid* grid;//Added a pointer to the grid
	float time; //Current time to be displayed
	float bestTime; //Lowest time
	bool load2; //Load level 2
	bool load3; //Load level 3
	bool sparkVis;
	////////////////////////////////////////////////////////////////////////////


protected:
    virtual void createScene(void);
	virtual void createGUI(void); // Lecture 16
	OgreBites::ParamsPanel* mParamsPanel; // Lecture 16
};

#endif // #ifndef __TutorialApplication_h_
