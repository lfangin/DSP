# coding = big5-hkscs
import sys
map1 = open("Big5-ZhuYin.map", 'r', encoding = "big5-hkscs")
map2 = open("ZhuYin-Big5.map", 'w', encoding = "big5-hkscs")
dict = dict()

for line in map1:
    big5 = line.split(' ')
    zhuyin = big5[1].split('/')
    for element in zhuyin:
        if element[:1] in dict:
            if big5[0] not in dict[element[:1]]:
                dict[element[:1]].append(big5[0])
        else :
            dict[element[:1]] = [big5[0]]
for key, value in dict.items():
    map2.write(key + ' ')
    for word in value:
        map2.write(' ' + word)
    map2.write('\n')
    for word in value:
        map2.write(word + "  " + word + '\n')
map1.close()
map2.close()
