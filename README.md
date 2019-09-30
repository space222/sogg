# sogg
Converter from Ogg to Simplified Ogg. Meant for vorbis bitstream. 
Ogg format seems over-complicated for a single vorbis track in a file; libogg seemed like a lot of work just 
to get at a series of decoder packets. This converter assumes the Ogg pages are sequential in the file, but will
warn if they aren't. Shouldn't be too hard to do a separate pass to properly re-order them, but so far I'm just 
hoping that won't be necessary most of the time.

# format
Self-explanatory from the end of the code, but for each stream writes out a header of three 32bit values: the serial used in the original Ogg file,
followed by the total stream length (in bytes) and the number of packets. This is followed by the packets, each one
prefixed with only a 16bit length value.


