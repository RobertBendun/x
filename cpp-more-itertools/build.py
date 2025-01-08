#!/usr/bin/env python3

import subprocess
import glob
import os
import pathlib
import shlex

CXX = "c++"
CXXFLAGS = ["-std=c++23", "-Wall", "-Wextra"]
BUILD_DIR = pathlib.Path("build")

def needs_rebuild(target, *sources):
    target = pathlib.Path(target)
    if target.exists():
        target_mtime = pathlib.Path(target).stat().st_mtime
        return any(target_mtime < pathlib.Path(source).stat().st_mtime for source in sources)
    return True

def cmd(*args):
    args = list(map(str, args))
    print(f"[CMD] {shlex.join(args)}", flush=True)
    subprocess.run(args, check=True)

def cxx_obj(source):
    result = str(BUILD_DIR / (source + ".o"))
    if needs_rebuild(result, source):
        cmd(CXX, *CXXFLAGS, "-c", source, "-o", result)
    return result

def cxx_exe(source, *deps):
    result = str(source)
    result, _ = os.path.splitext(result)
    if needs_rebuild(result, source, *deps):
        cmd(CXX, *CXXFLAGS, source, "-o", result, *deps)
    return result


def main():
    BUILD_DIR.mkdir(exist_ok=True)

    namespaces = []
    test_objs = []

    for test_case in glob.glob("*_test.cc"):
        name = test_case.removesuffix("_test.cc")
        namespaces.append(name)
        test_objs.append(cxx_obj(test_case))

    test_main_file = BUILD_DIR / "test.cc"

    with open(test_main_file, "w") as f:
        for namespace in namespaces:
            print(f"namespace test::{namespace} {{ void runtime(); }}", file=f)

        print("int main() {", file=f)
        for namespace in namespaces:
            print(f"  ::test::{namespace}::runtime();", file=f)
        print("}", file=f)

    cmd("./" + cxx_exe(test_main_file, *test_objs))

if __name__ == "__main__":
    main()
