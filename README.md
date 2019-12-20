chip9 emulator
==============

about
-----

emulator for the imaginary CHIP9 cpu architecture, described in the pdf.
written in C for good speed.

includes disassembler, fast 8bit SDL rendering.

bugs
----
this emulator assumes little endian CPU.
the addx command probably doesn't set the flag register as expected.
it was almost impossible to figure out the semantics for 16 bit store.
i ended up copying some macros from gnubuy (targetting Z80 cpu), but
even that didn't work, so i patched the bootrom to skip the problematic
code (was basically only a dumb loop to slow down the emulator).
the app rom also needed a patch because when its execution starts, the
N flag isn't set as expected.

