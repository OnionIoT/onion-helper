#!/bin/sh /etc/rc.common
# Copyright (C) 2016 Onion Corporation
START=61

USE_PROCD=1
BIN="/usr/sbin/onion-helper"
OPT=""

start_service() {
        procd_open_instance
        procd_set_param command $BIN $OPT
        procd_set_param respawn
        procd_close_instance
}


