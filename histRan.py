import random
f = open('hist.txt','w')
for i in range(0,255):
	f.write(str(int(random.uniform(0,255))))
	f.write('\n')