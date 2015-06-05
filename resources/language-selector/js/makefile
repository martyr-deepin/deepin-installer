com_dir = ../../common/js
com_coffee = ${com_dir}/config.coffee ${com_dir}/utils.coffee ${com_dir}/module.coffee ${com_dir}/widget.coffee ${com_dir}/selector.coffee 

all: ${com_coffee} theme.coffee language.coffee
	cat $^ | coffee -c -s  > language.js
