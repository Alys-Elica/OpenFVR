# LibFVR

This library contains the code for reading multiple files from the Phoenix VR engine.

## Structure

| Folder       | Description                                    |
| ------------ | ---------------------------------------------- |
| `fvr/`       | General classes                                |
| `fvr_files/` | Classes for reading the different file formats |
| `internal/`  | Classes used internally by the library         |

Classes follow the [opaque pointer](https://en.wikipedia.org/wiki/Opaque_pointer) pattern.

## Currently supported file formats

| File    | Description        | Class name  | Class header             |
| ------- | ------------------ | ----------- | ------------------------ |
| ARN/VIT | Image archive      | `FvrArnVit` | `fvr_files/fvr_arnvit.h` |
| DAT     | Archive            | `FvrDat`    | `fvr_files/fvr_dat.h`    |
| MTR     | Bitmap image       | `FvrMtr`    | `fvr_files/fvr_mtr.h`    |
| PAK     | Compressed archive | `FvrPak`    | `fvr_files/fvr_pak.h`    |
| TST     | Click zones list   | `FvrTst`    | `fvr_files/fvr_tst.h`    |
| VR      | Game images        | `FvrVr`     | `fvr_files/fvr_vr.h`     |
