
# Install

It requires gcc 4.9+ and c11 standard.

For now the latest centos still has no gcc-4.9 included.
We need to install gcc-4.9 with devtoolset-3.

To install gcc 4.9 on centos:

```
yum install centos-release-scl-rh -y
yum install devtoolset-3-gcc -y

```

To use gcc-4.9 in devtoolset-3, start a new bash with devtoolset-3 enabled:

```
scl enable devtoolset-3 bash
```

To enable devtoolset-3 for ever, add the following line into `.bashrc`:

```
source /opt/rh/devtoolset-3/enable
```
