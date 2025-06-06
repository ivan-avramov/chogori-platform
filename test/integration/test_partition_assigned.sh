#!/bin/bash
set -e
topname=$(dirname "$0")
source ${topname}/common_defs.sh
cd ${topname}/../..
new_nodepool="tcp+k2rpc://0.0.0.0:10099"

# start nodepool
./build/src/k2/cmd/nodepool/nodepool ${COMMON_ARGS} -c${#EPS[@]} --tcp_endpoints ${EPS[@]} --k23si_persistence_endpoint ${PERSISTENCE} --prometheus_port 63001 &
nodepool_child_pid=$!

# start persistence
./build/src/k2/cmd/persistence/persistence ${COMMON_ARGS} -c1 --tcp_endpoints ${PERSISTENCE} --prometheus_port 63002 &
persistence_child_pid=$!

# start tso
./build/src/k2/cmd/tso/tso ${COMMON_ARGS} -c1 --tcp_endpoints ${TSO} --prometheus_port 63003 --tso.clock_poller_cpu=${TSO_POLLER_CORE} &
tso_child_pid=$!

# starting the CPO with a nodepool that won't exist at start up
./build/src/k2/cmd/controlPlaneOracle/cpo_main ${COMMON_ARGS} -c1 --tcp_endpoints ${CPO} --data_dir ${CPODIR} --txn_heartbeat_deadline=10s --prometheus_port 63000 --assignment_timeout=1s --nodepool_endpoints $new_nodepool --tso_endpoints ${TSO} --tso_error_bound=100us --persistence_endpoints ${PERSISTENCE} &
cpo_child_pid=$!

function finish {
  rv=$?
  # cleanup code
  rm -rf ${CPODIR}

  kill ${cpo_child_pid}
  echo "Waiting for cpo child pid: ${cpo_child_pid}"
  wait ${cpo_child_pid}

  kill ${nodepool_child_pid}
  echo "Waiting for nodepool child pid: ${nodepool_child_pid}"
  wait ${nodepool_child_pid}

  kill ${persistence_child_pid}
  echo "Waiting for persistence child pid: ${persistence_child_pid}"
  wait ${persistence_child_pid}

  kill ${tso_child_pid}
  echo "Waiting for tso child pid: ${tso_child_pid}"
  wait ${tso_child_pid}
  echo ">>>> Test ${0} finished with code ${rv}"
}
trap finish EXIT

sleep 1

./build/test/cpo/cpo_client_test ${COMMON_ARGS} --cpo ${CPO} --prometheus_port 63100
