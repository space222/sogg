# sogg
Converter from Ogg to Simplified Ogg. Meant for vorbis bitstream. 
Ogg format seems over-complicated for a single vorbis track in a file; libogg seemed like a lot of work just 
to get at a series of decoder packets. This converter assumes the Ogg pages are sequential in the file, but will
warn if they aren't. Shouldn't be too hard to do a separate pass to properly re-order them, but so far I'm just 
hoping that won't be necessary most of the time.

# format
Self-explanatory from the end of the code, but for each stream writes out a header of: the serial used in the original Ogg file (32bit),
followed by the total stream length (in bytes, 32bits), the final timestamp of the Ogg (64bits), and the number of packets (32bit). 
This is followed by the packets, each one prefixed with only a 16bit length value.


