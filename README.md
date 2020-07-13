# xdp-wireless-scanner
Linux application for getting main information about access points in wireless environment. Tested on Ubuntu 20.04 (kernel 5.4.0).
GUI created using Qt.

If you are interesting in XDP, you should know that build process might be a bit tricky :)
I will not describe the full process here, because there are a lot examples how to build XDP applications.

Very short:
1. XDP app is easier to build directly in the Linux kernel sources (see samples/bpf).
2. Create a library from the created object files.
3. Link created library into Qt .pro file:
    LIBS += -lxdp_wlscan
4. Add dependencies to bpf library into .pro file, something like this:
    INCLUDEPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/bpf
    INCLUDEPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf
    INCLUDEPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/perf
    INCLUDEPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/testing/selftests/bpf/
    DEPENDPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/bpf
    DEPENDPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf
    DEPENDPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/perf
    DEPENDPATH += /usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/testing/selftests/bpf/
    LIBS += -L$$PWD/../../../usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf/ -lbpf
    INCLUDEPATH += $$PWD/../../../usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf
    DEPENDPATH += $$PWD/../../../usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf
    PRE_TARGETDEPS += $$PWD/../../../usr/src/linux-source-*kernel_version*/linux-source-*kernel_version*/tools/lib/bpf/libbpf.a
5. As the application uses libnl for sending requests, it should be installed and linked into .pro file:
    LIBS += -lnl-genl-3 -lnl-3
    INCLUDEPATH += /usr/include/libnl3/
6. Hope you are lucky enough to build the app :)

You can start and stop scanning, configure scan period, sort and filter data, save the results to file.
