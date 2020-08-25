#!/usr/bin/env bash

gcloud compute networks create game-network
gcloud compute firewall-rules create http-server --network game-network --allow tcp:80
gcloud compute firewall-rules create chat-server --network game-network --allow tcp:8080-8090