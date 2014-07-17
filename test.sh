#!/bin/bash
set -i

reg2='
#(include|define)
#include
#define
unsigned
auto
break
c(ase|har|onst|countinue)
d(efault|o|ouble)
e(lse|num|extern)
f(loat|or)
goto
i(f|nt)
long
si(gned|zeof)
short
st(atic|ruct)
switch
typedef
un(signed|ion)
vo(id|latile)
while
'
ts=$(cat main.txt)
for r in $reg2;do
    echo "Test reg $r  ==> `./reg $r $ts`"
    echo "Test nfa $r  ==> `./nfa $r $ts`"
done
