#!/bin/bash
sudo ./compile.sh

mkdir -p /usr/local/bin/the-retro-center
sudo cp -r ./bin/* /usr/local/bin/the-retro-center/

sudo cp -r boat-rudder.service /etc/systemd/system/

sudo systemctl daemon-reload
sudo systemctl enable boat-rudder.service
sudo systemctl start boat-rudder.service

