<!--
[![GitHub Downloads](https://img.shields.io/github/downloads/KojoBailey/XFBIN-Parser-PlusPlus/total)](https://github.com/KojoBailey/XFBIN-Parser-PlusPlus/releases)
-->
### Table of Contents
- [Introduction](#cyberconnect2-xfbin-parser)
- [Usage](#usage)
  - [Unpacking XFBINs](#unpacking-xfbins)
  - [Repacking XFBINs](#repacking-xfbins)
  - [Converting from binary and JSON](#converting-from-binary-and-json)
  - [Merging](#merging)

# CyberConnect2 XFBIN Parser++
This tool aims to be the **ultimate parser** for the **XFBIN** file container format created by **CyberConnect2**, used in the games developed in their own engine, sometimes referred to as the **NU** engine.

For those unfamiliar, XFBIN files use the `.xfbin` file extension, and contain the magic (first 4 bytes of a file) `NUCC`. They are a container since they contain other file data within them in **pages** and **chunks** (such as `nuccChunkBinary` and `nuccChunkModel`). For more information, see the [XFBIN wiki page](https://jojomodding.miraheze.org/wiki/xfbin) on JoJo's Bizarre Modding Wiki.

What this tool does is **extract** these chunks by separating them into different files for easier viewing, replacing, or removing; it is similar to the [XFBIN Parser by SutandoTsukai181](https://github.com/SutandoTsukai181/xfbin_lib). It then goes a step further and offers to convert any `nuccChunkBinary` to **JSON**, a format that is much easier to read and edit quickly.

<details>
<summary><b>See an example of JSON</b></summary>

<br/>This data isn't taken from any particular file. Notice how **easy** it would be to edit the different parameters, as well as how useful it can be just to read them for understanding.
```json
{
  "Metadata": {
    "Chunk Map Index": 1,
    "Name": "example",
    "Type": "nuccChunkBinary",
    "Path": "cmnparam/bin/230/example.bin"
  },
  "Some Character ID": "4tno01",
  "Random Number": 1987,
  "An Array": {
    "Bing": "bong",
    "Magic Number": 23
  },
  "Some String": "Do you believe in \"gravity\"?",
  "Some Japanese String": "調理場という所は... バイキンが一番の敵デスッ！"
}
```
</details>

Since every `nuccChunkBinary` is different, separate functions need to be defined for the conversion of each one. Currently, I as the creator of this tool am focusing on support for data from *JoJo's Bizarre Adventure: All-Star Battle R*, but adding support for other games will also be possible eventually - faster with contributions from others.

This tool is very much still a **WORK IN PROGRESS**, so until the first release, everything below is what I aim to have the tool be able to do.

## Installation
This tool is a **WORK IN PROGRESS** and therefore not yet available for download. An initial release will be made when it is at least somewhat functional (otherwise it'd be useless to share).

## Usage
### Unpacking XFBINs
To **unpack an XFBIN**, simply drag-and-drop it onto the tool's EXE. A folder will then be created with the same name as that XFBIN, and if a folder of the same name already exists, the name will be slightly altered instead (e.g. `xfbin (0)`) to prevent accidental file overwriting/replacement. Inside this folder will be `_xfbin.json`, and other folders with a naming format like `[XXX] Y (nuccChunkZ)`.
- `[XXX]` → The **page** index, as each of these folders is representative of an XFBIN page.
- `Y` → The page's name, determined by the most dominant **chunk** from the page.
- `(nuccChunkZ)` → The dominant chunk's type (e.g. `nuccChunkTexture`, `nuccChunkModel`, `nuccChunkBinary`, etc.)

An example folder name could be `[055] cmn_color_thumbnail_icon (nuccChunkAnm)`.

The `_xfbin.json` contains information directly about the XFBIN itself. This file **is** important, and shouldn't be modified unless you know what you're doing.

The folders are representative of the different **pages** within the XFBIN, which in turn contain **chunk** data. Within these, you will find a `_page.json` and (usually) at least one file containing the chunk's data.

### Repacking XFBINs


### Converting from binary and JSON


### Merging


<!--
To convert a file **to JSON or back**, simply _drag_ the file you want onto the EXE.
Ensure that the formatting is correct for the XFBIN or JSON, as incorrect formatting will cause errors. Feel free to report any errors you do come across, and I'll add error detection for those specific things in future releases.

### Merging
To **merge JSON files**, _open_ the EXE and input (text) the type of file you want to merge.

You will need to have prepared a folder within the `merging` folder with the name of your wanted data type (e.g. `merging/messageInfo`).

Within that folder, have the JSON files you want to merge with the original (original or "base" JSON is already included in the `data` folder) alongside a corresponding TXT file of the same name. For example, `apple.json` would be accompanied by `apple.txt`. Inside these JSON files, you only need to include what you want to modify:

```json
{
    "1c30e109": {
    "message": "bro just got violated",
    "is_ref": -1,
    "ref_crc32": "",
    "char_id": 70,
    "cue_id": 84
  },
    "6416af04": {
    "message": "woohoo yippee",
    "is_ref": 1,
    "ref_crc32": "c37860ee",
    "char_id": 70,
    "cue_id": 83
  }
}
```
^ An example for `messageInfo`. Include the key (in this case the CRC32 ID) and then all the data within it, with whatever you want modified.

Within these TXTs should be a **priority integer**. This determines the order in which the files are merged, which is useful for dealing with files that try to modify the same values. **Higher number = higher priority (merges after others)**. For example, `apple.json` with priority `5` will override `pear.json` with priority `2`.

