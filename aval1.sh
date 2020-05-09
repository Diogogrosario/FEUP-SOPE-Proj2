#/! /bin/sh
#
# Utilização (2 lançamentos):
#
# Q1  -t 30 fifinho > $QLOG 2> q.err
# U1  -t 20 fifinho > $ULOG 2> u.err
#
# Q1  -t 15 fifinho > $QLOG 2> q.err
# U1  -t 20 fifinho > $ULOG 2> u.err
#
# Verificação:
#
# nIWANT = nRECVD = nIAMIN + nCLOSD + nFAILD
# nIAMIN = nENTER = nTIMUP
# nCLOSD = n2LATE
# nFAILD = nGAVUP

case $# in
	0) echo; echo "Usage: $0 Ulogfile Qlogfile"; echo ; exit 1 ;;
	1) echo; echo "Usage: $0 Ulogfile Qlogfile"; echo ; exit 1 ;;
	*) 	ULOG=$1
		QLOG=$2
esac

nIWANT=`grep IWANT $ULOG | wc -l`
nIAMIN=`grep IAMIN $ULOG | wc -l`
nCLOSD=`grep CLOSD $ULOG | wc -l`
nFAILD=`grep FAILD $ULOG | wc -l`
nRECVD=`grep RECVD $QLOG | wc -l`
nENTER=`grep ENTER $QLOG | wc -l`
nTIMUP=`grep TIMUP $QLOG | wc -l`
n2LATE=`grep 2LATE $QLOG | wc -l`
nGAVUP=`grep GAVUP $QLOG | wc -l`

if [ $nIWANT != $nRECVD ]
then echo "nIWANT=${nIWANT} != nRECVD=${nRECVD}"
fi
if [ $nRECVD != $((nIAMIN + nCLOSD + nFAILD)) ]
then echo "nRECVD=${nRECVD} != (nIAMIN=${nIAMIN} + nCLOSD=${nCLOSD} + nFAILD=${nFAILD})"
fi
# if [ $nRECVD != `expr $nIAMIN + $nCLOSD + $nFAILD` ]
# then echo "nRECVD=${nRECVD} != (nIAMIN=${nIAMIN} + nCLOSD=${nCLOSD} + nFAILD=${nFAILD})"
# fi
if [ $nIAMIN != $nENTER ]
then echo "nIAMIN=${nIAMIN} != nENTER=${nENTER}"
fi
if [ $nENTER != $nTIMUP ]
then echo "nENTER=${nENTER} != nTIMUP=${nTIMUP}"
fi
if [ $nCLOSD != $n2LATE ]
then echo "nCLOSD=${nCLOSD} != n2LATE=${n2LATE}"
fi
if [ $nFAILD != $nGAVUP ]
then echo "nFAILD=${nFAILD} != nGAVUP=${nGAVUP}"
fi

if [ $nIWANT = 0 ]
then echo -n "nIWANT=${nIWANT} "
fi
if [ $nIAMIN = 0 ]
then echo -n "nIAMIN=${nIAMIN} "
fi
if [ $nCLOSD = 0 ]
then echo -n "nCLOSD=${nCLOSD} "
fi
if [ $nFAILD = 0 ]
then echo -n "nFAILD=${nFAILD} "
fi
if [ $nRECVD = 0 ]
then echo -n "nRECVD=${nRECVD} "
fi
if [ $nENTER = 0 ]
then echo -n "nENTER=${nENTER} "
fi
if [ $nTIMUP = 0 ]
then echo -n "nTIMUP=${nTIMUP} "
fi
if [ $n2LATE = 0 ]
then echo -n "n2LATE=${n2LATE} "
fi
if [ $nGAVUP = 0 ]
then echo -n "nGAVUP=${nGAVUP} "
fi
echo

nrequestsU=`sort  -n -k3 $ULOG | tail -1 | awk '{print $3}' `
nrequestsQ=`sort  -n -k3 $QLOG | tail -1 | awk '{print $3}' `
echo "nrequestsU: $nrequestsU ; nrequestsQ: $nrequestsQ"
exit 0
