
RealtimeFabricClient.py --server=1 --api-ip=MASTER_IP --add-peer=tcp://SLAVE1_IP:10000
RealtimeFabricClient.py --server=2 --api-ip=SLAVE1_IP --add-peer=tcp://MASTER_IP:10000

RealtimeFabricClient.py --server=1 --api-ip=MASTER_IP --add-peer=tcp://SLAVE2_IP:10000
RealtimeFabricClient.py --server=3 --api-ip=SLAVE2_IP --add-peer=tcp://MASTER_IP:10000

transmute_sql.py --server=1 --api-ip=MASTER_IP --send-to-fabric < /usr/local/deep-demo/master1.sql

transmute_sql.py --server=2 --api-ip=SLAVE1_IP --send-to-fabric < /usr/local/deep-demo/slave1.sql
transmute_sql.py --server=3 --api-ip=SLAVE2_IP --send-to-fabric < /usr/local/deep-demo/slave3-1.sql

transmute_sql.py --server=1 --api-ip=MASTER_IP --send-to-fabric < /usr/local/deep-demo/master2.sql

transmute_sql.py --server=2 --api-ip=SLAVE1_IP --send-to-fabric < /usr/local/deep-demo/slave2.sql
transmute_sql.py --server=3 --api-ip=SLAVE2_IP --send-to-fabric < /usr/local/deep-demo/slave3-2.sql

START_AT=$SECONDS
transmute_sql.py --server=1 --api-ip=MASTER_IP --send-to-fabric < /usr/local/deep-demo/inserts.sql
STOP_AT=$SECONDS

query_time=`expr $STOP_AT - $START_AT`

echo "It took $query_time seconds to insert all the data into the master."

transmute_sql.py --server=2 --api-ip=SLAVE1_IP --send-to-fabric < /usr/local/deep-demo/slave3.sql
transmute_sql.py --server=3 --api-ip=SLAVE2_IP --send-to-fabric < /usr/local/deep-demo/slave3-3.sql

