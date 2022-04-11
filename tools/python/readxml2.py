'''
Author: czf
Date: 2022-03-26 10:30:42
LastEditTime: 2022-03-26 22:21:41
LastEditors: czf
Description: file description
FilePath: /cpp_project/tools/python/readxml2.py
custom_string_obkoro1
Copyright (c) 2022 by cn/cn, All Rights Reserved.
'''
# /bin/python
# -*- coding: gb2312 -*-

import xml.etree.ElementTree as ET
import sys
import os


# file = sys.stdout


def printline(file, depth, txt):
    for i in range(0, depth):
        print("\t", end="")
    print(txt)


def is_key_words(str):
    return str in ("vector", "list", "dequeue", "map", "multimap", "unordered_map", "container_", "cont_", "key_", "key__")


def is_seq_cont(cont):
    return cont in ("vector", "list", "dequeue")


def is_map_cont(cont):
    return cont in ("map", "multimap", "unordered_map")


def to_class_name(str):
    return str[0:1].upper() + str[1:]


def to_member_name(str):
    return str[0:1].lower() + str[1:]


def get_cont(fields):
    cont = ""
    key = ""
    for k, v in fields.items():
        if k in ("container_", "cont_"):
            cont = v
        elif k in ("key_", "key__"):
            key = v

    return cont, key


def hpp(file, node, dept):
    classname = node.tag
    fields = {}

    # printline(file, dept, "\n")
    printline(file, dept, "struct %s" % to_class_name(classname))
    printline(file, dept, "{")

    printline(file, dept, "private:")
    for k, v in node.attrib.items():
        fields[k] = v
        if is_key_words(v) or is_key_words(k):
            continue
        printline(file, dept+1, "%s \t%s_ = {};" % (v, k))

    printline(file, dept, "public:")
    for k, v in node.attrib.items():
        fields[k] = v
        if is_key_words(v) or is_key_words(k):
            continue
        printline(file, dept+1, "const %s& \t%s() const" % (v, k))
        printline(file, dept+1, "{")
        printline(file, dept+2, "return %s_;" % (k))
        printline(file, dept+1, "}")

    for child in node:
        hpp(file, child, dept + 1)

    printline(file, dept, "};\n")

    container, key = get_cont(fields)
    if len(container):
        if is_seq_cont(container):
            printline(file, dept, "using %sCont = std::%s<%s>;" %
                      (to_class_name(classname), container, to_class_name(classname)))
            printline(file, dept, "using %sIter = %sCont::iterator;" %
                      (to_class_name(classname), to_class_name(classname)))
            printline(file, dept, "using %sConstIter = %sCont::const_iterator;\n" %
                      (to_class_name(classname), to_class_name(classname)))
            printline(file, dept, "%sCont %s_cont = {};\n" %
                      (to_class_name(classname),  to_member_name(classname)))

        elif is_map_cont(container):
            printline(file, dept, "using %sCont = std::%s<%s, %s>;" %
                      (to_class_name(classname), container, key, to_class_name(classname)))
            printline(file, dept, "using %sIter = %sCont::iterator;" %
                      (to_class_name(classname), to_class_name(classname)))
            printline(file, dept, "using %sConstIter = %sCont::const_iterator;\n" %
                      (to_class_name(classname), to_class_name(classname)))
            printline(file, dept, "%sCont %s_cont = {};\n" %
                      (to_class_name(classname),  to_member_name(classname)))
    else:
        printline(file, dept, "%s %s = {};\n" %
                  (to_class_name(classname),  to_member_name(classname)))


def readxml2():
    tree = ET.parse("/Users/cn/work/cpp/cpp_project/configs/readxml2.xml")
    root = tree.getroot()
    classname = root.tag

    dept = 0
    file = sys.stdout

    printline(file, dept, "#pragma once\n")
    printline(file, dept, "namespace xml_%s" % classname)
    printline(file, dept, "{")
    for child in root:
        hpp(file, child, dept + 1)

    printline(file, dept, "}\n")


def main():
    readxml2()
    print("hello, main")


if __name__ == "__main__":
    main()
