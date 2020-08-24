#!/usr/bin/env bash

gcloud compute networks create game-network
gcloud compute firewall-rules create chat-server --network game-network --allow tcp:80