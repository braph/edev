#!/usr/bin/python3

# TODO: ifdef then undef (NCURSES_NOMACROS)

import sys
from braceexpand import braceexpand

suffix = '_CPP'
undef_ncurses_ok_addr = False

def fstr(template):
    return eval(f"f'''{template}'''")

def to_list(v):
    if not isinstance(v, list):
        return list(filter(lambda x:x, [e.strip() for e in v.split(',')]))
    return v

def mk_call(function, arguments):
    return f'{function}({", ".join(arguments)})'

def mk_args(arguments):
    return ', '.join(arguments)

def mk_function(template_args, function, arguments, body):
    if template_args:
        template_args = "template<%s> " % mk_args(template_args)
    else:
        template_args = ''

    return \
        f'{template_args}inline {function}({", ".join(arguments)}) {{ {body} }}'

def redef_(from_, to_):
    return f'''
#undef  {from_}
#define {from_} {to_}'''

def move(call, call_args):
    call_args = to_list(call_args)
    if True:
        return 'wmove(%s, y, x) == ERR ? ERR : %s(%s)' % (call_args[0], call, mk_args(call_args))
    else:
        return 'wmove(%s, y, x), %s(%s)' % (call_args[0], call, mk_args(call_args))

def mk_stdscr_call(tpl, func, args, call, call_args):
    return mk_function(tpl, func, args, "return %s(%s);" % (call, mk_args(['stdscr']+call_args)))

def mk_window_call(tpl, func, args, call, call_args):
    return mk_function(tpl, func, ['WINDOW* w']+args, "return %s(%s);" % (call, mk_args(['w']+call_args)))

def mk_mv_stdscr_call(tpl, func, args, call, call_args):
    return mk_function(tpl, func, ['int y', 'int x']+args, 'return '+move(call, ['stdscr']+call_args)+';')

def mk_mv_window_call(tpl, func, args, call, call_args):
    return mk_function(tpl, func, ['WINDOW* w', 'int y', 'int x']+args, 'return '+move(call, ['w']+call_args)+';')

def w(func, args, call, call_args):
    args = to_list(args)
    call_args = to_list(call_args)
    print('%s\n%s' % (
        mk_stdscr_call([], func, args, call, call_args),
        mk_window_call([], func, args, call, call_args)))
#    return \
#        f'{func}({args})                          {{ return {call}(stdscr, {call_args}); }}\n' \
#        f'{func}(WINDOW* w, {args})               {{ return {call}(w, {call_args}); }}\n'

def mvw(tpl, func, args, call, call_args):
    tpl = to_list(tpl)
    args = to_list(args)
    call_args = to_list(call_args)

    print('%s\n%s\n%s\n%s' % (
        mk_stdscr_call(tpl, func, args, call, call_args),
        mk_window_call(tpl, func, args, call, call_args),
        mk_mv_stdscr_call(tpl, func, args, call, call_args),
        mk_mv_window_call(tpl, func, args, call, call_args)))

def redef(from_, to_):
    for f in braceexpand(from_):
        print(redef_(f, to_))

def f(tpl, func, args, call, call_args):
    if tpl:
        tpl = "template<%s> " % tpl

    print( f'{tpl}inline {func}({args}) {{ return {call}({call_args}); }}' )

with open('curs.tpl.cpp', 'r') as fh:
    python = None
    for line in fh:
        if line == 'PYTHON:\n':
            python = "\n"
        elif line == 'NO_PYTHON:\n':
            exec(python)
            python = None
        elif python:
            python += line
        else:
            print(line, end='')

    if python:
        exec(python)

    #print(fstr(fh.read()))
    sys.exit(0)
    raise

# TODO: _slen on getstr/insstr?
mvw('template<class Chr> inline int addch_CPP',  'Chr ch',              'waddch_generic',   'ch')
mvw('                    inline int getch_CPP',  '',                    'wgetch',           '')
mvw('                    inline int getch_CPP',  'wint_t* ch',          'wget_wch',         'ch')
mvw('template<class Str> inline int addstr_CPP', 'const Str& s',        'waddnstr_generic', '_cstr(s), _slen(s)')
mvw('template<class Str> inline int addstr_CPP', 'const Str& s, int n', 'waddnstr_generic', '_cstr(s), n')
mvw('template<class Str> inline int insstr_CPP', 'const Str& s',        'winsnstr_generic',  '_cstr(s), _slen(s)')
mvw('template<class Str> inline int insstr_CPP', 'const Str& s, int n', 'winsnstr_generic',  '_cstr(s), n')
mvw('template<class Str> inline int getstr_CPP', 'Str& s',              'wgetnstr_generic',  '_cstr(s), _slen(s)')
mvw('template<class Str> inline int getstr_CPP', 'Str& s, int n',       'wgetnstr_generic',  '_cstr(s), n')
raise

# TODO: wget_wch in braceexpand
# TODO: ungetch...
for f in braceexpand('{mv,}{w,}addch'):             redef(f, 'addch_CPP')
for f in braceexpand('{mv,}{w,}getch'):             redef(f, 'getch_CPP')
for f in braceexpand('{mv,}{w,}add{n,}{w,}str'):    redef(f, 'addstr_CPP')
for f in braceexpand('{mv,}{w,}ins{n,_nw,_w}str'):  redef(f, 'insstr_CPP')
for f in braceexpand('{mv,}{w,}get{n,_nw,_w}str'):  redef(f, 'getstr_CPP')

#is_cleared, is_idlok, is_idcok, is_immedok, is_keypad, is_leaveok, is_nodelay, is_notimeout, is_pad, is_scrollok, is_subwin,
#is_syncok, wgetdelay, wgetparent, wgetscrreg 

#gen('template<>')

#    return \
#        create_function(tpl, func, args, "return %s;" % mk_call(call, ['stdscr']+call_args)) \
#        create_function(tpl, func, ['WINDOW* w']+args, % mk_call()) \
#        create_function(tpl, func, ['int y', 'int x']+args, mk_call(

#        f'{tpl}inline {func}(int y, int x, {args})            {{ return {move(f"{call}(stdscr, {call_args})")}; }}\n'   \
#        f'{tpl}inline {func}(WINDOW* w, int y, int x, {args}) {{ return {move(f"{call}(w, {call_args})")}; }}'          \

#        f'{tpl}inline {func}({args})                          {{ return {call}(stdscr, {call_args}); }}\n'              \
#        f'{tpl}inline {func}(WINDOW* w, {args})               {{ return {call}(w, {call_args}); }}\n'                   \
#        f'{tpl}inline {func}(int y, int x, {args})            {{ return {move(call, "stdscr, " + call_args)}; }}\n'   \
#        f'{tpl}inline {func}(WINDOW* w, int y, int x, {args}) {{ return {move(call, "w, " + call_args)}; }}'          \
