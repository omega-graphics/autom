# AUTOM 
## (Automate,Automatic,etc..)

An open-source native code build system generator used by the OmegaG Suite. It can generate to a few build systems including CMake, and GN.

NOTE: Some build targets declared are only supported by GN.

## Setup

Prerequisites:

- Python (3.4 or greater)

- (For Windows Users Only) Visual Studio 16 2019

#### Unix

```sh
git clone https://github.com/omega-graphics/autom ./autom
cd ./autom
python3 ./init.py

# Add ./bin to PATH
export PATH=$PATH:~/autom/bin
```

#### Windows

```bat
git clone https://github.com/omega-graphics/autom ./autom
cd autom
py -3 init.py

rem Add ./bin to PATH

set PATH="%PATH%;C:\Users\example-user\autom\bin"
```



## License 

BSD 3-Clause

See [LICENSE](LICENSE)