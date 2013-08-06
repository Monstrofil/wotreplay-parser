World of Tanks Replay Parser
==============================================================

# What?

This program aims to decode replay files and convert them into more accessible formats such as an image or json files. Which may be uses in other programs. The alternative project [wotreplay-viewer](http://github.com/evido/wotreplay-viewer) implements a small HTML5 viewer for the JSON data. 

When using the json output option, the replay is decoded into a list of packets, in the same way it is described in the replay file. The following information can be extracted from from the replay.

* Position
* Health
* Tanks being tracked
* Tanks getting shot or killed
* Hull Orientation
* Messages

# Build

## Dependencies

* OpenSSL
* Boost (Complete)
* LibPNG
* LibXML2
* libjson-cpp (included in the source, from [http://jsoncpp.sourceforge.net/](http://jsoncpp.sourceforge.net/))

## Compiler Support

* GCC (>= 4.7)
* CLANG

## Compile

    git clone https://github.com/evido/wotreplay-parser.git
    cmake .
    make

You may need to specify the include directory for LibXML2 using the CMAKE variable INCLUDE_DIRECTORIES. This can be done as following.

    cmake -DINCLUDE_DIRECTORIES="/usr/include/libxml2" .

# Data

To use this software you need some data extracted from World of Tanks, this data contains some elements to draw the mini map, the actual minimaps and the arena definitions. I have prepared an archive which is available [here](https://docs.google.com/file/d/0B3nvac-cSAKRdjV1d2JtQkJJZkE/edit?usp=sharing). This should be available in the ```root``` folder (see next paragraph).

# Run

# Decode Replays

The program can be used to convert wotreplay files to images or json files.

    wotreplay-parser --parse --root <working directory> --type <output type> --input <input file> --output <output file>

* ```output type``` can be png or json
* ```output``` is optional, the program will then write to stdout
* ```input``` and  ```output``` are relative to root
* ```root ``` should contain a folder maps with the images to maps and the arena definitions

# Create Minimaps

The program can also be used to create all the minimaps for usage in [wotreplay-viewer](http://github.com/evido/wotreplay-viewer).

    wotreplay-parser --create-minimaps --root <working directory> --output <output directory>

*```root``` the working directory containing the necessary data
*```output``` the output directory relative to the root directory

# License

All code is licensed under the permissive BSD license.
