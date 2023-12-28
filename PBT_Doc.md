# Parser Binary Template (PBT) - Syntax Doc

**PBT**, 'Parser Binary Template',  is a file extension created by [Kojo Bailey](https://github.com/KojoBailey) for his [tool](https://github.com/KojoBailey/CC2-Parser-Plus) that parses **binary data** files into JSON and back. As the name implies, they act as **templates** for the parser to read and **act** based on.

PBTs are intended to be similar to [010 Editor](https://www.sweetscape.com/010editor/) **BT** files, and like BTs, are very similar in syntax to [**C**](https://en.wikipedia.org/wiki/C_(programming_language)), although with built-in functions, types, and more features that will prove useful to users. 

This doc outlines all the **syntax** for in PBT files, outlining the similarities to BT and C files, and explaining the stuff that is new or different. There are also many **examples** included to help make your understanding as clear as possible. Since PBT is a custom language, syntax highlighting from **C/C++** will be used, although note that some things may not be highlighted appropiately anyway.

## Structure
First of all, PBTs do not require any access point other than the start of the file itself. This means no `int main()` or anything to start.

Although the syntax of PBT is based on C, it does not have support for C or C++ libraries, and `#include` will therefore not work. The same goes for the other `#` commands, like `#define` or `#using`.
