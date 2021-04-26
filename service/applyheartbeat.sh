cd /mnt/dietpi_userdata &&
tar -xzf heartbeat.tgz &&
cp heartbeat/heartbeat.service /etc/systemd/system/ &&
systemctl enable heartbeat &&
systemctl start heartbeat