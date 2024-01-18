# OpenFVR

WIP documentation of 4X Technologies's Phoenix VR engine used in adventure games like Amerzone, The messenger and some Dracula adventure games.

## Current status

Project is currently focused on documenting the interpreter engine (file formats, scripting, etc.) and creating some basic conversion tools.

### File formats

| File                              | Description        | Status  | Tool                                |
| --------------------------------- | ------------------ | ------- | ----------------------------------- |
| [ARN/VIT](Doc/Formats/ARN_VIT.md) | Archive            | Partial | None                                |
| DAT                               | Archive            | Partial | [DatExtractor](Tools/DatExtractor/) |
| LST                               | Game script        | Unknown | None                                |
| MTR                               | Bitmap image       | Partial | [MtrComverter](Tools/MtrConverter/) |
| [PAK](Doc/Formats/PAK.md)         | Compressed archive | Partial | [PakConverter](Tools/PakConverter/) |
| [TST](Doc/Formats/TST.md)         | Clickable zones ?  | Unknown | None                                |
| [VR](Doc/Formats/VR.md)           | Game images        | Partial | [VrConverter](Tools/VrConverter/)   |

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
