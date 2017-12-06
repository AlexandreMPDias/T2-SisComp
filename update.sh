#!/bin/bash
echo "Press [CTRL+C] to stop.."
while true
do
    echo atualizando
    git pull
    git add *
    git commit -m "ok"
    git push
	sleep 300
done