# Install instructions

--

WARNING: These instructions are for use on MacOS. They will be updated for Windows and Linux in the future. Work in progress!

--

Follow the build instructions to compile LLVM and the LIL compiler, and then link it all together.

Once you've successfully built the LIL compiler, you need to create a folder that contains the following:

- `configure_defaults.lil` (where all the defaults live)
- `lil` (the binary of the compiler)
- `std` (a folder containing the standard lil library)

You'll also need a file called `std_lil_mac.o`, which is produced by calling `clang -c std_lil_mac.m -o std_lil_mac.o` from the command line.

Finally, you'll have to compile the metal shader library, by calling:

`xcrun -sdk macosx metal -c lil_shaders.metal -o lil_shaders.air`
`xcrun -sdk macosx metallib lil_shaders.air -o lil_shaders.metallib`

Copy the metallib file into the Contents/Resources folder of the app bundle of your game/app.

Put this folder wherever you like on your system, and then add it to your PATH, so you can use it from any directory without writing the full path to the compiler binary. There are many tutorials about this on the internet if you don't know what this means.
