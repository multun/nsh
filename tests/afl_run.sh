#!/bin/bash
AFL_SKIP_CPUFREQ=1 afl-fuzz -i in -o out ./42sh @@
