package main

var shell_code = `
#!/bin/bash
language=$1
if [ -z "${language}" ];then
	language=en
fi

lang2locale() {
	langpart="${1%%_*}"
	if [ "$1" != "C" ]; then
		# Match the language code with 1st field in languagelist
		line=$(grep -v "^#" /usr/share/localechooser/languagelist | cut -f1,3,5 -d\; | grep -v ';C$' | grep "^$langpart[_;]")
		if [ -n "$line" ]; then
			if [ "$(echo "$line" | grep -c '')" -gt 1 ]; then
				# More than one match; try matching the
				# country as well.
				countrypart="${1#*_}"
				if [ "$countrypart" = "$1" ]; then
					countryline="$(echo "$line" | head -n1)"
					echo "${countryline##*;}"
					return
				fi
				countrypart="${countrypart%%[@.]*}"
				countryline="$(echo "$line" | grep ";$countrypart;" | head -n1 || true)"
				if [ "$countryline" ]; then
					echo "${countryline##*;}"
					return
				fi
			fi
			echo "${line##*;}"
		fi
	else
		echo "C"
	fi
}

locale="$(lang2locale "$language")"

LANG=$(grep "^${locale}" /usr/share/i18n/SUPPORTED | grep UTF-8 | sed -e 's/ .*//' -e q)

echo $LANG
printf 'LANGUAGE="%s"\nLANG="%s"' "${LANG%%.*}" "${LANG}" > /etc/default/locale
printf '%s UTF-8' "${LANG}" > /etc/locale.gen
/usr/sbin/locale-gen ${LANG}
[ -x /bin/zsh ] && echo | chsh -s /bin/zsh
`
