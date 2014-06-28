com_dir = ../../common/js

com_coffee = ${com_dir}/utils.coffee ${com_dir}/animation.coffee ${com_dir}/module.coffee ${com_dir}/widget.coffee ${com_dir}/selector.coffee

installer_coffee = conf.coffee base.coffee dialog.coffee welcome.coffee part_util.coffee part.coffee progress.coffee finish.coffee installer.coffee

all: ${com_coffee} ${installer_coffee}
	coffee -cj installer.js ${com_coffee} ${installer_coffee}
