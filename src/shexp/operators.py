#!/usr/bin/env python3

'''
This script generates the arithmetic operators X-macro
'''


# these token can't be passed as function arguments
IGNORE_TOKEN_TYPES = {
    "NOP",
    "GROUP"
}


_TOKENS = """
exclam PREFIX !
different INFIX !=
mod INFIX %
mod_equal ASSIGN_OP %=
and INFIX &
and_and INFIX &&
and_equal ASSIGN_OP &=
lparen GROUP (
rparen NOP )
times INFIX *
times_equal ASSIGN_OP *=
plus INFIX_PREFIX +
plus_plus PREFIX_POSTFIX ++
plus_equal ASSIGN_OP +=
minus INFIX_PREFIX -
minus_minus PREFIX_POSTFIX --
minus_equal ASSIGN_OP -=
div INFIX /
div_equal ASSIGN_OP /=
colon NOP :
inferior INFIX <
lshift INFIX <<
lshift_equal ASSIGN_OP <<=
inferior_equal INFIX <=
equal ASSIGN =
equal_equal INFIX ==
superior INFIX >
superior_equal INFIX >=
rshift INFIX >>
rshift_equal ASSIGN_OP >>=
question TERNARY ?
xor INFIX ^
xor_equal ASSIGN_OP ^=
or INFIX |
or_equal ASSIGN_OP |=
or_or INFIX ||
tilde PREFIX ~
"""


X_ATTRIBUTES = (
    'pre_prio',
    'post_prio',
    'name',
    'token_class',
    'token_value',
    'value',
)

class Token():
    __slots__ = (
        'pre_prio',
        'post_prio',
        'name',
        'raw_token_class',
        'raw_value',
    )

    def __init__(self, name, raw_token_class, value):
        self.raw_token_class = raw_token_class
        self.pre_prio = None
        self.post_prio = None
        self.name = name
        self.raw_value = value

    @property
    def token_value(self):
        if self.raw_token_class in IGNORE_TOKEN_TYPES:
            return "PLACEHOLDER"
        return self.raw_value

    @property
    def token_class(self):
        return 'TOKEN_OPERATOR_' + self.raw_token_class

    @property
    def value(self):
        return ", ".join(f"'{c}'" for c in self.raw_value) + ", 0"

    def ready(self):
        return all(attr is not None for attr in self.__slots__)


def read_tokens(tokens):
    for line in tokens:
        if not line:
            continue
        name, token_class, value = line.split()
        yield name, Token(name, token_class, value)


tokens = dict(read_tokens(_TOKENS.splitlines()))


PRIO_POST_TAG = "-post"
PRIO_PRE_TAG = "-pre"

_PRIORITIES = '''
rparen colon
plus_plus-post minus_minus-post lparen
plus_plus-pre minus_minus-pre plus-pre minus-pre exclam tilde
times div mod
plus-post minus-post
lshift rshift
inferior inferior_equal superior superior_equal
equal_equal different
and
xor
or
and_and
or_or
question
equal plus_equal minus_equal times_equal div_equal mod_equal lshift_equal rshift_equal and_equal xor_equal or_equal
'''

priority_lines = _PRIORITIES.splitlines()
for line_i, line in enumerate(priority_lines):
    if not line:
        continue

    prio = str((len(priority_lines) - line_i) * 10)

    line_tokens = line.split()
    for token_name in line_tokens:
        if token_name.endswith(PRIO_POST_TAG):
            token_name = token_name[:-len(PRIO_POST_TAG)]
            tokens[token_name].post_prio = prio
        elif token_name.endswith(PRIO_PRE_TAG):
            token_name = token_name[:-len(PRIO_PRE_TAG)]
            tokens[token_name].pre_prio = prio
        else:
            token = tokens[token_name]
            token.pre_prio = token.post_prio = prio

assert(all(token.ready() for token in tokens.values()))

# sort tokens to enable easy binary search
tokens_list = list(tokens.values())
tokens_list.sort(key=lambda token: token.raw_value)

print("/* Don't modify this file directly ! run operators.py */")
print(f"/* X({', '.join(X_ATTRIBUTES)}) */")

for token in tokens_list:
    print(f'X({", ".join(getattr(token, attr) for attr in X_ATTRIBUTES)})')
