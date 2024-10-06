# Game Design Document

## Section 0: Summary
This section should tell the reader what to expect in the future sections.  In particular, it should contain at least:
- a working title for your game
- a list of your team members (and their roles if decided)
- a "concept statement" of your game ("your game in a tweet")

### a5traeμs Team & Concept Statement:
The game, a5traeμs, by Ty Schaller, Ying Tan, and Jeana To is a multiplayer space fighting game. 
Pilot your spaceship through epic zero-gravity battles in the far reaches of space. Engage with a friend in multiplayer mayhem, where skill, strategy, and luck determine the ultimate cosmic champion! Make sure to avoid the hazards of outer space as you collect power-ups and points!

## Section 1: Gameplay
This section should address simple questions about how your game works:
- How does your game progress?
- What are the win and loss conditions?
- Are there levels?
- Are there points?

This section should also address:
- **controls** (keyboard? mouse? tongue?)
- **physics** (how does your game incorporate the physics engine?)
- **game flow** (what does the game look like from start to end for the player?)
- **graphics** (will you draw polygons? use sprites? make your own vector graphics?)

### Game Progression & Game Flow:
Both players spawn in. Slowly, obstacles spawn in from off-screen directed at the player with the intention of colliding into the player-controlled body/ship. Players should attempt to avoid these obstacles while simultaneously trying to shoot them and each other down. Keeping track of the bullet-limiting restraints (shooting cooldown) in the process, the players should stay alive and continue fighting. Power-ups spawn randomly and can be collected by players to get boosts in the point system or physics engine. A fully depleted health bar will result in “death" where a player will be unable to play for 5 seconds. The game ends when one of the players reaches 500 points.

### How do you win?
A player wins if they have earned 500 points before their opponent.

### How do you lose? 
A player loses if they fail to earn 500 points before their opponent.

### Levels:
There are no levels. Instead, it is a single environment where the multiplayer action occurs with variability (from asteroids and temporary events affecting physics) in the stage hazards between game playthroughs.

### Points:
Points are collected by shooting and destroying randomly spawning asteroids.

### Game Control:
The keyboard will be used to control the player ship and shoot bullets (Player 1: WASD V; Player 2: ←↑↓→ M), and the mouse will be used to start the game.

### Physics Engine Incorporatation:
Each object rendered on screen must have an underlying body and asset stored in the cache. We will implement the physics engine in creating velocities for the player-controlled bodies based on inputs. Stage hazards will also require physics impulses to create stationary environments that cannot be controlled by exterior forces.

### Graphics Incorporation:
We will use sprites. We will have a sprite on the initial boot-up screen that acts as a start button. Upon pressing this button, we will load the scene with the level and player mechanics. We are going to implement copyright-free sprites as our textures for the player bodies, bullets, asteroids, and scene hazards. Polygons will not be drawn on the screen. We will implement a new function in sdl_wrapper.c that rotates textures based on the bodies underlying them's rotation angle with respect to a central axis.

## Section 2: Feature Set
This section should reduce your game to a set of individual features (remember iterative development?).  Your team should
assign these features to individual group members and assign each one a priority from one to four (1 = game cannot work without this, 4 = cool but not necessary).

