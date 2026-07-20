# Build Environment Plan

Status: **Plan only. No WSL feature, Ubuntu distribution, compiler package, or system dependency was installed or changed during M1. Explicit user approval is required before running any installation command below.**

Checked against official Microsoft, Canonical, and Ubuntu sources on 21 July 2026.

## 1. Recommended pinned environment

| Component | Project choice |
| --- | --- |
| Host integration | WSL 2 |
| Linux distribution | Ubuntu 24.04 LTS (`Ubuntu-24.04`) |
| Implementation language | C11 |
| Scanner generator | Distribution-supported Flex |
| Parser generator | Distribution-supported Bison |
| C compiler | Distribution-supported GCC |
| Build tool | GNU Make from `build-essential` |
| Version control | Git |
| Test shell | Bash/POSIX shell supplied by Ubuntu |

Ubuntu 26.04 is the current latest LTS and the moving `Ubuntu` WSL alias now points to it. This project deliberately pins `Ubuntu-24.04`: it is a mature supported LTS, satisfies the instructor's Ubuntu 22.04-or-later guidance, and avoids silently changing the grading/development baseline. Ubuntu 26.04 may be a later compatibility check, not the primary M1 environment.

Do not substitute the moving `Ubuntu` alias in recorded setup commands.

## 2. Current host audit

- The registry reports `Windows 10 Home Single Language`, display version `25H2`, build `26200.8894`; the build number exceeds Microsoft's WSL install minimum of Windows 10 build 19041.
- `wsl.exe --status` and `wsl.exe --list --verbose` report that WSL is not installed.
- Native Windows `PATH` does not currently provide Make, GCC, Flex, or Bison.
- An old MinGW GCC/Make exists off `PATH`, but Flex/Bison do not; this is not the supported project environment.
- Hardware virtualization status could not be confirmed under the current read-only permissions. It must be enabled in firmware for WSL 2.

WSL installation can require administrator/UAC approval, Windows optional-feature changes, network downloads, virtualization support, and a restart. None is authorized merely by this document.

## 3. Commands requiring future explicit approval

### 3.1 Elevated PowerShell

Open PowerShell with **Run as administrator** and run only after approval:

```powershell
wsl --install -d Ubuntu-24.04
```

This is the official one-command path that enables required WSL components and installs the explicitly pinned distribution. Restart Windows when prompted.

If the Store-backed download hangs or is unavailable, the official network-download fallback is:

```powershell
wsl --install --web-download -d Ubuntu-24.04
```

Use the fallback only after the normal command actually fails and the user approves the additional download attempt.

### 3.2 PowerShell after restart

Update and set the default architecture, then verify:

```powershell
wsl --update
wsl --set-default-version 2
wsl --version
wsl --status
wsl --list --verbose
```

The final listing must contain `Ubuntu-24.04` with `VERSION` equal to `2`. Only if it shows version 1, run:

```powershell
wsl --set-version Ubuntu-24.04 2
```

Canonical's current Ubuntu-on-WSL guidance requires WSL `2.4.10` or later for the modern Ubuntu 24.04-and-later image format. Confirm that `wsl --version` reports at least `2.4.10` before treating the environment as ready; if it does not, stop and diagnose the failed `wsl --update` rather than continuing.

Launch the distribution in its Linux home directory:

```powershell
wsl ~ -d Ubuntu-24.04
```

On first launch, create the requested Linux username and password. The password is used for `sudo`; it is not stored in this repository.

### 3.3 Ubuntu shell

Inside Ubuntu—not PowerShell—run only after package-install approval:

```bash
cat /etc/os-release
sudo apt update
sudo apt install build-essential flex bison git
```

`build-essential` supplies GCC, GNU Make, C library headers, and related build tools. Flex's planned `%option noyywrap` avoids an unnecessary `-lfl` link dependency. If that decision changes, check/install the appropriate `libfl-dev` package explicitly rather than assuming it exists.

Record observed versions instead of assuming archive versions:

