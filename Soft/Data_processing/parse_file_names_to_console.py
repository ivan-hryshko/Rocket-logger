from os import listdir
from os.path import isfile, join
import sys

if __name__ == "__main__":
    FILE_FOLDER_PATH = sys.argv[1]
allFilesList = [f for f in listdir(FILE_FOLDER_PATH) if isfile(join(FILE_FOLDER_PATH, f))]

sortFileList = []
for files in allFilesList:
    pack = [(files.replace(".BIN","").split('_'))[0]] + [files]
    sortFileList.append(pack)
filepack = dict.fromkeys((part[0] for part in sortFileList),[])
for num in sortFileList:
    filepack[num[0]] = filepack[num[0]] + [num[1]]
for num in filepack:
    print(num + ": ")
    print(filepack[num])