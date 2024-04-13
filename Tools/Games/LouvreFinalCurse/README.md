# Louvre: The final curse

Adventure game centered around mystical artifacts called Satan's Keys hidden inside the Louvre. Also with time travel.

## Status

The game is still in a early experimental state.

It *runs* but is still heavily unplayable. The main menu kind of works, and a new game can be started but you will quickly become stuck as inventory and object usage are still not properly implemented.

## Data convertion

To run the game, you must first provide data from the original 2 CD-ROMs.  
Once the project is compiled, create a `input` directory alognside the executable. In this `input` directory create 2 additional directories called `CD1` and `CD2`, then in each directory copy the entire content of the correcponding game disc. Then execute the LouvreConverter executable; it will copy the needed game data to a new `data` directory.  

With the newly created `data` directory, you can now run the LouvreFinalCurse executable to *enjoy* the game.
