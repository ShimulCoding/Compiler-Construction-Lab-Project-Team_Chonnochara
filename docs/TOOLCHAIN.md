# Build Environment Plan

Status: **Installed and validated under explicit staged approval on 21 July 2026. The complete environment gate passed before M2 implementation.**

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

## 2. Current verified host

- Windows WSL package: `2.7.10.0`; default WSL architecture version: 2.
- Distribution: `Ubuntu-24.04`, running as WSL version 2.
- Ubuntu release: `24.04.4 LTS`; initialized Linux user: `shimul`.
- The canonical repository remains the Windows `E:` checkout and is readable through `/mnt/e`.
- Native Windows compilation remains unsupported; Windows Git is authoritative for status, diff, identity, commits, and pushes.
- WSL performs GCC/Flex/Bison/Make builds and tests against the mounted checkout. No second Linux clone or permanent global `safe.directory` exception is used.

## 3. Recorded approved setup commands

### 3.1 WSL and Ubuntu installation

The normal pinned-distribution path was approved and completed; the web-download fallback was not needed:

```powershell
wsl --install -d Ubuntu-24.04
```

Windows was restarted when requested before post-install verification continued.

### 3.2 Post-restart verification

```powershell
wsl --version
wsl --status
wsl --list --verbose
```

The observed WSL version exceeded the required `2.4.10`, the default was already 2, and Ubuntu was already WSL2. No `wsl --update` or distribution-conversion command was needed.

Launch the distribution in its Linux home directory:

```powershell
wsl ~ -d Ubuntu-24.04
```

On first launch, create the requested Linux username and password. The password is used for `sudo`; it is not stored in this repository.

### 3.3 Ubuntu package installation

Inside Ubuntu—not PowerShell—the separately approved minimal package commands completed successfully:

```bash
cat /etc/os-release
sudo apt update
sudo apt install build-essential flex bison
```

Git `2.43.0` was already present and was not reinstalled solely for the project. No `apt upgrade` or unrelated package command ran. `build-essential` supplies GCC, GNU Make, C library headers, and related build tools. Flex's planned `%option noyywrap` avoids an unnecessary `-lfl` link dependency.

Observed versions were recorded rather than assumed from package documentation:

```bash
gcc --version
make --version
flex --version
bison --version
git --version
dpkg-query -W -f='${binary:Package}\t${Version}\n' build-essential gcc make flex bison git
```

| Tool | Observed version |
| --- | --- |
| GCC | `13.3.0` |
| GNU Make | `4.3` |
| Flex | `2.6.4` |
| Bison | `3.8.2` |
| Git | `2.43.0` |

Observed package versions: `build-essential 12.10ubuntu1`, `gcc 4:13.2.0-7ubuntu1`, `make 4.3-4.1build2`, `flex 2.6.4-8.2build1`, `bison 2:3.8.2+dfsg-1build2`, and `git 1:2.43.0-1ubuntu7.3`.

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

### 4.2 Deferred Linux-filesystem alternative

The team currently chooses one Windows canonical worktree to avoid divergent clones and to keep contributor identity/commit handling on Windows. Do not create the following alternative unless a later measured performance or filesystem problem causes the team to approve a migration:

```bash
mkdir -p ~/projects
cd ~/projects
git clone https://github.com/ShimulCoding/Compiler-Construction-Lab-Project-Team_Chonnochara.git
cd Compiler-Construction-Lab-Project-Team_Chonnochara
```

If a migration is later approved, reopen the IDE/Codex against that clone and choose exactly one canonical worktree. Do not let Windows and WSL clones diverge.

## 5. Git identity policy

Windows Git is authoritative for contributor identity, status, commits, and pushes. Do not set a global shared-laptop identity. The first approved M1 commit used its approved per-command identity:

```bash
git -c user.name='Shimul' -c user.email='shimulc17@gmail.com' commit -m 'Shimul: Defined the mandatory language contract and formal grammar'
```

The user approved this exact identity and message for the M1 commit. Future contributors must continue using their own repository-scoped or per-command identity; do not copy Shimul's identity for their work.

## 6. M2 environment gate

All preconditions passed before M2 implementation:

1. `wsl --version` reports WSL `2.4.10` or later.
2. `wsl --list --verbose` shows `Ubuntu-24.04` on WSL 2.
3. `/etc/os-release` identifies Ubuntu 24.04 LTS.
4. `gcc`, `make`, `flex`, `bison`, and `git` version commands succeed.
5. The repository is reachable from the selected canonical workspace.
6. `git status --short --branch` shows the expected branch and reviewed state.

Additional temporary tests outside the repository passed:

- strict C11 GCC compilation and execution;
- Bison header/C generation with `-Wall -Wcounterexamples`;
- Flex generation consuming the Bison token header;
- direct GCC linking without `-lfl` using `%option noyywrap`; and
- `make clean`, `make`, and executable acceptance through a temporary Makefile.

M2 then passed its repository build with `make clean`, `make`, and `make test` under the same environment.

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
