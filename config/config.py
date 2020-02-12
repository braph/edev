#!/usr/bin/python

import sys, os
from lxml import etree

os.chdir(os.path.dirname(sys.argv[0]))

doc  = etree.parse('config.xml')
root = doc.getroot()

options = []
for o in root:
    options.append(dict(
        type=o.attrib['type'],
        name=o.attrib['name'],
        default=o.attrib['default'],
        parse=o.attrib['parse'],
        lateinit=o.attrib.get('lateinit', '')
    ))

with open('config.members.hpp', 'w') as fh:
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))
    for o in options:
        print('extern', o['type'], o['name'].replace('.', '_'), end=';\n', file=fh)

with open('config.members.set.cpp', 'w') as fh:
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    for o in options:
        print('else if (option == \"%s\") %s = %s(value);' % (
            o['name'], o['name'].replace('.', '_'), o['parse']
        ), file=fh)

with open('config.members.declare.cpp', 'w') as fh:
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))

    pad = max(map(lambda o: len(o['name']), options))
    fmt = '%%-%ds Config :: %%s' % pad

    for o in options:
        print(fmt % (o['type'], o['name'].replace('.', '_')), end='', file=fh)

        if o['lateinit'] != 'yes':
            print(' =', o['default'], end='', file=fh);
        else:
            print(' /* will be initialized later */', end='', file=fh)

        print(';', file=fh)

with open('config.members.initialize.cpp', 'w') as fh:
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))
    for o in options:
        if o['lateinit'] != 'yes':
            continue
        print('%s = %s(%s);' % (
            o['name'].replace('.', '_'), o['parse'], o['default']
        ), file=fh)
