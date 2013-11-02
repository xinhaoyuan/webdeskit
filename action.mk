.PHONY: ${PRJ}-install ${PRJ}-debug

${PRJ}-install: ${PRJ}
	-${V}pkill webdeskit-main
	${V}cp -r ${T_OBJ}/$<-local ${T_OBJ}/$< ~/bin
	${V}echo INSTALL FINISHED
