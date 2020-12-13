#!/usr/bin/python3
import sys, string
from braceexpand import braceexpand

'''
(Almost) every function in ncurses is implemented as a function and defined as a macro:
  - extern NCURSES_EXPORT(int)  addstr (const char *);	   /* generated */
  - #define addstr(str)         waddnstr(stdscr,(str),-1)

We cannot provide overloads for char[] by defining `template<size_t N> addstr(char[N])` since the compiler
would pick the addstr(const char*) version.

- Automated generation of function overloads wouldn't make sense, since we want to take advatage of the
  array size: addstr(array) will translate to addnstr(array, N)

However, we can provide template<class T> addstr(T s) { addstr(s.c_str()); }
'''

# TODO: generate w* functions, too!
# TODO: ifdef then undef... (NCURSES_NOMACROS)
# TODO: printw: to string everything
# TODO: wbkgdset, wbg ...
# TODO: default arguments!

prefix = 'NC_'
undef_ncurses_ok_addr = False
void = "void"
WINDOW = "WINDOW*"
funcs = []
define = {}

def type_to_str(typ):
    return {int:'int', bool:'bool'}.get(typ, typ)

def mk_func_signature(template_args, ret, name, args, const=False, noexcept=False):
    return "%sinline %s (%s)(%s)%s%s" % (
        'template<%s> ' % template_args if template_args else '',
        type_to_str(ret), name, args, 
        ' const' if const else '',
        ' noexcept' if noexcept else '')

class Func():
    # TODO: make this configuratble
    WMOVE = 'wmove(%s, y, x) == ERR ? ERR : '
    WMOVE = 'wmove(%s, y, x), '

    def __init__(self, ret, name, args, wrapped_func, wrapped_args, const=False, mv=False, template=None):
        self.ret = ret
        self.name = name
        self.args = args
        self.wrapped_func = wrapped_func
        self.wrapped_args = wrapped_args
        self.const = const
        self.mv = mv
        self.template = template

    def make_function(self, prefix, win_arg='win', wrapped_win_arg='win', mv=False):
        args = ('int y, int x, ' + self.args).strip(', ') if mv else self.args
        if win_arg:
            args = ('const WINDOW* ' if self.const else 'WINDOW* ') + win_arg + ', ' + args
            args = args.strip(', ')

        return "%s { %s%s%s; }" % (
            mk_func_signature(self.template, self.ret, prefix+self.name, args),
            "return " if self.ret != void else "",
            Func.WMOVE % wrapped_win_arg if mv else '',
            self.wrapped_call(wrapped_win_arg))
        
    def make_method(self, prefix, wrapped_win_arg='win', mv=False):
        args = ('int y, int x, ' + self.args).strip(', ') if mv else self.args

        return "%s { %s%s%s; }" % (
            mk_func_signature(self.template, self.ret, prefix+self.name, args, const=self.const, noexcept=True),
            "return " if self.ret != void else "",
            Func.WMOVE % wrapped_win_arg if mv else '',
            self.wrapped_call(wrapped_win_arg))

    def wrapped_call(self, wrapped_win_arg):
        return '%s(%s%s%s)' % (
            self.wrapped_func,
            wrapped_win_arg,
            ', ' if self.wrapped_args else '',
            self.wrapped_args)

def a(*args, **kw):  funcs.append(Func(*args, **kw))
def mv(*args, **kw): funcs.append(Func(*args, **kw, mv=True))

# curs_move(3X):
a(int, "move",     "int y, int x",  "wmove", "y, x")

# curs_clear(3X):
a(int, "erase",    "", "werase",    "")
a(int, "clear",    "", "wclear",    "")
a(int, "clrtobot", "", "wclrtobot", "") # TODO: also add mv?
a(int, "clrtoeol", "", "wclrtoeol", "") # TODO: also add mv?

# curs_attr(3X):
a(int, "standend", "",              "wstandend",  "")
a(int, "standout", "",              "wstandout",  "")
a(int, "attron",   "Attr attrs",    "wattron",    "static_cast<int>(attrs)", template='class Attr')
a(int, "attroff",  "Attr attrs",    "wattroff",   "static_cast<int>(attrs)", template='class Attr')
a(int, "attrset",  "Attr attrs",    "wattrset",   "static_cast<int>(attrs)", template='class Attr')
a(int, "attr_on",  "attr_t attrs, void* opts = NULL",   "wattr_on",  "attrs, opts")
a(int, "attr_off", "attr_t attrs, void* opts = NULL",   "wattr_off", "attrs, opts")
a(int, "attr_set", "attr_t attrs, NCURSES_PAIRS_T pair, void* opts = NULL",          "wattr_set",  "attrs, pair, opts")
a(int, "attr_get", "attr_t* attrs, NCURSES_PAIRS_T* pair = NULL, void* opts = NULL", "wattr_get",  "attrs, pair, opts", const=True)
a(int, "chgat",    "int n, attr_t attr, short pair, const void *opts = NULL",        "wchgat",     "n, attr, pair, opts")
a(int, "color_set", "short pair, void* opts = NULL",                                 "wcolor_set", "pair, opts")

