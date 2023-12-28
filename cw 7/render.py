
import os
rootdir = './'
for filename in os.listdir(rootdir):
    if filename.endswith(".md"):
        name = filename[:-3]
        os.system(f'pandoc -s -o "{name}.html" "{name}.md" --mathjax --css style.css')   