#!/usr/bin/python

import sys, os
from lxml import etree

os.chdir(os.path.dirname(sys.argv[0]))
cmd = sys.argv[1]

doc  = etree.parse('config.xml')
root = doc.getroot()

options = []
for o in root:
    options.append(dict(
        type=o.attrib['type'],
        name=o.attrib['name'],
        default=o.attrib['default'],
        parse=o.attrib['parse']
    ))

if cmd == 'hpp':
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))
    for o in options:
        print(o['type'], o['name'].replace('.', '_'), end=';\n')

elif cmd == 'cpp-set':
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    for o in options:
        print('else if (option == \"%s\") %s = %s(value);' % (
            o['name'], o['name'].replace('.', '_'), o['parse']
        ))

elif cmd == 'cpp-decl':
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))

    pad = max(map(lambda o: len(o['name']), options))
    fmt = '%%-%ds Config :: %%s' % pad

    for o in options:
        print(fmt % (o['type'], o['name'].replace('.', '_')))

elif cmd == 'cpp-init':
    options.sort(key=lambda o: o['name'])
    options.sort(key=lambda o: len(o['name']))
    options.sort(key=lambda o: o['type'])
    options.sort(key=lambda o: len(o['type']))
    for o in options:
        print('%s = %s;' % (
            o['name'].replace('.', '_'), o['default']
        ))
else:
    print('Invalid command. Choose one of hpp, cpp-set')
