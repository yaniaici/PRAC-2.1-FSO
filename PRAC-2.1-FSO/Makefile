cocos0 : cocos0.c winsuport.o winsuport.h
	gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses


cocos1 : cocos1.c winsuport.o winsuport.h
	gcc -Wall cocos1.c winsuport.o -o cocos1 -lcurses -lpthread


cocos2 : cocos2.c winsuport.o winsuport.h
	gcc -Wall cocos2.c winsuport.o -o cocos2 -lcurses -lpthread


winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o 

clean:
	rm -f cocos0 cocos1 cocos2 winsuport.o
