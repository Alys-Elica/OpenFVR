# LST scripts

LSTs are written in a proprietary script language designed for the point-and-click mechanics.  
There are game-specific plugins not documented on this page.

TODO: Describe the functions

## Declaration statements

### [bool]

Declares a named boolean variable, which are typically used for control statements.  
All booleans must be declared before any warpzones are declared and are accessible by all warpzones.  
The booleans are then altered throught the `set` statement defined within the game mechanics.

Example:

```
[bool]=var
```

### [warp]

Declares a game area from a VR and TST file. Each warp declaration contains a number of test declarations, which together form the logic and interactions per scene.  
Areas are connected with the `gotowarp` statement within the game mechanics.

#### [test]

The game mechanics are described per interactive zone and are fired when the corresponding zone has been clicked.  
Each "zone" is refered to by its index within the TST-file, with the exception of `-1` which fires when warping to the scene.

## Functions

These are the generic functions present in the engine. Plugin functions - placed in `plugin` and `endplugin` blocs are game specific and will not be documented here. There may be more undocumented functions not listed here.

### gotowarp

Teleports the player to the given warp.

### playsound

Plays a given sound. The other parameters are supposed to be the volume, and the play count (-1 for infinite sound looping, e.g. ambient sounds).

### stopsound

Stops the given sound.

### playsound3d

### stopsound3d

### playmusique

### stopmusique

### set

Sets the given variable to the given value.

### lockkey

Locks a given warp to a key (e.g. right click to go the inventory warp).

| Key value | Corresponding key/button |
| --------- | ------------------------ |
| 0         | Escape                   |
| 12        | Right click              |

### resetlockkey

Clears the locked keys.

### setcursor

Sets the given cursor file to draw when the mouse is over the given warp and test zone. 

### setcursordefault

### fade

### end

### setangle

### hidecursor

### not

### anglexmax

### angleymax

### setzoom

### interpolangle
