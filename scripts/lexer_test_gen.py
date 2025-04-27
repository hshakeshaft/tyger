#!/usr/bin/env python3
import argparse
import copy
import enum as E
from dataclasses import dataclass
from typing import List, Dict
import os
import pathlib

class TokenKind(E.Enum):
    """Specifies the kind of Token
    """
    EOF = E.auto()
    ILLEGAL = E.auto()

    # "punctuation"
    LPAREN = E.auto()
    RPAREN = E.auto()
    LBRACE = E.auto()
    RBRACE = E.auto()
    LBRACKET = E.auto()
    RBRACKET = E.auto()
    SEMICOLON = E.auto()

    # Operators
    # ===== Arithmetic =====
    PLUS = E.auto()
    MINUS = E.auto()
    ASTERISK = E.auto()
    SLASH = E.auto()
    # ===== Assignment =====
    ASSIGN = E.auto()
    # ===== Logical =====
    BANG = E.auto()
    LT = E.auto()
    GT = E.auto()
    LTE = E.auto()
    GTE = E.auto()
    EQ = E.auto()
    NOT_EQ = E.auto()

    # Idents And Expressions
    INTEGER = E.auto()
    STRING = E.auto()
    IDENT = E.auto()

    # Keywords and Builtins
    # ===== Keywords =====
    VAR = E.auto()
    # ===== Builtins =====
    PRINTLN = E.auto()

    def __str__(self) -> str:
        return self.name

@dataclass
class Location:
    pos: int
    line: int
    col: int


@dataclass
class Token:
    location: Location
    kind: TokenKind = TokenKind.ILLEGAL
    literal: str = ""

    def __str__(self) -> str:
        """NOTE(HS): prints in C++ struct layout
        """
        return "Token{{ Location{{ {}, {}, {} }}, String_View{{ {}, {} }}, {}  }}".format(
            self.location.pos, self.location.col, self.location.line,
            self.location.pos, len(self.literal),
            f"TK_{self.kind}"
        )


PUNCTUATION_TO_TOKEN_KIND: Dict[str, TokenKind] = {
    '(': TokenKind.LPAREN,
    ')': TokenKind.RPAREN,
    '{': TokenKind.LBRACE,
    '}': TokenKind.RBRACE,
    '[': TokenKind.LBRACKET,
    ']': TokenKind.RBRACKET,
    ';': TokenKind.SEMICOLON,
}

ARITHMETIC_OP_TO_TOKEN_KIND: Dict[str, TokenKind] = {
    '+': TokenKind.PLUS,
    '-': TokenKind.MINUS,
    '*': TokenKind.ASTERISK,
    '/': TokenKind.SLASH,
}


KEYWORD_OR_BUILTIN: Dict[str, TokenKind] = {
    'var': TokenKind.VAR,
    'println': TokenKind.PRINTLN,
}


class Lexer:
    def __init__(self, prog: str) -> None:
        self.prog: str = prog
        self.ch: str = "\0"
        self.pos: Location = Location(0, 0, 0)
        self.read_pos: int = 0
        self.read_char()

    # TODO(HS): implement proper position increment, tracking col/row
    def read_char(self):
        if self.read_pos >= len(self.prog):
            self.ch = "\0"
        else:
            self.ch = self.prog[self.read_pos]
        self.pos.pos = self.read_pos
        self.read_pos += 1
        return

    def peek_char(self) -> str:
        if self.read_pos >= len(self.prog):
            return '\0'
        else:
            return self.prog[self.read_pos]

    def skip_whitespace(self):
        while self.ch.isspace():
            self.read_char()
        return

    def next_token(self) -> Token:
        self.skip_whitespace()

        token = Token(copy.copy(self.pos), literal=self.ch)
        match self.ch:
            case "\0":
                token.kind = TokenKind.EOF
                token.literal = "\0"

            case '(' | ')' | '{' | '}' | '[' | ']' | ';':
                token.kind = PUNCTUATION_TO_TOKEN_KIND[self.ch]

            case '+' | '-' | '*' | '/':
                token.kind = ARITHMETIC_OP_TO_TOKEN_KIND[self.ch]

            case '!':
                if self.peek_char() == '=':
                    pos = self.pos.pos
                    self.read_char()
                    token.kind = TokenKind.NOT_EQ
                    token.literal = self.prog[pos:self.read_pos]
                else:
                    token.kind = TokenKind.BANG

            case '=':
                 if self.peek_char() == '=':
                    pos = self.pos.pos
                    self.read_char()
                    token.kind = TokenKind.EQ
                    token.literal = self.prog[pos:self.read_pos]
                 else:
                    token.kind = TokenKind.ASSIGN

            case '<' | '>':
                ch = self.ch
                if self.peek_char() == '=':
                    self.read_char()
                    token.kind = TokenKind.LTE if ch == '<' else TokenKind.GTE
                    token.literal = ch + self.ch
                else:
                    token.kind = TokenKind.LT if ch == '<' else TokenKind.GT

            case _:
                # TODO(HS): check int vs floats later
                if self.ch.isdigit():
                    token.kind = TokenKind.INTEGER
                    token.literal = self.read_number()
                    return token
                elif self.ch == '\"':
                    token.literal = self.read_string()
                    token.kind = TokenKind.STRING
                    return token
                elif self.ch.isalpha():
                    token.literal = self.read_ident()
                    kind = KEYWORD_OR_BUILTIN.get(token.literal)
                    token.kind = kind if kind is not None else TokenKind.IDENT
                    return token
                else:
                    token.kind = TokenKind.ILLEGAL

        self.read_char()

        return token

    def read_number(self) -> str:
        pos = self.pos.pos
        while self.ch.isdigit():
            self.read_char()
        num_len = self.pos.pos - pos
        return self.prog[pos:pos + num_len]

    def read_string(self) -> str:
        """NOTE(HS): this consumes the quotation marks
        """
        self.read_char()
        pos = self.pos.pos
        while self.ch != '\"' and self.ch != '\0':
            if self.ch == '\\' and self.peek_char() == '\"':
                self.read_char()
            self.read_char()
        slen = self.pos.pos - pos
        self.read_char()
        return self.prog[pos:pos + slen]

    def read_ident(self) -> str:
        pos = self.pos.pos
        while self.ch.isalnum() and not self.ch.isspace() and self.ch != '\0':
            self.read_char()
        slen = self.pos.pos - pos
        return self.prog[pos:pos + slen]


class ExportType(E.Enum):
    TEST_CASES = "testCase"
    DEFS = "defs"
    SOURCE = "source"

    def __str__(self) -> str:
        return self.value


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog="tyger_test_gen",
        description="Script for generating lexer test cases, and some other useful things"
    )

    parser.add_argument(
        "-f", "--file",
        help="The file to read, used to generate cases",
        type=str, action="store"
    )

    parser.add_argument(
        "-t", "--output-type",
        help="What kind of data to output",
        type=ExportType, choices=list(ExportType), action="store"
    )

    args = parser.parse_args()

    file_content = None
    assert os.path.exists(pathlib.Path(args.file))
    with open(args.file, "r") as f:
        file_content = f.read()

    lexer = Lexer(file_content)

    match args.output_type:
        case ExportType.TEST_CASES:
            tokens: List[Token] = []
            t = lexer.next_token()
            tokens.append(t)

            while t.kind != TokenKind.EOF:
                t = lexer.next_token()
                tokens.append(t)
                
            for t in tokens:
                print(f"{t},")

        case ExportType.DEFS:
            for tk in list(TokenKind):
                print(f"X({tk}) \\")

        case ExportType.SOURCE:
            print(file_content)
