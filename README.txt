How To Play Boomerang Orangutan:

-To move, us the "I" key for up, "K" for down, "J" for left, and "L" for right.  
-To throw the boomerang, use the space bar
-The objective of the game is to hit all of the Ogre's  with the boomerang while not touching an Ogre yourself.
-The Ogre's will move toward you while your boomerang is moving.
-If you win, hit the "Q" key to load the next level (There are three).
-If you lose, hitting "Q" will restart the same level.
-The game displays the fastest time you defeated the Ogre's in.
-Winning let's you play around with the boomerang until you restart

How The Requirements Were Met:

Game States: The boomerang has three states: Not thrown, where it's invisible, Thrown, where it's going in the direction you throw it
in, and Thrown back, where it's heading toward whereever you are.  The Ogre's have two states, heading away from the player and heading
toward the player.  The game itself also has a Won and a Loss state, which affects what is displayed in the top panel and what level you 
can load.  

Animation: The Orangutan and Ogre's walk and the boomerang rotates while you throw it, and rotates the opposite direction when it comes 
back.

Chance: The Orge's are radomly rotated when they are initialzed, and depending where you hit them when you restart the game, they may
be moved to a different location than where you were expecting.

Levels: There are three levels.  Though they are not much different from each other, I believe this shows that I have the ability to load
different levels

Flocking: The Ogre's use the flocking method to move

Physics: Physics are used to calculate the boomerang's position when it's thrown

Collision detection: Bounding boxes are used for Ogre-to-boomerang collision and Ogre-to-Orangutan collision.  Rays are used for Ogre,
Orangutan, and boomerang collision with the wall.  They should not be able to move through the walls.

Feedback: The GUI displays the time, your best time, and how many Ogre's are left.  The Ogre's disappear when you hit them with the 
boomerang and you disappear if you hit an Ogre.

Differences:  I used rays for collision detection that were not discussed in previous assigments and the way the boomerang moves
is unique.

Bugs:  Occasionally the boomerang fires slightly to the left instead of straight on.  And it's not a bug per se, but since the boomerang
starts a little ahead of the player, then if you stand right up against a wall you can fire the boomerang off the other side.  Also,
I intended the boomerang and Orangutan's materials not to load correctly, I wanted them white.  I tweaked the game to be run in release mode.
Make sure the Optimizations are turned off.  I had some problem opening my level in release mode, but I think that was because the folder
was in my dropbox.  If it still isn't working, uncomment the part I commented out and add the complete path to the text file.  It is 
possible to get stuck in an inner wall, but you have to sidle into it (down, left, down left or up, right, up right).


