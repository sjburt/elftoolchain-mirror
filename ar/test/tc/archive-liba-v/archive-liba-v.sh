inittest archive-liba-v tc/archive-liba-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar cruv liba.a a1.o a2.o a3.o a4.o" work true
rundiff false
runcmd "plugin/teraser -c -t archive-liba-v liba.a" work false
runcmd "plugin/ardiff -cnlt archive-liba-v ${RLTDIR}/liba.a liba.a" work false
