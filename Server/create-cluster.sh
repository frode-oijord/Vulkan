#!/usr/bin/env bash

gcloud beta container --project "foijord-project" clusters create "game-cluster" \
--zone "europe-west1-b" \
--no-enable-basic-auth \
--cluster-version "1.15.12-gke.2" \
--machine-type "e2-micro" \
--image-type "COS" \
--disk-type "pd-standard" \
--disk-size "100" \
--metadata disable-legacy-endpoints=true \
--scopes "https://www.googleapis.com/auth/cloud-platform" \
--num-nodes "3" \
--enable-stackdriver-kubernetes \
--enable-ip-alias \
--network "projects/foijord-project/global/networks/game-network" \
--subnetwork "projects/foijord-project/regions/europe-west1/subnetworks/game-network" \
--default-max-pods-per-node "110" \
--enable-legacy-authorization \
--no-enable-master-authorized-networks \
--addons HorizontalPodAutoscaling \
--enable-autoupgrade \
--enable-autorepair \
--max-surge-upgrade 1 \
--max-unavailable-upgrade 0

gcloud container clusters get-credentials game-cluster
