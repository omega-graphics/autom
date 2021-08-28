==========
Interfaces
==========

Interfaces or Modules are files in AUTOM that can be used to define functions and variables for other build files to use.

Below are the standard interfaces that are provided with AUTOM and can be imported likewise.

----
"fs"
----


| Functions:

**fs_abspath(path:string) -> string**
		
	 Convert the path provided by the argument `string` into an absolute path

	 Usage::

		 print(msg:)

**fs_exists(path:string) -> boolean**
	
	 Tests whether the provided path exists.

**fs_glob(path:string) -> string[]**
	
	 Globs files non-recursively and returns the result.
	 

	

.. ----
.. "apple"
.. ----
..
.. ----
.. "bridge"
.. ----



