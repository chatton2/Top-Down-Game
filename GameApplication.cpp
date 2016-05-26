#include "GameApplication.h" // Lecture 5
#include <fstream>
#include <sstream>
#include <map> 

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
	time = 0;
	bestTime = 10000;
	agent = NULL; // Init member data
	load2 = false;
	load3 = false;
	sparkVis = false;
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
	mSceneMgr->clearScene();
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
}

void GameApplication::createGUI(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 16
	if (mTrayMgr == NULL) return;
	using namespace OgreBites;
	
	// Lecture 16: Setup parameter panel: Updated in addTime
	Ogre::StringVector items;
	items.push_back("Ogre's Left");
	items.push_back("Time");
	items.push_back("Best Time");
	mParamsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOP,"SamplePanel",250,items);

	mTrayMgr->showAll();

	//////////////////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////
// Lecture 5: Returns a unique name for loaded objects and agents
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

std::string getKnotName() // In order to check for collisions on walls
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "Carl" + s;	// append the current count onto the string
}

// Lecture 5: Load level from file!
void // Load the buildings or ground plane, etc
GameApplication::loadEnv()
{
	using namespace Ogre;	// use both namespaces
	using namespace std;

	class readEntity // need a structure for holding entities
	{
	public:
		string filename;
		float y;
		float scale;
		float orient;
		bool agent;
	};

	ifstream inputfile;		// Holds a pointer into the file

	string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "levelChynna.txt"; //if txt file is in the same directory as cpp file
//	inputfile.open(path);
	inputfile.open("C:/Users/Chynna/Desktop/levelChynna.txt"); // bad explicit path!!!
	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	// Hmm. No output?
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;	// read in the dimensions of the grid
	string matName;
	inputfile >> matName;	// read in the material name

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);

	grid = new Grid(mSceneMgr, z, x); // Set up the grid. z is rows, x is columns
	string buf;
	inputfile >> buf;	// Start looking for the Objects section
	while  (buf != "Objects")
		inputfile >> buf;
	if (buf != "Objects")	// Oops, the file must not be formated correctly
	{
		cout << "ERROR: Level file error" << endl;
		return;
	}

	// read in the objects
	readEntity *rent = new readEntity();	// hold info for one object
	std::map<string,readEntity*> objs;		// hold all object and agent types;
	while (!inputfile.eof() && buf != "Characters") // read through until you find the Characters section
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> rent->y >> rent->orient >> rent->scale;  // read the rest of the line
			rent->agent = false;		// these are objects
			objs[buf] = rent;			// store this object in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	while  (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters
	while (!inputfile.eof() && buf != "World") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> rent->y >> rent->scale; // read the rest of the line
			rent->agent = true;			// this is an agent
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}
	delete rent; // we didn't need the last one

	// read through the placement map
	char c;
	Ogre::Camera* cammy = mCameraMan->mCamera; //Pointer to camera
	for (int i = 0; i < z; i++)			// down (row)
		for (int j = 0; j < x; j++)		// across (column)
		{
			inputfile >> c;			// read one char at a time
			buf = c + '\0';			// convert char to string
			rent = objs[buf];		// find cooresponding object or agent
			if (rent != NULL)		// it might not be an agent or object
				if (rent->agent)	// if it is an agent...
				{
					if(buf.compare("s") == 0){
					// Use subclasses instead
						agent = new Agent(this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale, grid, cammy);
						agentList.push_back(agent);
						agent->setPosition(grid -> getPosition(i,j).x, rent->y, grid -> getPosition(i,j).z);
					}
					else if(buf.compare("b") == 0){
						Flock* flo = new Flock(this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale, grid, cammy);
						flock.push_back(flo);
						flo->setPosition(grid -> getPosition(i,j).x, rent->y, grid -> getPosition(i,j).z);
						flo->player = agent;//Gives it a pointer to the player
					}
					// If we were using different characters, we'd have to deal with 
					// different animation clips. 
				}
				else	// Load objects
				{
					grid -> loadObject(getKnotName(), rent->filename, i, rent->y, j, rent->scale);
				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
					//Uses knotName to make it easy to check for collisions
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
					grid -> getNode(i,j)->setOccupied();  // indicate that agents can't pass through
					mNode->setPosition(grid -> getPosition(i,j).x, 10.0f, grid -> getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(grid -> getPosition(i,j).x, 0.0f, grid -> getPosition(i,j).z);
					ps->setVisible(false);
					sparks.push_back(ps);
				}
				else if (c == 'G')//Checks for the Goal location
				{
					grid->Goal = grid->getNode(i,j);
				}
			}
		}
	
	// delete all of the readEntities in the objs map
	rent = objs["s"]; // just so we can see what is going on in memory (delete this later)
	
	std::map<string,readEntity*>::iterator it;
	for (it = objs.begin(); it != objs.end(); it++) // iterate through the map
	{
		delete (*it).second; // delete each readEntity
	}
	objs.clear(); // calls their destructors if there are any. (not good enough)
	
	inputfile.close();
	grid->printToFile(); // see what the initial grid looks like.
	agent->flockList = flock;
}

