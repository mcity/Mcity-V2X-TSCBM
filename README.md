# Mcity Traffic Signal Controller Broadcast Message (tscbm) SPaT parser
Traffic Signal Controller Broadcast Mesasag SPaT format parser/decoders in Python and C. Includes Python/C binding examples.

Used for parsing V2X Signal Phase and Timing Messages (SPaT) sent from traffic controllers in Mini SPaT / MIBSPaT format.

***Bug fixes and updates to the original TSCBM for CAMP TOSCo, SAE CI and CTI T&V projects for MAP/SPAT Analysis***

## Files
### test_TSCBM.py 
* Makes use of Python byte array and uses array indexing to parse results.
* This a bug fixed version that works with the latest Python 3 version.  Used in my PCAP decoder to JSON converter used for the CI/CTI MAP/SPAT Analyzer.

### test_TSCBM_bit.py 
* Uses python bitstring ConstBitStream to implement the same TSCBM parser. About 30% slower than the byte array version, but easier to read and follow.

### test_c_TSCBM.py
* Calls the C library version of the TSCBM parser from Python. Allocates and frees string buffers in C, so that library is "threadsafe".

### TSCBM.c
* Parser for TSCBM format messages written in C. Can be built as stand alone example for shared library. Written for readability vs speed. Possible speed improvements are listed in comments in the C code.

### Makefile
* Builds both the stand alone and shared library version of the C code.

### requirements.txt
* Package listing for packages used with python examples.

## Sample message
All examples attempt to parse the following hex encoded SPaT message:
```hex
cd100100dc02aa0000000000000000020000007d00dc02aa000000000300dc01db000000000000000004003f00bc003f00bc0000000005003f02d400000000000000000600000093003f02d40000000007003f00d2000000000000000008003f01a1003f01a100000000090000000000000000000000000a0000000000000000000000000b0000000000000000000000000c0000000000000000000000000d0000000000000000000000000e0000000000000000000000000f0000000000000000000000001000000000000000000000000000dd0000002200ff00000000000000000000000000000000085d003eca03ce00000000
```

This is the binary representation of that message:
```binary
1100110100010000000000010000000011011100000000101010101000000000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000001111101000000001101110000000010101010100000000000000000000000000000000000000011000000001101110000000001110110110000000000000000000000000000000000000000000000000000000000000000000001000000000000111111000000001011110000000000001111110000000010111100000000000000000000000000000000000000010100000000001111110000001011010100000000000000000000000000000000000000000000000000000000000000000000000110000000000000000000000000100100110000000000111111000000101101010000000000000000000000000000000000000001110000000000111111000000001101001000000000000000000000000000000000000000000000000000000000000000000000100000000000001111110000000110100001000000000011111100000001101000010000000000000000000000000000000000001001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000101100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011011101000000000000000000000000001000100000000011111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100001011101000000000011111011001010000000111100111000000000000000000000000000000000
```

## Usage
### Python
Two python examples are provided. One is an easy to read implementation done with the bitstring library. A second is a faster version done using byte arrays.

Setup 
```sh
$ python3 -m venv venv #Create a new virtual environment
$ source venv/bin/activate #Start a virtual environment
$ pip install -r requirements.txt # Install Python Packages
```

Run
```sh
$ source venv/bin/activate #Start a virtual environment
$ python test_TSCBM.py #Try byte array version
$ python test_TSCBM_bit.py #Try bitstring version
```

### C
The C code in this package can build both stand alone and shared library versions of the code.

```sh
$ make main #Build both the main and library versions. Alternatively make all
$ ./a.out #Try the stand alone version. 
```

Cleanup
```sh
$ make clean #Cleanup all build components.
```

### C Shared Library
This builds the shared library version of the C TSCBM parser.

```sh
$ make library #Build both the main and library versions. Alternatively make all
$ ./a.out #Try the stand alone version. 
```

Depending on your environment you may need to add your PWD to LD_LIBRARY_PATH so that Python can find the shared library.
```sh
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD
```

Cleanup
```sh
$ make clean #Cleanup all build components.
```

### Python C bindings
This is an example of using C bindings from Python to call the C shared library version of the code.

Setup 
```sh
$ python3 -m venv venv #Create a new virtual environment
$ source venv/bin/activate #Start a virtual environment
$ pip install -r requirements.txt # Install Python Packages
$ make library #Build the C library
```

Run
```sh
$ source venv/bin/activate #Start a virtual environment
$ python test_c_TSCBM.py #Test the shared library version of the code.
```

Clean-up
```sh
$ make clean
```
