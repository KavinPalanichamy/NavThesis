#!/bin/bash -ex
DOCKER_BUILDKIT=1 docker build -t xsens:latest -f Dockerfile .
