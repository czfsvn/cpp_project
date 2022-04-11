'''
Author: czf
Date: 2022-03-25 20:53:55
LastEditTime: 2022-03-26 10:27:58
LastEditors: czf
Description: file description
FilePath: /cpp_project/tools/python/readxml1.py
custom_string_obkoro1
Copyright (c) 2022 by cn/cn, All Rights Reserved. 
'''

# /bin/python

import xml.etree.ElementTree as ET


def readxml_country():
    tree = ET.parse("/Users/cn/work/cpp/cpp_project/configs/country.xml")
    root = tree.getroot()
    print("root.flag: %s : root.attrib: %s" % (root.tag, root.attrib))

    for child in root:
        print("\t tag: %s, attrib: %s(%s--)" %
              (child.tag, child.attrib, child.attrib))
        for children in child:
            print("\t\t tag: %s, attrib: %s" %
                  (children.tag, children.attrib))
            for key, val in children.attrib.items():
                print("\t\t\t %s ====  %s" % (key, val))


def main():
    readxml_country()
    print("hello, main")


if __name__ == "__main__":
    main()
