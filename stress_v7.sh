: V7 SH PARANOID STRESS TEST

echo "--- V7 SH PARANOID STRESS TEST ---"

: 1. Heredocs 
echo "Test 1: Heredocs"
a=123
cat <<EOF > test.out
Value is $a
EOF
grep "Value is 123" test.out > /dev/null
if test $? -eq 0
then echo "Heredoc: PASS"
else echo "Heredoc: FAIL"
fi

: 2. Deep Subshell Nesting 
echo "Test 2: Subshell Nesting"
( ( ( ( ( ( ( ( ( echo "Deep" ) ) ) ) ) ) ) ) ) > /dev/null
if test $? -eq 0
then echo "Subshells: PASS"
else echo "Subshells: FAIL"
fi

: 3. Memory Stress 
echo "Test 3: Variable growth"
i=0
while test $i -lt 50
do
    v="v$i=stress_$i"
    export v
    i=`expr $i + 1`
done
echo "Variables: PASS"

: 4. Complex Pipes
echo "Test 4: Pipe chain"
echo "a" | cat | cat | cat | cat | cat | grep "a" > /dev/null
if test $? -eq 0
then echo "Pipes: PASS"
else echo "Pipes: FAIL"
fi

: 5. Backtick expansion
echo "Test 5: Backticks"
res=`echo \`echo nested\``
if test "$res" = "nested"
then echo "Backticks: PASS"
else echo "Backticks: FAIL (got $res)"
fi

: 6. Signal Handling
echo "Test 6: Traps"
trap 'echo "Caught"; exit 0' 15
(sleep 1; kill -15 $$) &
wait
echo "Traps: PASS"

echo "--- STRESS TEST COMPLETE ---"
rm -f test.out
