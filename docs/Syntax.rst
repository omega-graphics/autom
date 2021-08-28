=====================
Autom Language Syntax
=====================

Autom uses an expressive, object-oriented language similar to the syntax of Python.

There are 2 file types used in Autom. ( AUTOM.build and \*.autom)

==========


--------
Keywords
--------
**var**
    Defines a variable

    Usage::

        var myVar = ["foo","bar"]

**func**
    Defines a function

    Usage::

        var testVar = "Hello World"
        func myFunc(param) {
            print(msg:param)
        }

        myFunc(param:testVar)

**import**
    Imports \*.autom file with the corresenponding name.

    Usage::

        import "./autom/myModule"

**if**
    Defines the start of a conditional collection as well as a first conditional test.

    Usage::

        if(toolchain.name == "LLVM"){
            print(msg:"Using LLVM Toolchain")
        }

**elif**
    Defines of an alternative conditonal test in an existing collection.
**else**
    Defines the end of a conditional collection.

--------------------------
Builtin Variables/Objects:
--------------------------

**autom**
    An object interface for accessing internal values
    such as the cfamily toolchain or the target build system.

    *Properties:*
        ```.toolchain```
            The name of the cfamily toolchain that was selected by AUTOM.
        
        ```.c_flags```
            The default C compiler flags to use 

        ```.cxx_flags```
            The default CXX compiler flags to use.

------------------
Object Properties:
------------------

**Target Properties**
    
    ```.name```
        The name of the target used by AUTOM internal.
        NOTE: Each target declared must have a unique name.

**Compiled Target Properties**

    ```.output_name```
        The output filename of the compiled target.
        (This value is by default equal to the value set in ```.name```)
    
    ```.output_ext```
        The output filename extension of the compiled target.
        (This value is by default equal to the standard output file extension of the compiled target type with the corresponding target system.
        For example, a Shared target compiled for aarch64-darwin will have an output extension dylib)

        Default Values on each Target OS :
            
            Windows --> .exe (Executable), .lib (Static), .dll (Shared)

            Darwin  --> .a (Static), .dylib (Shared)

            Linux   --> .a (Static), .so (Shared)

    ```.include_dirs```   
        The extra directories to search for include files. (Headers)

    ```.libs```
        The extra libraries to link to the compiled target.
    
    ```.lib_dirs```
        The extra directories to search for linkable libraries.

**Target Config Properties**
    Every Target has there own



------------------
Builtin Functions:
------------------

**print(msg:any) -> Void**
    Prints a value to the console

    Usage::

        print(msg:"Hello World")
        # --> "Hello World"

        print(msg:["foo","bar"])
        # --> ["foo","bar"]

        print(msg:true)
        # --> true



**Executable(name:string,sources:string[]) -> Executable**

    Creates an Executable target.


**Shared(name:string,sources:string[]) -> Shared**

    Creates a Shared Library target.


**Static(name:string,sources:string[]) -> Static**

    Creates an Static Library target.

**SourceGroup(name:string,sources:string[]) -> SourceGroup**

    Creates a Source Group target.






