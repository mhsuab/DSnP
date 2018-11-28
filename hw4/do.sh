make clean
make mac
make
echo "========== tests/do1& =========="
echo "./memTest"
./memTest <tests/do1&> vm61.txt
echo "./ref/memTest"
./ref/memTest <tests/do1&> ta1.txt
colordiff vm61.txt ta1.txt
echo "=========== tests/do1 ==========="
echo "./memTest"
./memTest <tests/do1> vm61.txt
echo "./ref/memTest"
./ref/memTest <tests/do1> ta1.txt
colordiff vm61.txt ta1.txt
echo "=========== tests/do2& ==========="
echo "./memTest"
./memTest <tests/do2&> vm62.txt
echo "./ref/memTest"
./ref/memTest <tests/do2&> ta2.txt
colordiff vm62.txt ta2.txt
echo "============ tests/do2 ==========="
echo "./memTest"
./memTest <tests/do2> vm62.txt
echo "./ref/memTest"
./ref/memTest <tests/do2> ta2.txt
colordiff vm62.txt ta2.txt
echo "=========== tests/do3& ============"
echo "./memTest"
./memTest <tests/do3&> vm63.txt
echo "./ref/memTest"
./ref/memTest <tests/do3&> ta3.txt
colordiff vm63.txt ta3.txt
echo "========== tests/do3 ==========="
echo "./memTest"
./memTest <tests/do3> vm63.txt
echo "./ref/memTest"
./ref/memTest <tests/do3> ta3.txt
colordiff vm63.txt ta3.txt
echo "========== tests/do4& ==========="
echo "./memTest"
./memTest <tests/do4&> vm64.txt
echo "./ref/memTest"
./ref/memTest <tests/do4&> ta4.txt
colordiff vm64.txt ta4.txt
echo "=========== tests/do4 ==========="
echo "./memTest"
./memTest <tests/do4> vm64.txt
echo "./ref/memTest"
./ref/memTest <tests/do4> ta4.txt
colordiff vm64.txt ta4.txt
echo "========== tests/do5& ==========="
echo "./memTest"
./memTest <tests/do5&> vm65.txt
echo "./ref/memTest"
./ref/memTest <tests/do5&> ta5.txt
colordiff vm65.txt ta5.txt
echo "========== tests/do5 ============="
echo "./memTest"
./memTest <tests/do5> vm65.txt
echo "./ref/memTest"
./ref/memTest <tests/do5> ta5.txt
colordiff vm65.txt ta5.txt