```bash
gcc --version
make --version
flex --version
bison --version
git --version
dpkg-query -W -f='${binary:Package}\t${Version}\n' build-essential gcc make flex bison git
```

## 4. Repository location after installation

### 4.1 Immediate access to the current Windows workspace

The current Windows `E:` checkout is expected at this quoted WSL path:

```bash
cd '/mnt/e/4.2/Compiler Construction Lab/Compiler-construction-project/Compiler-Construction-Lab-Project-Team_Chonnochara'
pwd
git status --short --branch
```

Quoting is mandatory because the path contains spaces. Do not run simultaneous Git mutations from Windows Git and WSL Git.

This mounted path is acceptable for the first build smoke test. Test scripts must normalize CRLF/LF and must not depend on executable bits when operating under `/mnt/e`.

### 4.2 Preferred long-term Linux workspace

Microsoft recommends storing Linux-command-line projects in the WSL filesystem for performance and Linux filesystem semantics. Only after the approved M1 commit is pushed should the team create a fresh canonical Linux clone:

```bash
mkdir -p ~/projects
cd ~/projects
git clone https://github.com/ShimulCoding/Compiler-Construction-Lab-Project-Team_Chonnochara.git
cd Compiler-Construction-Lab-Project-Team_Chonnochara
```

Reopen the IDE/Codex against that clone and choose one canonical working copy. Do not let Windows and WSL clones diverge. Windows Explorer can access the Linux files through `\\wsl$\Ubuntu-24.04\home\<linux-user>\projects\`.

## 5. Git identity policy in WSL

Do not set a global shared-laptop identity. The first approved M1 commit uses per-command identity from inside the intended repository:

```bash
git -c user.name='Shimul' -c user.email='shimulc17@gmail.com' commit -m 'Shimul: Defined the mandatory language contract and formal grammar'
```

The user approved this exact identity and message for the M1 commit. Future contributors must continue using their own repository-scoped or per-command identity; do not copy Shimul's identity for their work.

## 6. M2 environment gate

M2 implementation must not begin until all of these preconditions are observed and recorded:

1. `wsl --version` reports WSL `2.4.10` or later.
2. `wsl --list --verbose` shows `Ubuntu-24.04` on WSL 2.
3. `/etc/os-release` identifies Ubuntu 24.04 LTS.
4. `gcc`, `make`, `flex`, `bison`, and `git` version commands succeed.
5. The repository is reachable from the selected canonical workspace.
6. `git status --short --branch` shows the expected branch and reviewed state.

The first M2 implementation check is a tiny generated Flex/Bison smoke build. It is an M2 task/exit check, not a circular pre-M2 requirement and not an M1 activity.

## 7. Official sources

- [Microsoft: Install WSL](https://learn.microsoft.com/en-us/windows/wsl/install)
- [Microsoft: Basic WSL commands](https://learn.microsoft.com/en-us/windows/wsl/basic-commands)
- [Microsoft: Working across Windows and Linux filesystems](https://learn.microsoft.com/en-us/windows/wsl/filesystems)
- [Microsoft's live WSL distribution catalog](https://raw.githubusercontent.com/microsoft/WSL/master/distributions/DistributionInfo.json)
- [Canonical: Ubuntu on WSL downloads](https://ubuntu.com/download/wsl)
- [Canonical: Install Ubuntu on WSL 2](https://documentation.ubuntu.com/wsl/latest/guides/install-ubuntu-wsl2/)
- [Ubuntu: Package management](https://ubuntu.com/server/docs/how-to/software/package-management/)
- [Ubuntu 24.04 package: build-essential](https://packages.ubuntu.com/noble/build-essential)
- [Ubuntu 24.04 package: Flex](https://packages.ubuntu.com/noble/flex)
- [Ubuntu 24.04 package: Bison](https://packages.ubuntu.com/noble/bison)
- [Ubuntu 24.04 package: Git](https://packages.ubuntu.com/noble/git)
