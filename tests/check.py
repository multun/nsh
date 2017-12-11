#!/usr/bin/env python3

import subprocess
import sys
import difflib
import tempfile
import json

from collections import namedtuple
from argparse import ArgumentParser
from pathlib import Path


tests_path = Path(__file__).resolve().parent
ipath = tests_path / 'itests'

command_ref = ["bash", "--posix"]
command_san = ['valgrind', '-q', '--error-exitcode=100',
               '--leak-check=full', '--show-leak-kinds=all', '--']


TIMEOUT_DEFAULT = 2


_isatty = sys.stdout.isatty()


def highlight(string, status=False, bold=False):
    if not _isatty:
        return string
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

    paa('--check', action='store_true',
        help=f'Run the test suite against {" ".join(command_ref)}')

    paa('-q', '--quiet', action='store_true',
        help='Don\'t display diffs on failed tests')

    paa('target', nargs='?', default=str(tests_path.parent / '42sh'),
        help='hosts to run tests on')

    return parser


class TestResult():
    @classmethod
    def from_dict(self, d):
        return TestResult(d['stdout'].encode('utf-8'),
                          d['stderr'].encode('utf-8'),
                          str(d['retcode']))

    @classmethod
    def from_proc(self, d):
        return TestResult(d.stdout, d.stderr, str(d.returncode))

    def __init__(self, stdout, stderr, retcode):
        self.stdout = stdout
        self.stderr = stderr
        self.retcode = retcode


class Test():
    def __init__(self, path):
        self.path = path
        self.name = path.stem
        test_dict = json.loads(path.read_bytes().decode('utf-8'))
        self.description = test_dict['desc']
        self.stdin = test_dict['stdin']
        self.expected = TestResult.from_dict(test_dict)


def discover_suite(suite_path):
    return [Test(tpath)
            for tpath in suite_path.glob('*.json') if tpath.is_file()]


def should_run(args, cat):
    return args.category is None or cat.name == args.category


def discover_integration_tests(args):
    return [(suite_path, discover_suite(suite_path))
            for suite_path in ipath.iterdir()
            if suite_path.is_dir() and should_run(args, suite_path)]


def run_process(conf, args, stdin):
    try:
        with tempfile.TemporaryDirectory() as tempdir:
            proc = subprocess.run(args, input=stdin,
                                  timeout=conf.timeout, cwd=tempdir,
                                  stderr=subprocess.PIPE,
                                  stdout=subprocess.PIPE)
            return TestResult.from_proc(proc)
    except subprocess.TimeoutExpired:
        return None


def diffio(mine, ref):
    res = difflib.diff_bytes(difflib.unified_diff, [mine], [ref],
                             fromfile=b"42sh", tofile=b"ref")
    return '\n'.join(b.decode('unicode_escape') for b in res)


Comparison = namedtuple('Comparison', ['name', 'obj'])


def compare_results(pa, pb):
    if pa is None:
        return [Comparison('timeout', 'timeout expired')]

    def diff_pair(pair):
        return diffio(*pair)

    def get_pair(attr):
        return map(lambda s: getattr(s, attr), (pa, pb))

    def compare_stream(stream):
        return Comparison(stream, diff_pair(get_pair(stream)))

    res = [compare_stream('stdout')]
    stderr_diff = bool(pa.stderr) != bool(pb.stderr)

    def decs(p):
        return 'wrote' if p.stderr else 'did not write'

    stderr_msg = (f'tested program {decs(pa)} on stderr, '
                  f'whereas reference {decs(pb)}') if stderr_diff else ''

    res.append(Comparison('stderr', stderr_msg))
    retcode_res = '' if pa.retcode == pb.retcode \
                  else f'{pa.retcode} != {pb.retcode}'
    res.append(Comparison('return code', retcode_res))
    return res


def gen_command(conf):
    cmd = command_ref if conf.check else [conf.target]
    if conf.sanity:
        return command_san + cmd
    return cmd


def run_test(conf, test):
    stdin = test.stdin.encode('utf-8')
    cmd = gen_command(conf)
    return compare_results(run_process(conf, cmd, stdin), test.expected)


def format_test(conf, test):
    print(f'Currently running {test.name}...', end='')
    res = run_test(args, test)
    success = not any(comp.obj for comp in res)
    prefix = "\x1b[2K\r" if _isatty else '\n'
    tag = highlight('[OK]' if success else '[KO]', success, True)
    print(f"{prefix}{tag}\t{test.name}")
    if not success and not conf.quiet:
        print(f"\n\t### {test.description}\n")
        for comp in (comp for comp in res if comp.obj):
            failmsg = f'>>> mismatched {comp.name} <<<'
            print(highlight(failmsg, False, True))
            print(highlight('<' * len(failmsg), False, True))
            print(highlight(comp.obj, success, True))


if __name__ == '__main__':
    args = _test_parser().parse_args()
    args.target = str(Path(args.target).resolve())
    tests = discover_integration_tests(args)
    if args.list:
        for suite_name in (suite.name for suite, _ in tests):
            print(suite_name)
        exit(0)
    for suite_path, suite_tests in tests:
        print(f'\n\t>> {highlight(suite_path.name, True, False)} <<', end='\n\n')
        for test in suite_tests:
            format_test(args, test)