void // Set up lights, shadows, etc
GameApplication::setupEnv()
{
	// set shadow properties
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(Ogre::ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	//Changes to overhead camera

	mCameraMan->mCamera->setPosition(0, 314.6 ,0);
	mCamera->lookAt(grid->getPosition(grid->Goal->getRow(),grid->Goal->getColumn()));
//	mCamera->lookAt(Ogre::Vector3(0,0,0));
	mCameraMan->mCamera->rotate(Ogre::Quaternion(sqrt(0.48),	0,	-.5,	0));

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Ogre::Light* light = mSceneMgr->createLight();
	light->setType(Ogre::Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(Ogre::ColourValue::White);

}

void // Load other props or objects
GameApplication::loadObjects()
{

}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{

}

void
GameApplication::addTime(Ogre::Real deltaTime)
{
	// Lecture 5: Iterate over the list of agents
	std::list<Agent*>::iterator iter;
	for (iter = agentList.begin(); iter != agentList.end(); iter++)
		if (*iter != NULL)
			(*iter)->update(deltaTime);
	std::list<Flock*>::iterator it;
	for (it = flock.begin(); it != flock.end(); it++){
		if (*it != NULL){
			(*it)->flockList = this->flock;
			(*it)->update(deltaTime);
		}
	}
	//If the player got a new high score
	if(agent != NULL){
	if(agent->gameWin == true && time < bestTime){
		bestTime = time;
	}
	//If the player is still playing
	else if(agent->gameLose == false && agent->gameWin == false){
		time = time + deltaTime;
	}
	mParamsPanel->setParamValue(0, Ogre::StringConverter::toString(agent->left));
	mParamsPanel->setParamValue(1, Ogre::StringConverter::toString(time));
	mParamsPanel->setParamValue(2, Ogre::StringConverter::toString(bestTime));
	//If the player has lost
	if (agent->gameLose == true){
		mParamsPanel->setParamValue(1, "Game Over!");
	}
	//If the player has won
	if (agent->gameWin == true){
		mParamsPanel->setParamValue(1, "You Win!");
		if(sparkVis == false){
			std::list<Ogre::ParticleSystem*>::iterator it = this->sparks.begin();
			for(it; it != sparks.end(); it++){
				(*it)->setVisible(true);
			}
			sparkVis = true;
		}
	}
	}
}

bool 
GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
	//Walk up
	 else if(arg.key == OIS::KC_I)
    {
        agent->mKeyDirection.z = -1;
		agent->doTheThing();
    }
	//Walk down
	 else if(arg.key == OIS::KC_K)  
    {
       agent->mKeyDirection.z = 1;
	   agent->doTheThing();
    }
	 //Walk Left
	  else if(arg.key == OIS::KC_J)  
    {
        agent->mKeyDirection.x = -1;
		agent->doTheThing();
    }
	 //Walk Right
	  else if(arg.key == OIS::KC_L)  
    {
        agent->mKeyDirection.x = 1;
		agent->doTheThing();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	//Restart the level
	else if(arg.key == OIS::KC_Q)   // refresh all textures
    {
		//If the player has won or lost
        if(agent->gameWin == true || agent->gameLose == true){
			//If player beat first level, load the second one
			if(agent->gameWin == true && load2 == false){
				Ogre::Entity* ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				//Uses knotName to make it easy to check for collisions
				ent->setMaterialName("Examples/RustySteel");
				Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(5,5)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(5,5).x, 10.0f, grid -> getPosition(5,5).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				//Uses knotName to make it easy to check for collisions
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(5,6)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(5,6).x, 10.0f, grid -> getPosition(5,6).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(5,7)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(5,7).x, 10.0f, grid -> getPosition(5,7).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(5,8)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(5,8).x, 10.0f, grid -> getPosition(5,8).z);
				load2 = true;
			}
			//If player beat second level, load the third one
			else if(agent->gameWin == true && load2 == true && load3 == false){
				Ogre::Entity* ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				//Uses knotName to make it easy to check for collisions
				ent->setMaterialName("Examples/RustySteel");
				Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(8,25)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(8,25).x, 10.0f, grid -> getPosition(8,25).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				//Uses knotName to make it easy to check for collisions
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(9,25)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(9,25).x, 10.0f, grid -> getPosition(9,25).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(10,25)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(10,25).x, 10.0f, grid -> getPosition(10,25).z);
				ent = mSceneMgr->createEntity(getKnotName(), Ogre::SceneManager::PT_CUBE);
				ent->setMaterialName("Examples/RustySteel");
				mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
				mNode->attachObject(ent);
				mNode->scale(0.1f,0.2f,0.1f); // cube is 100 x 100
				grid -> getNode(11,25)->setOccupied();  // indicate that agents can't pass through
				mNode->setPosition(grid -> getPosition(11,25).x, 10.0f, grid -> getPosition(11,25).z);
				load3 = true;
			}
			//Go through each Ogre and reset them to the field and start their flocking
			std::list<Flock*>::iterator it = this->flock.begin();
			for(it; it != flock.end(); it++){
				//If Ogre is in wall or on Orangutan, move it 
				(*it)->mBodyNode->setVisible(true);
				if((*it)->getRCoord() <= 5 || (*it)->getCCoord() <= 5){
					(*it)->mBodyNode->setPosition(grid->getPosition(10, 10));
				}
				if(load2 == true && (*it)->getRCoord() == 5){
					(*it)->mBodyNode->setPosition(grid->getPosition(6, (*it)->getCCoord()));
				}
				if(load3 == true && (*it)->getCCoord() == 25){
					(*it)->mBodyNode->setPosition(grid->getPosition((*it)->getRCoord(), 26));
				}
				(*it)->setPosition((*it)->mBodyNode->getPosition()[0], (*it)->height, (*it)->mBodyNode->getPosition()[2]);
				(*it)->keepFlock = true;
			}
			//Move the agent back to their starting position
			float h = agent->mBodyNode->getPosition()[1];
			Ogre::Vector3 pos = grid->getPosition(2,2);
			pos[1] = h;
			agent->mBodyNode->setPosition(pos); 
			agent->boomNode->setVisible(false);
			agent->gameWin = false;
			agent->gameLose = false;
			time = 0;
			agent->left = 4;
			agent->mBodyNode->setVisible(true);
			std::list<Ogre::ParticleSystem*>::iterator iter = this->sparks.begin();
			for(iter; iter != sparks.end(); iter++){
				(*iter)->setVisible(false);
			}
			agent->isThrown = false;
			sparkVis = false;
		}
    }
   else if (arg.key == OIS::KC_SPACE)//IF space key is pressed
    {
		//Throw the boomerang if the player hasn't lost yet
		if(agent->gameLose == false){
			agent->timeKeep = 0; //Keeps track of how long boomerang's been out
		    agent->throwStart = 50 * (agent->mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z);//Starting velocity
			agent->throwBack = false;
			agent->boomNode->setVisible(true);
			agent->boomNode->setPosition(agent->mBodyNode->getPosition() + (5 * agent->mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z));
			//Throw it a little ahead of the player
			agent->isThrown = true;
		}
    }
 //   mCameraMan->injectKeyDown(arg);
    return true;
}

//Move the player
void GameApplication::injectKeyDown(const OIS::KeyEvent& evt){
		if (evt.key == OIS::KC_I) agent->mKeyDirection.z = -1;
		else if (evt.key == OIS::KC_J) agent->mKeyDirection.x = -1;
		else if (evt.key == OIS::KC_K) agent->mKeyDirection.z = 1;
		else if (evt.key == OIS::KC_L) agent->mKeyDirection.x = 1;
}

void GameApplication::injectKeyUp(const OIS::KeyEvent& arg){

}

//If the kep is not released
bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
		if (arg.key == OIS::KC_I && agent->mKeyDirection.z == -1) agent->mKeyDirection.z = 0;
		else if (arg.key == OIS::KC_J && agent->mKeyDirection.x == -1) agent->mKeyDirection.x = 0;
		else if (arg.key == OIS::KC_K && agent->mKeyDirection.z == 1) agent->mKeyDirection.z = 0;
		else if (arg.key == OIS::KC_L && agent->mKeyDirection.x == 1) agent->mKeyDirection.x = 0;
		agent->doTheThing();
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    return true;
}
