#!/bin/env python3

import subprocess
import tempfile
import sys
from pathlib import Path

def run_process(args, stdin):
    with tempfile.TemporaryDirectory() as tempdir:
        return subprocess.run(args, input=stdin,
                              timeout=1, cwd=tempdir,
                              stderr=subprocess.PIPE, stdout=subprocess.PIPE)



b = sys.stdin.buffer.read()
command_san = ['valgrind', '-q', '--error-exitcode=100',
               '--leak-check=full', '--show-leak-kinds=all', '--']

path_42 = Path("./42sh").resolve()

for i in range(len(b)):
    rb = b[:i]
    print(f'>> {i} {rb}')
    print(run_process(command_san + [str(path_42)], rb).stderr)
