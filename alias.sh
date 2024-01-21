#!/bin/bash
shopt -s expand_aliases  # Enables alias expansion.

__tr() {
	./build/T --gtest_filter=diskcache.${1}
}

alias _tr='__tr'
alias _tl='./build/T --gtest_list_tests'
