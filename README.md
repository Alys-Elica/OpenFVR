# OpenFVR

WIP documentation of 4X Technologies's Phoenix VR engine used in adventure games like Amerzone, The messenger and some Dracula adventure games.

## Current status

Project is currently focused on documenting the interpreter engine (file formats, scripting, etc.) and creating some basic conversion tools.

### File formats

| File                              | Description                                                                      | Status                                            |
| --------------------------------- | -------------------------------------------------------------------------------- | ------------------------------------------------- |
| [ARN/VIT](Doc/Formats/ARN_VIT.md) | Archive format - VIT contains the index - ARN contains the files data            | Partial - Some fields are still unknown           |
| DAT                               | Archive format                                                                   | No doc done yet - Tool exists                     |
| LST                               | Custom game scripts                                                              | No doc done yet - Tool exists                     |
| MTR                               | Bitmap format                                                                    | No doc done yet - Tool exists                     |
| [PAK](Doc/Formats/PAK.md)         | Compressed archive format - Mainly used for storing script files                 | Partial - Other compression level/types may exist |
| [TST](Doc/Formats/TST.md)         | Used in conjunction with most VR files - Suspected of containing clickable zones | Content still unknown                             |
| [VR](Doc/Formats/VR.md)           | Game images - These can be static 640\*480 images or 256\*6144 panoramic images  | Partial - DCT not fully documented                |

### Tools

| Tool                                | Description                          |
| ----------------------------------- | ------------------------------------ |
| [DatExtractor](Tools/DatExtractor/) | Extracts files from DAT files        |
| [MtrComverter](Tools/MtrConverter/) | Converts MTR images to PNG           |
| [PakConverter](Tools/PakConverter/) | Extracts content from PAK files      |
| [VrConverter](Tools/VrConverter/)   | Converts and displays VR image files |

### Kaitai files

Some Kaitai struct files and parsers exists, but they mainly are test stuff. Consider it an optional addition to documenting files, the real documentation should still be done in Markdown.

## Future plans

Once the engine is mostly documented, work will be initiated to eventually create an open source interpreter that can be used to play these games on modern platforms (Windows, Linux, MacOS and even consoles).

Ideally, a more complete game engine could be made (including features like greater resolutions, better graphics and LUA scripting) to create new games while still providing older games compatibility (maybe through the use of one-way conversion tools).

## Contribute

Contributions to the documentation and tools are widely welcome.  
Tools must be kept "simple". More complete/advanced tools may be created once most of the original engine is documented.  
Any generally used language is allowed (Python, C/C++ or Rust for example)  
Copyrighted content is NOT allowed. Do not include game data, binaries, etc.

Feel free to fork this project and/or join the [Discord](https://discord.gg/jtAgMwrMFH) if you have suggestions or ideas.
