#!/usr/bin/python3

import re

tmp = '''
int   add ( int a , int b ) {
  int c = a + b ;
  return c ;
}

class type2fmt ( class T ) { return lol ( 1 , 2 ) ; }

'''

'''
bool  always_false ( ) { return false ; }
class type2fmt ( class T ) { return false ; }
'''


# =========================== lexer ===========================================
def lex(s):
    yield from tmp.split()

# ==================== data classes ===========================================
class Function():
    def __init__(self, ret, name, params, body):
        self.ret, self.name, self.params, self.body = ret, name, params, body

class Param():
    __slots__ = ('type', 'name')
    def __init__(self, type, name):  self.type, self.name = type, name
    def __str__(self):               return '%s %s' % (self.type, self.name)

class Call():
    __slots__ = ('function', 'params')
    def __init__(self, function, params):  self.function, self.params = function, params
    def __str__(self):
        return '%s<%s>()::ret' % (self.function, ', '.join(map(str, self.params)))

class Assignment():
    __slots__ = ('type', 'var', 'statement')
    def __init__(self, type, var, statement):  self.type, self.var, self.statement = type, var, statement
    def __str__(self):
        if self.type in ('class', 'typename'):
            return 'using %s = %s' % (self.var, self.statement)
        else:
            return 'static constexpr %s %s = %s' % (self.type, self.var, self.statement)

def Return(type, statement):
    return Assignment(type, 'ret', statement)

class UnparsedShit():
    __slots__ = ('tokens')
    def __init__(self, tokens): self.tokens = tokens
    def __str__(self):          return ' '.join(self.tokens)


# ============================== regex n stuff ================================
def c_identifier(s):
    return re.match('[a-zA-Z_][a-zA-Z0-9_]*', s)

def c_type(s):
    return s in ('bool', 'int', 'class')

def c_function(tokens, pos):
    return c_type(tokens[pos]) and c_identifier(tokens[pos+1]) and tokens[pos+2] == '('

def c_param(tokens, pos):
    return c_type(tokens[pos]) and c_identifier(tokens[pos+1])

def c_call(tokens, pos):
    return c_identifier(tokens[pos]) and tokens[pos+1] == '('

def c_return(tokens, pos):
    return tokens[pos] == 'return'

def c_assign(tokens, pos):
    return c_type(tokens[pos]) and c_identifier(tokens[pos+1]) and tokens[pos+2] == '='

# ========================= parsing ===========================================

def p_return(function, tokens, pos):
    pos, stmt = p_statement(function, tokens, pos + 1)
    return pos, Return(function.ret, stmt)

def p_assign(tokens, pos):
    new_pos, stmt = p_statement(None, tokens, pos + 3)
    return new_pos, Assignment(tokens[pos], tokens[pos + 1], stmt)

def p_param(tokens, pos):
    return pos + 2, Param(tokens[pos], tokens[pos+1])

def p_call(tokens, pos):
    call = Call(tokens[pos], [])
    pos += 2
    while tokens[pos] != ')':
        if tokens[pos] == ',':
            pos += 1
        else:
            pos, stmt = p_statement(None, tokens, pos)
            call.params.append(stmt)
    return pos, call

def p_statement(function, tokens, pos):
    if c_return(tokens, pos):
        return p_return(function, tokens, pos)
    elif c_assign(tokens, pos):
        return p_assign(tokens, pos)
    elif c_call(tokens, pos):
        return p_call(tokens, pos)
    else:
        stmt = UnparsedShit([])
        while tokens[pos] not in (';',')',','):
            stmt.tokens.append(tokens[pos])
            pos += 1
        return pos, stmt

    raise

def p_function(tokens, pos):
    function = Function(tokens[pos], tokens[pos+1], [], [])
    pos += 3
    while True:
        if c_param(tokens, pos):
            pos, param = p_param(tokens, pos)
            function.params.append(param)

        elif tokens[pos] == ',':
            pos += 1

        elif tokens[pos] == ')':
            pos += 1
            break

    assert tokens[pos] == '{'
    pos += 1

    while True:
        if tokens[pos] == '}':
            pos += 1
            break
        else:
            pos, stmt = p_statement(function, tokens, pos)
            function.body.append(stmt)
            pos += 1

    return pos, function

def to_meta_function(function):
    body = ';\n'.join(map(str, function.body)) #to_meta_stmt(function.ret, stmt) for stmt in function[3])

    print('template <%s> struct %s { %s; };' % (
        ', '.join(map(str, function.params)),
        function.name,
        body
    ))

def to_meta_stmt(function_return_type, stmt):
    if stmt[0] == 'return':
        if function_return_type in ('class', 'typename'):
            return 'using ret = %s' % (stmt,)
        else:
            return 'static constexpr %s ret = %s' % (function_return_type, stmt)


class Parser():
    def parse(self, tokens):
        pos = 0

        while pos < len(tokens):
            if c_function(tokens, pos):
                pos, function = p_function(tokens, pos)
                to_meta_function(function)

            else:
                raise Exception(tokens[pos:])


p = Parser()
print(tmp, '\n\nbecomes\n')
p.parse(list(lex(tmp)))
