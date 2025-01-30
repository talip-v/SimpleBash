import os
import subprocess
import sys
from colors import Color

test_presets = "test_presets.txt"
valgrind_cmd = ["valgrind", "--leak-check=full"]

def run(my, pwd):
    os.chdir(pwd)
    if not os.path.isfile(test_presets):
        print("File with arg presets 'test_presets.txt' not found")
        sys.exit(1)

    if not os.path.isfile(my):
        print(f"Пожалуйста, положи свой {Color.BOLD}{my[2:]}{Color.NC} в {Color.BOLD}{pwd}{Color.NC}")
        sys.exit(1)

    with open(test_presets) as tests:
        total_tests = 0
        passed_tests = 0
        for test in tests:
            if not test.rstrip():
                continue
            total_tests += 1
            args = test.rstrip().split(' ')
            with open("out", "w") as out:
                subprocess.run(valgrind_cmd + [my] + args, stderr=out, stdout=subprocess.DEVNULL)

            res = True
            with open("out", "r") as out:
                for line in out:
                    if "ERROR SUMMARY" in line and "ERROR SUMMARY: 0" not in line: 
                        print(f"{Color.RED}FAILED with args: ", end="")
                        print(" ".join(args))
                        print(f"{Color.NC}")
                    elif "ERROR SUMMARY" in line:
                        passed_tests += 1

            text_res = f"{Color.GREEN}PASS{Color.NC}" if res else f"{Color.RED}FAIL{Color.NC}"
            print(f"Test: {total_tests}. {text_res}")
            os.remove("out")
        
        failed_color = f"{Color.RED}" if total_tests - passed_tests > 0 else f"{Color.NC}"
        print(f"{Color.GREEN}Passed: {passed_tests}{Color.NC} | {failed_color}Failed: {total_tests - passed_tests}{Color.NC} | Total: {total_tests}")