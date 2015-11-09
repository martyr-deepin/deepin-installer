com_dir = ../../common/js

com_coffee = ${com_dir}/config.coffee ${com_dir}/utils.coffee ${com_dir}/animation.coffee ${com_dir}/module.coffee ${com_dir}/widget.coffee ${com_dir}/selector.coffee ${com_dir}/tooltip.coffee

installer_coffee = trie.coffee base.coffee conf.coffee dialog.coffee virtual.coffee welcome.coffee part_util.coffee part.coffee progress.coffee finish.coffee installer.coffee

all: ${com_coffee} ${installer_coffee}
	cat ${com_coffee} ${installer_coffee} | coffee -c -s > installer.js
