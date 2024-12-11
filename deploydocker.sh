#!/bin/bash

cd /slurm-docker-cluster || exit

echo "Building Docker images..."
docker compose build

echo "Starting Docker containers..."
docker compose up -d

echo "Waiting for containers to initialize..."
sleep 5  # Adjust the wait time as needed

echo "Registering the Slurm cluster..."
./register_cluster.sh

