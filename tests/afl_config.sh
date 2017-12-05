#!/bin/bash
AFL_USE_ASAN=1 ./configure CC=afl-gcc LD=afl-gcc--disable-shared
