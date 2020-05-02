#!/bin/sh

FIFO_NAME="fifo"
Q="Q1"
U="U1"
q_log="${Q}.log"
q_err="${Q}.err"
u_log="${U}.log"
u_err="${U}.err"

test_logs() {
  echo ""

  nIWANT="$(grep -c IWANT "$u_log")"
  nIAMIN="$(grep -c IAMIN "$u_log")"
  nCLOSD="$(grep -c CLOSD "$u_log")"
  nFAILD="$(grep -c FAILD "$u_log")"

  nRECVD="$(grep -c RECVD "$q_log")"
  nENTER="$(grep -c ENTER "$q_log")"
  nTIMUP="$(grep -c TIMUP "$q_log")"
  n2LATE="$(grep -c 2LATE "$q_log")"
  nGAVUP="$(grep -c GAVUP "$q_log")"

  fail=0
  if [ "$nIWANT" != "$nRECVD" ]; then
    fail=1
    echo "nIWANT != nRECVD"
  fi
  if [ "$nIAMIN" != "$nENTER" ]; then
    fail=1
    echo "nIAMIN != nENTER"
  fi
  if [ "$nIAMIN" != "$nENTER" ] || [ "$nENTER" != "$nTIMUP" ]; then
    fail=1
    echo "nIAMIN != nENTER or nENTER != nTIMUP"
  fi
  if [ "$n2LATE" != "$nCLOSD" ]; then
    fail=1
    echo "n2LATE != nCLOSD"
  fi
  if [ "$nFAILD" != "$nGAVUP" ]; then
    fail=1
    echo "nFAILD != nGAVUP"
  fi

  [ -z "$(cat "$q_log")" ] || [ -z "$(cat "$u_log")" ] && echo "WARNING. At least 1 empry log file."

  if [ "$fail" -eq 1 ]; then
    echo "FAILURE. Num of 2LATE (Q1) is different from num of CLOSD (U1)."
    echo "ERROR Q:"
    cat "$q_err"
    echo "ERROR U:"
    cat "$u_err"
  else
    echo "OK. SUCCESS."
  fi
}

start_test() {
  echo "Starting server."
  ./$Q "$@" "$FIFO_NAME" >"$q_log" 2>"$q_err" &
  user_pid=$!
  sleep 1
  echo "Starting user."
  ./$U "$1" "$2" "$FIFO_NAME" >"$u_log" 2>"$u_err"
  echo "Waiting for server to finish."
  wait $user_pid # wait for the background process so we know they both finish

  test_logs
}

make
echo ""

if [ -z "$1" ] || [ "$1" -eq 1 ]; then
  # For part 1
  echo "Test 1"
  start_test -t 10
else
  # For part 2
  echo "Test 2"
  start_test -t 10 -l 5 -n 7
fi

printf "Remove .log and .err files? [Y/n] "
read -r ans
[ -z "$ans" ] || [ "$ans" = "y" ] || [ "$ans" = "Y" ] && rm -f "${Q}.log" "${Q}.err" "${U}.log" "${U}.err"

make clean
exit 0