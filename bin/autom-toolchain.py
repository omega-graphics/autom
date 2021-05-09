import os,sys,argparse,json 

parser = argparse.ArgumentParser()
parser.add_argument("--toolchain",choices=["msvc","llvm","gcc","vulkan","spirv"])
parser.add_argument("--export-toolchain-file",type=str)
