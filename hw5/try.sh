if [ "$2" = "n" ]; then
	make clean
	make mac
	make $1
fi
c='./ref/adtTest-mac.'
m='./adtTest.'
if [ "$1" = "a" ]; then
	echo "-------ARRAY-------"
	t='array';
elif [ "$1" = "d" ]; then
	echo "-------DList-------"
	t='dlist';
else
	echo "-------BST-------"
	t='bst';
fi
rm vm61 vm62 vm63 vm64 vm65 ta1 ta2 ta3 ta4 ta5
echo "===Comparing do1==="
$m$t <tests/do1&> vm61
$c$t <tests/do1&> ta1
colordiff ta1 vm61
echo "===Comparing do2==="
$m$t <tests/do2&> vm62
$c$t <tests/do2&> ta2
colordiff ta2 vm62
echo "===Comparing do3==="
$m$t <tests/do3&> vm63
$c$t <tests/do3&> ta3
colordiff ta3 vm63
echo "===Comparing do4==="
$m$t <tests/do4&> vm64
$c$t <tests/do4&> ta4
colordiff ta4 vm64
echo "===Comparing do5==="
$m$t <tests/do5&> vm65
$c$t <tests/do5&> ta5
colordiff ta5 vm65
echo "===Comparing do6==="
$m$t <tests/do6&> vm66
$c$t <tests/do6&> ta6
colordiff ta6 vm66

