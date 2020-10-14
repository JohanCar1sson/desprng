# DES PRNG

Lightweight (seven bytes of state) pseudo random number generator (PRNG) suitable for GPU computing with OpenACC. Based on the original Data Encryption Standard (DES) block cipher. Yes, I do realize it will be pronounced "despairing"...

Type "make" to build the libdesprng.a library, and the toypicmcc driver. The driver produces an output file xi.dat. Use xiplot.py to make a plot. It should look something like this:
![xi.png](http://crowscience.com/files/xi.png)

There should be no significant difference in the output for code executed on CPU and GPU, respectively. 

You'll need a fairly recent version of nvc to produce correct code for GPU. For nvc 20.4, "-O2" gives correct results, but "-O0" does not! We are told that for nvc 20.9, any reasonable optimization level works.

Makefile.crush and the three source files crush?.c can be used to test DES PRNG on CPU. You'll need to install the
[TestU01 library](http://simul.iro.umontreal.ca/testu01/tu01.html)
to get access to the Crush test suite used.

The files d3des.h, d3des.c and oldnewcomparison.c are used for regression testing. Run oldnewcomparison to produce the two output files desprng.out and d3des.out that should be identical. d3des is a public-domain DES implementation
[available as a ZIP archive on Bruce Schneier's web site](https://www.schneier.com/sccd/DES-OUTE.ZIP).
