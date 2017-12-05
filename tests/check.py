#!/usr/bin/env python3

import os
import subprocess
import sys
import difflib
import tempfile

from collections import namedtuple
from argparse import ArgumentParser, ArgumentError
from pathlib import Path


tests_path = Path(__file__).resolve().parent
ipath = tests_path / 'itests'

command_42sh = [str(tests_path.parent / '42sh')]
command_ref = ["bash", "--posix"]

TIMEOUT_DEFAULT = 2


def highlight(string, status=False, bold=False):
    attr = ['32' if status else '31']
    if bold:
        attr.append('1')
    return '\x1b[{}m{}\x1b[0m'.format(';'.join(attr), string)


def _test_parser():
    parser = ArgumentParser(description='Runs the 42sh test suite')
    paa = parser.add_argument

    paa('-t', '--timeout', action='store', type=int, default=TIMEOUT_DEFAULT,
        help='Define the maximum time a test can run for, in seconds')

    paa('-l', '--list', action='store_true',
        help='List all available modules')

    paa('-c', '--category', action='store', default=None,
        help='Only run the specified category')

    paa('-s', '--sanity', action='store_true',
        help='Run the test suite with sanity checks')

    return parser


def discover_suite(suite_path):
    return [(fname, fname.read_bytes()) \
            for fname in suite_path.iterdir() if fname.is_file()]


def should_run(args, cat):
    return args.category is None or cat.name == args.category


def discover_integration_tests(args):
    return  [(suite_path, discover_suite(suite_path))
             for suite_path in ipath.iterdir()
             if suite_path.is_dir() and should_run(args, suite_path)]


def run_process(conf, args, stdin):
    with tempfile.TemporaryDirectory() as tempdir:
        return subprocess.run(args, input=stdin,
                              timeout=conf.timeout, cwd=tempdir,
                              stderr=subprocess.PIPE, stdout=subprocess.PIPE)


def diffio(mine, ref):
    res = difflib.diff_bytes(difflib.unified_diff, [mine], [ref],
                             fromfile=b"42sh", tofile=b"ref")
    return '\n'.join(b.decode('unicode_escape') for b in res)


Comparison = namedtuple('Comparison', ['name', 'obj'])


def compare_processes(pa, pb):
    def diff_pair(pair):
        return diffio(*pair)

    def get_pair(attr):
        return map(lambda s: getattr(s, attr), (pa, pb))

    res = [Comparison(stream, diff_pair(get_pair(stream)))
           for stream in ('stderr', 'stdout')]
    res.append(Comparison('return code', pa.returncode != pb.returncode))
    return res


def run_test(conf, stdin):
    return compare_processes(run_process(conf, command_42sh, stdin),
                             run_process(conf, command_ref, stdin))


def format_test(conf, test_path, test_input):
    print(f'Currently running {test_path.name}...', end='')
    res = run_test(args, test_input)
    success = not any(comp.obj for comp in res)
    tag = highlight('[OK]' if success else '[KO]', success, True)
    print(f"\x1b[2K\r{tag}\t{test_path.name}")
    if not success:
        for comp in (comp for comp in res if comp.obj):
            failmsg = f'>>> mismatched {comp.name} <<<'
            print(highlight(failmsg, False, True))
            print(highlight('<' * len(failmsg), False, True))
            print(highlight(comp.obj, success, True))


if __name__ == '__main__':
    args = _test_parser().parse_args()
    tests = discover_integration_tests(args)
    if args.list:
        for suite_name in (suite.name for suite, _ in tests):
            print(suite_name)
        exit(0)
    for suite_path, suite_tests in tests:
        print(f'\t>> {highlight(suite_path.name, True, False)} <<', end='\n\n')
        for test_path, test_input in suite_tests:
            format_test(args, test_path, test_input)