# curs_opaque(3X):
# TODO: ifdef NCURSES_EXT_FUNCS
for f in braceexpand("is_{cleared,idcok,idlok,immedok,keypad,leaveok,nodelay,notimeout,pad,scrollok,subwin,syncok}"):
    a(bool, f, "", f, "", const=True)

a(int,    "wgetdelay",  "", "wgetdelay",  "", const=True)
a(WINDOW, "wgetparent", "", "wgetparent", "", const=True)
a(int,    "wgetscrreg", "int* top, int* bottom", "wgetscrreg", "top, bottom", const=True)

# curs_legacy(3X):
a(int, "getattrs", "", "getattrs", "", const=True)

for f in braceexpand("get{cur,beg,max,par}{x,y}"):
    a(int, f, "", f, "", const=True)

# curs_getyx(3X):
for f in braceexpand("get{,beg,max,par}yx"):
    a(void, f, "int& y, int& x", f, "y, x", const=True)

# OUTPUT functions
mv(int, "addch",  "chtype ch",           "waddch",            "ch")
mv(int, "addch",  "const cchar_t* ch",   "wadd_wch",          "ch")
mv(int, "addstr", "const Str& s",        "waddnstr_generic",  "cstr(s), len(s)",    template='class Str')
mv(int, "addstr", "const Str& s, int n", "waddnstr_generic",  "cstr(s), n",         template='class Str')
mv(int, "insch",  "chtype ch",           "winsch",            "ch")
mv(int, "insch",  "const cchar_t* ch",   "wins_wch",          "ch")
mv(int, "insstr", "const Str& s",        "winsnstr_generic",  "cstr(s), len(s)",    template='class Str')
mv(int, "insstr", "const Str& s, int n", "winsnstr_generic",  "cstr(s), n",         template='class Str')
mv(int, "delch",  "",                    "wdelch",            "")
# INPUT functions
mv(int, "getch",  "",                    "wgetch",            "")
mv(int, "getch",  "wint_t* ch",          "wget_wch",          "ch")
mv(int, "getstr", "Str& s",              "wgetnstr_generic",  "cstr(s), in_len(s)", template='class Str')
mv(int, "getstr", "Str& s, int n",       "wgetnstr_generic",  "cstr(s), n",         template='class Str')
mv(int, "instr",  "Str& s",              "winnstr_generic",   "cstr(s), in_len(s)", template='class Str')
mv(int, "instr",  "Str& s, int n",       "winnstr_generic",   "cstr(s), n",         template='class Str')
mv(int, "inch",   "cchar_t* ch",         "win_wch",           "ch")
mv("chtype", "inch", "",                 "winch",             "")

# Functions that are not bound to a window:
# ungetch(int), has_key(int)

CURSES_FUNCTIONS = ''
CURSES_WINDOW_METHODS = ''

for f in funcs:
    CURSES_FUNCTIONS += '\n'+f.make_function(prefix)
    CURSES_FUNCTIONS += '\n'+f.make_function(prefix, None, 'stdscr')
    if f.mv:
        CURSES_FUNCTIONS += '\n'+f.make_function(prefix, mv=True)
        CURSES_FUNCTIONS += '\n'+f.make_function(prefix, None, 'stdscr', mv=True)

    CURSES_WINDOW_METHODS += '\n'+f.make_method('', 'win')
    CURSES_WINDOW_METHODS += '\n'+f.make_method(prefix, 'win')
    if f.mv:
        CURSES_WINDOW_METHODS += '\n'+f.make_method('', 'win', mv=True)
        CURSES_WINDOW_METHODS += '\n'+f.make_method(prefix, 'win', mv=True)

for f in funcs:
    define[f.name] = prefix+f.name

for f in braceexpand('{mv,}{w,}addch'):             define[f] = prefix+'addch'
for f in braceexpand('{mv,}{w,}add{n,}{w,}str'):    define[f] = prefix+'addstr'
for f in braceexpand('{mv,}{w,}get{,_w}ch'):        define[f] = prefix+'getch'
for f in braceexpand('{mv,}{w,}get{n,_nw,_w}str'):  define[f] = prefix+'getstr'
for f in braceexpand('{mv,}{w,}ins{,_w}ch'):        define[f] = prefix+'insch'
for f in braceexpand('{mv,}{w,}ins{n,_nw,_w}str'):  define[f] = prefix+'insstr'
for f in braceexpand('{mv,}{w,}in{,_w}ch'):         define[f] = prefix+'inch'
for f in braceexpand('{mv,}{w,}in{ch,}str'):        define[f] = prefix+'instr'

define.pop('clear',None)
define.pop('erase',None)
define.pop('move',None)

REDEFS = '''
#undef clear
#undef erase
#undef move
'''
for from_, to_ in define.items():
    REDEFS += f'''
#undef  {from_}
#define {from_} {to_}'''


with open('curs.tpl.cpp', 'r') as fh:
    print('// This file was generated by:', sys.argv)
    print(string.Template(fh.read()).substitute(**globals()))

