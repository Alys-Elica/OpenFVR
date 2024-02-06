# LST scripts

LSTs are written in a proprietary script language designed for the point-and-click mechanics.  
There are likely game-specific plugins not documented on this page.

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
