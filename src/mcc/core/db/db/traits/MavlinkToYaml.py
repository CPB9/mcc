# -*- coding: utf-8 -*- 

import sys, re
import yaml
from xml.dom import minidom, Node

class Conv:
    cNodes = None        

    def findChildNodesMAV_CMD(self, obj):
        for child in obj.childNodes:
            if child.nodeType == Node.ELEMENT_NODE:
                if child.tagName == 'enum':
                    if child.hasAttribute('name'):
                        if child.getAttribute('name') == 'MAV_CMD':
                            self.cNodes = child.childNodes
                obj = self.findChildNodesMAV_CMD(child)

    def xml2yaml(self, obj):
        objDict = {}
        objDict['name'] = obj.nodeName
        attrs = obj.attributes
        if attrs is not None and attrs.length > 0:
            attrDict = {}
            for idx in range(attrs.length):
                attr = attrs.item(idx)
                attrDict[attr.name] = attr.value
            objDict['attributes'] = attrDict
        text = []
        for child in obj.childNodes:
            if child.nodeType == Node.TEXT_NODE and \
                not self.isAllWhiteSpace(child.nodeValue):
                text.append(child.nodeValue)
        if text:
            textStr = "".join(text)
            objDict['text'] = textStr
        children = []
        for child in obj.childNodes:
            if child.nodeType == Node.ELEMENT_NODE:
                obj = self.xml2yaml(child)
                children.append(obj)
        if children:
            objDict['children'] = children
        return objDict

    NonWhiteSpacePattern = re.compile('\S')
    def isAllWhiteSpace(self, text):
        if self.NonWhiteSpacePattern.search(text):
            return 0
        return 1

    def fitting(self, source_list, in_file_name):
        # f = open('log.yaml', 'w')
        # f.write(self.header_string)
        # f.close()

        f = open(in_file_name, 'w')
        f.write(source_list)
        f.close()

        f = open(in_file_name).readlines()
        for element in f[:]:
            if "- {name: '#text'}" in element or "name: entry" in element or "- {name: '#comment'}" in element or "children" in element:
                f.remove(element)

        new_list = ['firmwares:\n  - name: mavlink\n    info: mavlink\n    traits:\n      - name: Mavlink\n\
        info: "Commands to be executed by the MAV. They can be executed on user request, or as part of a mission script. If the action is used in a mission, the parameter mapping to the waypoint/mission message is as follows: Param 1, Param 2, Param 3, Param 4, X: Param 5, Y:Param 6, Z:Param 7. This command list is similar what ARINC 424 is for commercial aircraft: A data format how to interpret waypoint/mission data."\n        kind: interface\n        commands:\n']
        index = -1
        for element in f:
            index = index + 1
            # Это в итоге не используется
            if element[0:27] == "- {name: description, text:":
                mav_info = element.split("'")[1]
            elif element[0:20] == "- attributes: {name:":
                com_name = element[21:element.index(',')]
                com_name_id = element.split("'")[1]
                com_info = f[index + 1].split(":")[2][1:-2]
                new_list.append('          - info: "' + com_info + '"\n')
                new_list.append('            name: ' + com_name + '\n')
                new_list.append('            name_id: ' + com_name_id + '\n')
                new_list.append('            params:\n')
            elif element[0:15] == "  - attributes:":
                par_name = f[index + 1].split(":")[1][:-1] + '_' + element.split("'")[1]
                # par_name_type = 'f32'
                # par_unit = '~'
                par_info = f[index + 2][10:-1]
                new_list.append('              - {name: ' + par_name + ' ,type: f32, unit: ~, info: "' + par_info + '"}\n')

        with open(in_file_name, 'w') as F:
            F.writelines(new_list)
        F.close()

    def usage(self):
        print ("\nUsage: python convert.py <from_file> <in_file>\n")
        sys.exit(-1)

    def start(self):
        args = sys.argv[1:]
        if len(args) != 2:
            self.usage()
        fromFileName = args[0]
        inFileName = args[1]

        self.doc = minidom.parse(fromFileName)
        self.root = self.doc.childNodes[0]
        result_list = []
        self.findChildNodesMAV_CMD(self.root)
        for element in self.cNodes:
            result_list.append(self.xml2yaml(element))
        out = yaml.dump(result_list, width = 900)
        self.fitting(out, inFileName)


if __name__ == '__main__':
    convert = Conv()
    convert.start()