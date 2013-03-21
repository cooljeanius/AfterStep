#!/bin/sh

if test -f $HOME/.afterstep/banner; then
	ascompose -T Banner -f $HOME/.afterstep/banner --no-shape -q -g +0-0  --topmost --timeout 200 -I
else
    	ascompose -T Banner -f ${prefix}/share/afterstep/banner --no-shape -q -g +0-0  --topmost --timeout 200 -I	
fi

