import yaml
import json
from os import listdir
from os.path import isfile, join, splitext
import codecs

dir = '.'
files = [ join(dir,f) for f in listdir(dir) if isfile(join(dir,f)) and splitext(f)[1] == '.yaml']

for f in files:
    fin = codecs.open(f, 'r', encoding='utf-8')
    yml = fin.read()
    fin.close()

    data_in  = yaml.load(yml)
    data_out = json.dumps(data_in, indent=2, ensure_ascii=False, sort_keys=True)

    fout = codecs.open(splitext(f)[0] + '.json', 'w', encoding='utf-8')
    fout.write(data_out)
    fout.close()