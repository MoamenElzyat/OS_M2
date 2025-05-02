#!/bin/bash

echo "=== Running FCFS ==="
./bin/run FCFS > logs_fcfs.txt <<EOF
1
5
output1.txt
hello fcfs
output1.txt
EOF
echo "✅ FCFS done. Logs in logs_fcfs.txt"

echo "=== Running RR (quantum 2) ==="
./bin/run RR 2 > logs_rr2.txt <<EOF
1
5
output2.txt
hello rr2
output2.txt
EOF
echo "✅ RR (2) done. Logs in logs_rr2.txt"

echo "=== Running RR (quantum 4) ==="
./bin/run RR 4 > logs_rr4.txt <<EOF
1
5
output3.txt
hello rr4
output3.txt
EOF
echo "✅ RR (4) done. Logs in logs_rr4.txt"

echo "=== Running MLFQ ==="
./bin/run MLFQ > logs_mlfq.txt <<EOF
1
5
output4.txt
hello mlfq
output4.txt
EOF
echo "✅ MLFQ done. Logs in logs_mlfq.txt"

echo "=== ✅ All schedulers tested. Check log files for details."