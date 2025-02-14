#!/bin/bash
sudo systemctl stop boat-rudder.service
sudo systemctl disable boat-rudder.service
sudo rm /etc/systemd/system/boat-rudder.service
sudo systemctl daemon-reload
sudo rm -rf /usr/local/bin/the-retro-center/
systemctl list-unit-files | grep boat-rudder
