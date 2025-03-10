#!/usr/bin/env python3
# emoji-test.txt from https://unicode.org/Public/emoji/16.0/emoji-test.txt

print('static const std::unordered_map<QString, QString> emojiNameMap = {')

file = open('emoji-test.txt')

for line in file:
    l = line.strip()

    if not l:
        continue 

    if l[0] == '#':
        continue

    parts = l.split('#', 1)
    info = parts[1].lstrip().split(' ', 2)
    code = '"{emoji}", "{name}"'.format(emoji=info[0], name=info[2])
    code = '    {' + code + '},'
    print(code)

print('};')
