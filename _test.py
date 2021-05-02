import time
import sys

toolbar_width = 1
counter = 9

# setup toolbar
sys.stdout.write(f"[1/{counter}]")
sys.stdout.flush()
sys.stdout.write("\b" * (toolbar_width * 4)) # return to start of line, after '['

for i in range(0,counter):
    time.sleep(0.4) # do real work here
    # update the bar
    sys.stdout.write(f"{i+1}")
    sys.stdout.flush()
    sys.stdout.write("\b" * (toolbar_width))
sys.stdout.write(f"{counter}/\n")

