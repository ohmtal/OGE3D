#!/bin/sh

GAMEPATH="game"
# ------------------------------------------------------------------------------
SEARCH="\"art/"
REPL="\"$GAMEPATH/art/"
echo "------------------------------------------------------------------------"
echo "search $SEARCH replacement = $REPL"
echo "------------------------------------------------------------------------"
find ./ -name "*.cs" | xargs grep -l $SEARCH | xargs sed -i "s|$SEARCH|$REPL|g"
find ./ -name "*.gui" | xargs grep -l $SEARCH | xargs sed -i "s|$SEARCH|$REPL|g"
find ./ -name "*.mis" | xargs grep -l $SEARCH | xargs sed -i "s|$SEARCH|$REPL|g"
echo "------------------------------------------------------------------------"
grep -rl $SEARCH *
