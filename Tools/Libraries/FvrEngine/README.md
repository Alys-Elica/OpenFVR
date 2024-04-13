# FvrEngine

Experimental engine library.

## Status

This is a release of my previously private WIP engine for running FVR games.  
It is still heavily experimental/unstable/incomplete and WILL change/break.

## Structure

It is based on a main Engine class that manages everything. To use it, create an instance of the Engine class, register potential LST plugin functions and run the engine loop (see [examples](../../Games/README.md)).

Game files are located inside the `data` folder with the following sub-directories :

| Sub-directory | Description      |
| ------------- | ---------------- |
| audio         | -                |
| image         | -                |
| script        | -                |
| tst           | Clickable zones  |
| video         | -                |
| warp          | Static/VR images |
