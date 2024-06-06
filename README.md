# OpenFVR

WIP documentation of 4X Technologies's Phoenix VR engine used in adventure games like Amerzone, The messenger and some Dracula adventure games.

## Current status

Most file formats are now at least partially understood and have some kind of reading tool.  

Development of an experimental (read hacky, quick and dirty code hell) [engine](Tools/Libraries/FvrEngine/) has been started. It is able to *run* some games but needs some convertion (more like file copying).  
What remains to be done in the engine is mostly script function/plugin implementation and optimisation/better code. Some parts (like the inefficient VR cubemap renderer) may be entirely rewritten if necessary.

From then on it is possible to start implementing games (`Louvre: The final curse` - aka `The messenger` - is being worked on [here](Tools/Games/LouvreFinalCurse/README.md)).

## File formats

| File                              | Description        | Documentation | Tool                                      |
| --------------------------------- | ------------------ | ------------- | ----------------------------------------- |
| [ARN/VIT](Doc/Formats/ARN_VIT.md) | Archive            | Partial       | [ArnVitConverter](Tools/ArnVitConverter/) |
| DAT                               | Archive            | Partial       | [DatExtractor](Tools/DatExtractor/)       |
| [LST](Doc/Formats/LST.md)         | Game script        | Partial       | None, but a basic parser exists in LibFVR |
| MTR                               | Bitmap image       | Partial       | [MtrComverter](Tools/MtrConverter/)       |
| [PAK](Doc/Formats/PAK.md)         | Compressed archive | Partial       | [PakConverter](Tools/PakConverter/)       |
| [TST](Doc/Formats/TST.md)         | Clickable zones    | Documented    | [VrConverter](Tools/VrConverter/)         |
| [VR](Doc/Formats/VR.md)           | Game images        | Partial       | [VrConverter](Tools/VrConverter/)         |

### Kaitai files

Some Kaitai struct files and parsers exists, but they mainly are test stuff. Consider it an optional addition to documenting files, the real documentation should still be done in Markdown.

## Contribute

Contributions to the documentation and tools are widely welcome.  
Tools must be kept "simple". More complete/advanced tools may be created once most of the original engine is documented.  
Any generally used language is allowed (Python, C/C++ or Rust for example)  
Copyrighted content is NOT allowed. Do not include game data, binaries, etc.

Feel free to fork this project and/or join the [Discord](https://discord.gg/jtAgMwrMFH) if you have suggestions or ideas.
