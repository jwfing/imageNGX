#!/bin/bash

OUTPUT_LOG_ROOT=/var/log/lean-image/
ERROR_LOG_ROOT=/var/log/lean-image/
BIN_DIR=/var/backends/lean-image/bin/output
BASE_DIR=$(dirname $0)

if grep -i ubuntu /etc/issue > /dev/null; then
    OUTPUT_LOG_ROOT=/mnt/leancloud/logs/lean-image/
    ERROR_LOG_ROOT=/mnt/leancloud/logs/lean-image/
    BIN_DIR=/mnt/leancloud/lean-image/bin/
fi

WORKER=4
PORT=9100
SERVERS=""
for i in `seq 1 $WORKER`
do
    OUTPUT_LOG="${OUTPUT_LOG_ROOT}/output-$i.log"
    ERROR_LOG="${ERROR_LOG_ROOT}/error-$i.log"
    cd ${BIN_DIR} && ./imageService $PORT >>$OUTPUT_LOG 2>>$ERROR_LOG&
    cd-
    SERVERS="server 127.0.0.1:${PORT};\n${SERVERS}"
    (( PORT++ ))
done

# update nginx

sed "s/<SERVERS>/${SERVERS}/g" $BASE_DIR/nginx.conf.tmpl > $BASE_DIR/nginx.conf

sudo cp $BASE_DIR/nginx.conf /etc/nginx/sites-available/image-service
sudo /etc/init.d/nginx restart
#
#while true;
#do
#    cd $BIN_DIR && ./imageService >>$OUTPUT_LOG 2>>$ERROR_LOG
#    sleep 15
#done
