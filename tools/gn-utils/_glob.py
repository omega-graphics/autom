import argparse,glob,sys,json,os
sys.argv.pop(0)
parser = argparse.ArgumentParser()
parser.add_argument("glob_pattern",type=str)
args = parser.parse_args(args=sys.argv)

res = glob.glob(pathname=args.glob_pattern)
sys.stdout.write(json.dumps(res))