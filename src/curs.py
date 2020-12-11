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

# TODO: generate functions also for ... eerm w.. you now? not just ...
# TODO: ifdef then undef... (NCURSES_NOMACROS)
# TODO: inch, ungetch...
# TODO: printw: to string everything
# TODO: getattrs?
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
    WMOVE        = 'wmove(%s, y, x) == ERR ? ERR : '
    WMOVE_NO_CHK = 'wmove(%s, y, x), '

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

a(int,    "wgetdelay",  "", "wgetdelay",  "", const=True)
a(WINDOW, "wgetparent", "", "wgetparent", "", const=True)
a(int,    "wgetscrreg", "int* top, int* bottom", "wgetscrreg", "top, bottom", const=True)

a(int, "erase",    "", "werase",    "")
a(int, "clear",    "", "wclear",    "")
a(int, "clrtobot", "", "wclrtobot", "") # TODO: also add mv?
a(int, "clrtoeol", "", "wclrtoeol", "") # TODO: also add mv?


#a(int, "attron",   "int attrs",    "wattron",    "attrs")
#a(int, "attroff",  "int attrs",    "wattroff",   "attrs")
#a(int, "attrset",  "int attrs",    "wattrset",   "attrs")
#a(int, "attr_get",  "attr_t* attrs, NCURSES_PAIRS_T* pair, void* opts", "wattr_get", "attrs, pair, opt")
#a(int, "attr_on",      wattr_on (WINDOW *, attr_t, void *);		/* implemented */
#a(int, "attr_off",     wattr_off (WINDOW *, attr_t, void *);	/* implemented */
#a(int, "attr_set",     wattr_set (WINDOW *, attr_t, NCURSES_PAIRS_T, void *);	/* generated */


# getcurx, getcury, getbegx, getbegy, ...
for f in braceexpand("get{cur,beg,max,par}{x,y}"):
    a(int, f, "", f, "", const=True)

# getyx, getbegyx ...
for f in braceexpand("get{,beg,max,par}yx"):
    a(void, f, "int& y, int& x", f, "y, x", const=True)

# TODO: ifdef NCURSES_EXT_FUNCS
for f in braceexpand("is_{cleared,idcok,idlok,immedok,keypad,leaveok,nodelay,notimeout,pad,scrollok,subwin,syncok}"):
    a(bool, f, "", f, "", const=True)

# OUTPUT functions
mv(int, "addch",  "Chr ch",              "waddch_generic",    "ch",                 template='class Chr')
mv(int, "delch",  "",                    "wdelch",            "")
mv(int, "addstr", "const Str& s",        "waddnstr_generic",  "cstr(s), len(s)",    template='class Str')
mv(int, "addstr", "const Str& s, int n", "waddnstr_generic",  "cstr(s), n",         template='class Str')
mv(int, "insstr", "const Str& s",        "winsnstr_generic",  "cstr(s), len(s)",    template='class Str')
mv(int, "insstr", "const Str& s, int n", "winsnstr_generic",  "cstr(s), n",         template='class Str')
# INPUT functions
mv(int, "getch",  "",                    "wgetch",            "")
mv(int, "getch",  "wint_t* ch",          "wget_wch",          "ch")
mv(int, "getstr", "Str& s",              "wgetnstr_generic",  "cstr(s), in_len(s)", template='class Str')
mv(int, "getstr", "Str& s, int n",       "wgetnstr_generic",  "cstr(s), n",         template='class Str')
mv(int, "instr",  "Str& s",              "winnstr_generic",   "cstr(s), in_len(s)", template='class Str')
mv(int, "instr",  "Str& s, int n",       "winnstr_generic",   "cstr(s), n",         template='class Str')

CURSES_FUNCTIONS = ''
CURSES_WINDOW_METHODS = ''

for f in funcs:
    CURSES_FUNCTIONS += '\n'+f.make_function(prefix)
    CURSES_FUNCTIONS += '\n'+f.make_function(prefix, None, 'stdscr')
    if f.mv:
        CURSES_FUNCTIONS += '\n'+f.make_function(prefix, mv=True)
        CURSES_FUNCTIONS += '\n'+f.make_function(prefix, None, 'stdscr', mv=True)

    CURSES_WINDOW_METHODS += '\n'+f.make_method(prefix, 'win')
    if f.mv:
        CURSES_WINDOW_METHODS += '\n'+f.make_method(prefix, 'win', mv=True)

for f in funcs:
    define[f.name] = prefix+f.name

# TODO: wget_wch in braceexpand (I forgot what i meant with this...)
for f in braceexpand('{mv,}{w,}addch'):             define[f] = prefix+'addch'
for f in braceexpand('{mv,}{w,}getch'):             define[f] = prefix+'getch'
for f in braceexpand('{mv,}{w,}add{n,}{w,}str'):    define[f] = prefix+'addstr'
for f in braceexpand('{mv,}{w,}ins{n,_nw,_w}str'):  define[f] = prefix+'insstr'
for f in braceexpand('{mv,}{w,}get{n,_nw,_w}str'):  define[f] = prefix+'getstr'
for f in braceexpand('{mv,}{w,}in{ch,}str'):        define[f] = prefix+'instr'

REDEFS = ''
for from_, to_ in define.items():
    REDEFS += f'''\n#undef  {from_}\n#define {from_} {to_}'''

REDEFS += '''
#undef clear
#undef erase
#undef move
'''

with open('curs.tpl.cpp', 'r') as fh:
    print('// This file was generated by:', sys.argv)
    print(string.Template(fh.read()).substitute(**globals()))

