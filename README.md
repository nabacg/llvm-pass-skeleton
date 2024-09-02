# llvm-pass-skeleton

A completely useless LLVM pass.
It's for LLVM 17.

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:
    uncomment particular pass in [skeleton/Skeleton.cpp](skeleton/Skeleton.cpp) you want to run


    $ clang -fpass-plugin=`echo build/skeleton/SkeletonPass.*` tester.c


Emit LLVM IR:
    with and without skeleton pass

    $ clang -emit-llvm -S -o - ../example.c```
    $ clang -emit-llvm -S -o -  -fpass-plugin=`echo build/skeleton/SkeletonPass.*`   ../example.c
