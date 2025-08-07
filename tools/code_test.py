import os
import subprocess
import pathlib

def run_test(test_file):
    test_file = pathlib.Path(test_file)
    params = ["./cmake-build-debug/hoshi_lang", str(test_file), "-o", "build/" + test_file.stem, "--build-mode", "debug", "--build-type", "library"]
    res = subprocess.run(params)
    if res.returncode != 0:
        print("ERR: Compiler test failed for case: " + test_file.stem)
        return False
    else:
        print("OK: Compiler test passed for case: " + test_file.stem)
        return True
        
def get_test_files(examples: pathlib.Path = pathlib.Path("examples")):
    test_files = []
    for file in examples.iterdir():
        if file.is_file() and file.suffix == ".hoshi":
            test_files.append(file)
    return test_files

def compile():
    os.chdir("cmake-build-debug")
    if subprocess.run(["cmake", ".."]).returncode:
        print("ERR: CMake build failed")
        return False
    if subprocess.run(["make", "-j16"]).returncode:
        print("ERR: Make build failed")
        return False
    os.chdir("..")
    return True
    

if __name__ == "__main__":
    test_files = get_test_files()
    ok = 0
    failing = []
    print("Compiling...")
    compile()
    print("Running automatic code test...")
    for test_file in test_files:
        if run_test(test_file): ok += 1
        else: failing.append(test_file.stem)
        
    print("Automatic code test finished. Passed: " + str(ok) + "/" + str(len(test_files)))
    if failing:
        print("Failed cases: " + ", ".join(failing))
    