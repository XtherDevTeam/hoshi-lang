import os
import sys
import subprocess
import pathlib

TEST_CASES = {
    "aaa": {"type": "executable", "return_code": 3},
    "array": {"type": "executable", "return_code": 21},
    "basic_cast": {"type": "executable", "return_code": 7},
    "bbb": {"type": "executable", "return_code": 0},
    "ccc": {"type": "executable", "return_code": 139},
    "ddd": {"type": "executable", "return_code": 139},
    "demo": {"type": "library"},
    "direct-assignment": {"type": "executable", "return_code": 12},
    "foreign": {"type": "executable", "return_code": 0},
    "import": {"type": "executable", "return_code": 233},
    "interface-conv": {"type": "executable", "return_code": 0},
    "mem": {"type": "executable", "return_code": 0},
    "overload": {"type": "executable", "return_code": 9},
    "short-circuit": {"type": "executable", "return_code": 0},
    "template": {"type": "library"},
    "test": {"type": "executable", "return_code": 6},
    "test1": {"type": "library"},
    "worklist": {"type": "executable", "return_code": 78},
    "null-interface": {"type": "executable", "return_code": 0},
    "null": {"type": "executable", "return_code": 0},
    "new-expr": {"type": "executable", "return_code": 0},
    "type-info": {"type": "executable", "return_code": 0},
    "array-length": {"type": "executable", "return_code": 0},
    "va-args": {"type": "executable", "return_code": 123},
    "interface-of": {"type": "executable", "return_code": 0},
    "static-method": {"type": "executable", "return_code": 0},
}


def run_test(test_file):
    test_file = pathlib.Path(test_file)
    test_name = test_file.stem
    test_info = TEST_CASES.get(test_name, {"type": "library"})

    build_type = test_info["type"]
    output_path = "build/" + test_name
    params = ["./cmake-build-debug/hoshi_lang", str(test_file), "-o", output_path, "--build-mode", "release", "--build-type", build_type]
    
    res = subprocess.run(params)
    if res.returncode != 0:
        print(f"ERR: Compiler test failed for case: {test_name}")
        return False
    else:
        print(f"OK: Compiler test passed for case: {test_name}")

    if build_type == "executable":
        res = subprocess.run([output_path])
        expected_return_code = test_info.get("return_code")
        if expected_return_code is not None and res.returncode != expected_return_code and sys.platform != "win32": # disable code run on windows, fuck i16 return code
            print(f"ERR: Execution test failed for case: {test_name}. Expected {expected_return_code}, got {res.returncode}")
            return False
        else:
            print(f"OK: Execution test passed for case: {test_name}")
    
    return True


def get_test_files(examples: pathlib.Path = pathlib.Path("examples")):
    test_files = []
    for file in examples.iterdir():
        if file.is_file() and file.suffix == ".hoshi":
            test_files.append(file)
    return test_files


def compile_project():
    if not os.path.exists("cmake-build-debug"):
        os.makedirs("cmake-build-debug")
    if subprocess.run(["make", "cmake_debug"]).returncode != 0:
        print("ERR: CMake configuration failed")
        return False
    if subprocess.run(["make", "build_debug"]).returncode != 0:
        print("ERR: Make build failed")
        return False
    return True


if __name__ == "__main__":
    if not os.path.exists("build"):
        os.makedirs("build")
    if not os.path.exists("cmake-build-debug"):
        os.makedirs("cmake-build-debug")
    
    test_files = get_test_files()
    ok = 0
    failing = []
    
    print("Compiling project...")
    if not compile_project():
        exit(1)
    
    print("Running automatic code test...")
    for test_file in test_files:
        if test_file.stem in ['ultimate', 'ffi_array']:
            continue
        if run_test(test_file):
            ok += 1
        else:
            failing.append(test_file.stem)

    print(f"Automatic code test finished. Passed: {ok}/{len(test_files)}")
    if failing:
        print("Failed cases: " + ", ".join(failing))
        exit(1)
        
    exit(0)