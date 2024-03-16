# VrConverter

This CLI software loads the `dct.dll` and `Fnx_vr.dll` libraries (these are used by games developed with the PhoenixVR engine) to do the following:

-   Export VR files to PNG images (with conversion to cubemap images possible)
-   Export VR animations to WebP (with conversion to cubemap images possible)
-   View panoramic 360 VR files in the glorious old first person view

If you plan to use this software, you will need to provide your own copies of the `dct.dll` and `Fnx_vr.dll` libraries (check in the installation folder of your compatible game to find them).  
If you want to compile the source code yourself, it MUST be in `32 bit` version as there is no known version of the engine made for 64 bit PCs.

> `Is it possible to create/edit VR files ?`  
> In theory, this should be possible, yes. The `dct.dll` library contains functions to create images with the compression algorithm used in the VR file format. However it is not planned right now.
