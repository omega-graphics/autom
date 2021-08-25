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
            print(param)
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
            print("Using LLVM Toolchain")
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


------------------
Builtin Functions:
------------------

**print**
    Prints a value to the console

    Usage::

        print("Hello World")
        # --> "Hello World"

        print(["foo","bar"])
        # --> ["foo","bar"]

        print(true)
        # --> true



**Executable**

    Creates an Executable



