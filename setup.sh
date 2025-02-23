#!/bin/bash

PROJECT_DIR=$PWD
GOINFRE_DIR="/Users/orekabe/goinfre"
VM_NAME="ft_shield_vm"
BOX_NAME="debian/bullseye64"

cd "${PROJECT_DIR}" || (echo "${PROJECT_DIR} not found" && exit 1)
mkdir -p "${GOINFRE_DIR}/vagrant_vms"
if ! vagrant box list | grep -q "${BOX_NAME} "; then
	vagrant box add ~/Downloads/debian_bullseye64 --name ${BOX_NAME}
fi
vagrant up