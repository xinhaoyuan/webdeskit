.PHONY: all

RESFILES := $(shell find html) $(shell find bin)

all: ${T_OBJ}/webdeskit

${T_OBJ}/webdeskit: webdeskit ${RESFILES}
	@echo Generate webdeskit package
	${V}cp webdeskit $@
	${V}chmod +x $@
	${V}mkdir -p ${T_OBJ}/webdeskit-local ${T_OBJ}/webdeskit-local/cache
	${V}cp -r html ${T_OBJ}/webdeskit-local
	${V}cp -r bin ${T_OBJ}/webdeskit-local
