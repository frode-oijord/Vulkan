#!/usr/bin/env bash

sudo certbot certonly \
    --manual \
    --preferred-challenge=dns \
    --email frode.oijord@protonmmail.com \
    --server https://acme-v02.api.letsencrypt.org/directory \
    --agree-tos \
    --manual-public-ip-logging-ok \
    -d oijord.tech \
    -d *.oijord.tech
