#!/bin/bash

NUM_REQUESTS=20
URL="http://localhost:8080"

echo "Sending $NUM_REQUESTS concurrent requests to $URL ..."

for i in $(seq 1 $NUM_REQUESTS); do
    curl -s -o /dev/null -w "Request $i: %{http_code}\n" $URL &
done

wait
echo "All requests done."
