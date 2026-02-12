#!/bin/bash
list=$(cat $1 | grep -v '/' | sed 's/\.sv//')
echo $list
