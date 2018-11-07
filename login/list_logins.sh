#!/bin/bash

last_ip=""
last_status=""
regex=".*(Accepted|Failed).*for (invalid user )?(.*) from ([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}).*ssh2"
green="\033[1;32m"
default="\033[1;39m"
red="\033[1;31m"

while IFS=: read line; do
	if [[ $line =~ $regex ]]; then
		# 1 = Accepted/Failed
		# 2 = invalid user (ignored)
		# 3 = username
		# 4 = IP Address
		status="${BASH_REMATCH[1]}"
		if [[ "${BASH_REMATCH[4]}" != "$last_ip" || $last_status != "$status" ]]; then
			[ "$status" == "Accepted" ] && echo -en "$green" || echo -en "$red"
			echo -e "${status}:${default} ${BASH_REMATCH[4]} (${BASH_REMATCH[3]})"
		fi
		last_ip="${BASH_REMATCH[4]}"
		last_status=$status
	fi
done <$1