We have gathered together the following list of some example features you might choose to implement:
- make your own graphics or sprites
- add sound effects
- implement a scrolling environment
- implement a networked/multiplayer game
- implement 2D parallax (https://en.wikipedia.org/wiki/Parallax)
- implement rendering of text
- implement a mouse handler
- implement an AI for an enemy
- implement speed-independent friction
- implement more accurate integration (current implementation uses a trapezoid sum)
- implement music

### Priority 1:
- Implement spaceship movement and firing mechanics. The movement of the spaceship and bullets should include “skidding” motions like a car on ice. Rotating the SDL_Rects should be done for the rotations of the ship as well as the direction of the bullet projections. Bullets will be shot individually when the appropriate key is pressed and should damage players (-10 player health per bullet, -20 player health with damage-up bullet).
- Design basic space arena environments – e.g. walls and space. Make the arena look appropriately like space (as much as possible). Importantly, we will add a number of wall obstacles and their collision handlers.
- Implement a title screen using abstraction with a scene change function. The title screen should include a start button that takes the players to the game once it is clicked.

### Priority 2:
- Implement 2 scoring systems to track number of score points and health points of players.
- Implement asteroids (moving, spawn at random time intervals across the screen), and their collision handlers. Upon impact, asteroids should damage and explode apart off players (-20 player health per asteroid), and bullets should explode asteroids (-100% asteroid health per bullet).
- Implement 3 power-ups. The effect of a power-up will include increasing player movement speed, increasing bullet damage, or a random number of health points (going over cap if necessary).

### Priority 3:
- Add an energy management system for weapon use. After 50 bullets have been shot, the player will have to hold down the shoot button for their weapon to recharge before they may shoot again.
- Enhance graphics with particle effects for explosions and space effects. This will be implemented as a function that will create said effects using multiple different sprites in quick succession. Intention is to make explosions upon destructive collisions.
- Implement two dynamic, temporary events affecting physics within the game world. This will occur at random within certain intervals throughout the game. These events will include time dilation (where time slows down for the other player for a couple of seconds) and a black hole (reroutes asteroid projectories).

### Priority 4:
- Add sound effects for ship firing, explosions, and soundtrack for immersive gameplay experience.
- Integrate multiplayer functionality (two players playing on the same device).
- Implement a respawn mechanic -- when a player’s health depletes, they must wait 5 seconds before reentering the game. Their sprite should change according to their “ghost” status while they are stationary, dead.

### Priority 5:
- leaderboard functionality
- rotate asteroids in game [DONE BY TY]
- instruction screen [ALMOST COMPLETED BY YING]
- add more unique physics events
- add more unique power-ups
- additional space arenas
- tasks (bounties)
- bullet magnetism (bullets attracted towards target) [HALF DONE BY TY]
- kill/death ratio scoring system

## Section 3: Timeline
This section should assign every feature in the previous section to a particular group member and a particular week they will implement it.

### Week 1:
- **Ty:** 
  - Implement spaceship movement and firing mechanics (Priority 1)
- **Jeana:**
  - Design basic space arena environments (Priority 1)
- **Ying:**
  - Implement a title screen (Priority 1)

### Week 2:
- **Ty:** 
  - Implement 3 power-up pickups in the arena and effects on players (Priority 2).
  - Implement 2 dynamic, temporary events affecting physics within the game world (Priority 3).
- **Jeana:** 
  - Implement asteroids and their collision handlers. (Priority 2)
  - Enhance graphics with particle effects for explosions and space effects (Priority 3).
- **Ying:** 
  - Implement 2 scoring systems to track score points and health points of players (Priority 2).
  - Add an energy management system for weapon use. (Priority 3)

### Week 3:
- **Ty:**
  - Introduce respawn mechanic to let players re-enter the game. (Priority 4)
- **Jeana:** 
  - Add sound effects for ship firing, explosions, and soundtrack for immersive gameplay experience (Priority 4).
- **Ying:** 
  - Implement multiplayer functionality (Priority 4)

## Section 4: Disaster Recovery
This section should describe how each member plans to get back on track if they fall behind.  Please take this section seriously.

### Ty:
- **If fallen behind week 1:**
  - create simpler movement mechanics, i.e. create blockier movement, less fluid more static, 
- **If fallen behind week 2:**
  - push temporary physics event to third week, polish pickups instead
- **If fallen behind week 3:**
  - create less different types of physics events. 

### Ying:
- **If fallen behind week 1:**
  - make title screen less complicated (e.g. just start button)
- **If fallen behind week 2:**
  - make a simpler scoring system for each player (e.g. not worry about displaying in game)
- **If fallen behind week 3:**
  - make multiplayer functionality more limited (e.g. dont give second player entire ship, instead, let them be an obstacle for player 1)

### Jeana:
- **If fallen behind week 1:**
  - focus on creating the basic space arena environments, create less obstacles or none at all if time doesn't permit.
  - if possible, push designing obstacles to week 2 and create simpler/less asteroids instead.
- **If fallen behind week 2:**
  - create simpler/less particle effect graphics
- **If fallen behind week 3:**
  - add simpler and more repetitive sound effects or just add less sound effects. Choose not to add a soundtrack if there isn't enough time
