This subpage is to keep from cluttering the main C++ page with my rantings. Eventually, some or all of this content may end up on that page.

It is possible to compile C++ code on the ARM platform with the GNU toolchain. The generated code is remarkably compact, considering the reputation that C++ has for being bloated. One must eschew certain features such as exceptions and RTTI, lest one incur the bloat of the libraries which would be linked in to support those features. A good series of introductory articles can be found here as HTML and here as a PDF.

Any libraries used (iostream, etc.) also increase code size and memory usage. That is no different, however, from using printf in C.

The only other source of bloat that I've found when reviewing the generated assembly language is that constructors and destructors generate multiple copies of themselves. I get around that by defining my classes with init and deinit member functions which are called by the constructors and destructors, respectively. Multiple copies of the constructors and destructors are still generated, but they only contain calls to those member functions. Other than that, the generated code is pretty close to what one would get when doing object-oriented things in C.

Initially, this page exists as a place for me to post files for Liam's viewing. Any files presented here are mine. They are specific to a particular project which is not publicly available. There will, in many cases, be code or constructs in them which are of no use to any other project. They are presented in the hope the ideas contained therein will help Liam in the development of a C++ environment for the Make Controller platform. They are not necessarily ready for prime time.

## Startup Code ##

I actually went about uploading a makefile and some startup code, but, really, the startup code has only one thing in it special for C++, so I stopped. That is that it calls libc\_init\_array() before starting the main code. That is what ensures that static constructors get called. If one wants to, one could also call libc\_fini\_array() when shutting down to ensure that the static destructors get called, but I haven't yet found that to be necessary. If there is hardware that must be operated in a certain way (moving a servo to a safe position, for instance) when resetting the board, then there may be a need for it. That call would go in the reset code.

Actually, that isn't even peculiar to C++. GCC also has the constructor and destructor attributes which can be applied to regular C functions, and would also require the same measures. I use the constructor attribute on the function which initializes my malloc implementation (which is written in otherwise plain C). Otherwise, I'd have to call malloc\_init in my application, which, admittedly isn't that much of an imposition, but it works quite well this way. If nothing else, it illustrates the point I'm trying to make.

## Makefile ##

The makefile adds -fno-rtti and -fno-exceptions to CXXFLAGS.

## Support functions ##

There are a few functions that have to be defined. I put them in my libc. I don't use the standard libc. If you do, you may want to put them somewhere else. I put them in a library so that any unused functions (if a particular firmware doesn't use new or delete, for instance) don't bloat the resulting binary.

cxa\_pure\_virtual() has to be defined to keep from linking in code that throws exceptions and uses I/O (ostream, stdio, etc.). It is called when a pure virtual function is instantiated. In practice, that is usually (if not always) caught during compilation. The function could contain nothing (assuming it will never be called) or it could print a diagnostic message (if that facility is available) before resetting the board.

The new and delete operators must be defined in a manner similar to how they are in the above-cited series of articles. Again, this is to avoid linking in exception code. Mine are as follows.

```
void *
operator
new(
        size_t  size
) throw() {
        return(malloc(size));
}
```

```
void
operator
delete(
        void    *p
) throw() {
        free(p);
}
```

The linker script has to know about certain sections. Specifically .preinit\_array, .init\_array, and .fini\_array. The one currently used with the MC might already know about them. I've attached a copy of mine to this page.

## Denoument ##

Nothing here isn't covered by the cited articles, except maybe cxa\_pure\_virtual(). Once everything's squared away, one just starts writing C++ code and it just works. Statically allocated objects work. Objects allocated on the stack work. Objects instantiated with new and destroyed with delete work. Just don't try to use exceptions or RTTI.