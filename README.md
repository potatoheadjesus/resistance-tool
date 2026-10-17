# resistance-tool
A simple tool that translates resistor colour codes. Written in C.

Encoding mode converts numerical resistance and tolerance values into 5-band or 4-band identification.

Like this:    ./resistance-tool encode [band count] [resistance] [tolerance]

Decoding mode converts 5-band or 4-band identifications back to numerical resistance and tolerance values.

Like this:    ./resistance-tool decode [color 1] ... [color n]
