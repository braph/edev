#!/usr/bin/python

#int waddstr(WINDOW *win, const char *str);
#int waddnstr(WINDOW *win, const char *str, int n);
#int mvwaddstr(WINDOW *win, int y, int x, const char *str);
#int mvwaddnstr(WINDOW *win, int y, int x, const char *str, int n);

def typename(wide=0, const=0, array=0):
    return  ("const "   if const else "") + \
            ("wchar_t"  if wide  else "char") + \
            (" (&s)[N]" if array else "* s")

def method_name(mv=0, n=0):
    return  ("mvAdd"  if mv else "add") + \
            ("NStr"   if n  else "Str")

def method_params(mv=0, wide=0, const=0, array=0, n=0):
    return  ("int y, int x, " if mv else "") + \
            typename(wide=wide, const=const, array=array) + \
            (", int n"        if n  else "")

def method_template(mv=0, n=0):
    return  "template<typename T> int %s" + \
            "(%sT%s);" % (
                    "int y, int x, " if mv else "",
                    ", n"            if n  else "")

def method_body(mv=0, wide=0, n=0, const=0, array=0):
    if n:                       N = "n"
    elif const and array:       N = "N"
    else:                       N = "-1"

    return "return %swaddn%sstr(win, %ss, %s);" % (
        "mv"        if mv      else "",
        "w"         if wide    else "",
        "y, x, "    if mv      else "",
        N
    )

def gen(mv=0, n=0, wide=0):
    name = method_name(mv=mv, n=n)

    print(method_template(mv=mv, n=n) % name)

    for const in True, False:
        for array in True, False:
            r  = "template<> int %s(%s)<%s> { %s }" % (
                name,
                method_params(mv=mv, n=n, wide=wide, const=const, array=array),
                typename(wide=wide, const=const, array=array),
                method_body(mv=mv, n=n, wide=wide, const=const, array=array)
            )
            
            print(r)


for n in False, True:
    for wide in False, True:
        for mv in False, True:
            gen(mv=mv, n=n, wide=wide)


#         return """\
# return {mv}wadd{n}{w}str({yx}{T}{pn});
# """
# 
#     print("""\
# int {method_name}({yx}{T}{pn})<{T}> {
# }""")

