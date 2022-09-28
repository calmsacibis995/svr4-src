#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)fmli:oh/oh.mk	1.31"
#


INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY=liboh.a
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE= -I$(HEADER1) -I$(CURSES_H)
DFLAGS=	-DWISH
CFLAGS= -O	
AR=	ar


$(LIBRARY): \
		$(LIBRARY)(action.o) \
		$(LIBRARY)(alias.o) \
		$(LIBRARY)(cmd.o) \
		$(LIBRARY)(detab.o) \
		$(LIBRARY)(detect.o) \
		$(LIBRARY)(dispfuncs.o) \
		$(LIBRARY)(evstr.o) \
		$(LIBRARY)(externoot.o) \
		$(LIBRARY)(fm_mn_par.o) \
		$(LIBRARY)(getval.o) \
		$(LIBRARY)(helptext.o) \
		$(LIBRARY)(if_ascii.o) \
		$(LIBRARY)(if_dir.o) \
		$(LIBRARY)(if_init.o) \
		$(LIBRARY)(if_exec.o) \
		$(LIBRARY)(if_form.o) \
		$(LIBRARY)(if_help.o) \
		$(LIBRARY)(if_menu.o) \
		$(LIBRARY)(ifuncs.o) \
		$(LIBRARY)(interrupt.o) \
		$(LIBRARY)(is_objtype.o) \
		$(LIBRARY)(misc.o) \
		$(LIBRARY)(namecheck.o) \
		$(LIBRARY)(nextpart.o) \
		$(LIBRARY)(obj_to_opt.o) \
		$(LIBRARY)(obj_to_par.o) \
		$(LIBRARY)(odftread.o) \
		$(LIBRARY)(odikey.o) \
		$(LIBRARY)(oh_init.o) \
		$(LIBRARY)(ootpart.o) \
		$(LIBRARY)(ootread.o) \
		$(LIBRARY)(opt_rename.o) \
		$(LIBRARY)(optab.o) \
		$(LIBRARY)(optabfuncs.o) \
		$(LIBRARY)(ott_mv.o) \
		$(LIBRARY)(partab.o) \
		$(LIBRARY)(partabfunc.o) \
		$(LIBRARY)(pathtitle.o) \
		$(LIBRARY)(pathfstype.o) \
		$(LIBRARY)(path_to_vp.o) \
		$(LIBRARY)(pathott.o) \
		$(LIBRARY)(scram.o) \
		$(LIBRARY)(slk.o) \
		$(LIBRARY)(suffuncs.o) \
		$(LIBRARY)(typefuncs.o) \
		$(LIBRARY)(typetab.o)

$(LIBRARY)(action.o): $(HEADER1)/moremacros.h
$(LIBRARY)(action.o): $(HEADER1)/token.h
$(LIBRARY)(action.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(action.o): $(HEADER1)/wish.h
$(LIBRARY)(action.o): action.c

$(LIBRARY)(alias.o): $(HEADER1)/actrec.h
$(LIBRARY)(alias.o): $(HEADER1)/ctl.h
$(LIBRARY)(alias.o): $(HEADER1)/moremacros.h
$(LIBRARY)(alias.o): $(HEADER1)/sizes.h
$(LIBRARY)(alias.o): $(HEADER1)/slk.h
$(LIBRARY)(alias.o): $(HEADER1)/token.h
$(LIBRARY)(alias.o): $(HEADER1)/wish.h
$(LIBRARY)(alias.o): alias.c

$(LIBRARY)(cmd.o): $(HEADER1)/actrec.h
$(LIBRARY)(cmd.o): $(HEADER1)/ctl.h
$(LIBRARY)(cmd.o): $(HEADER1)/eval.h
$(LIBRARY)(cmd.o): $(HEADER1)/interrupt.h
$(LIBRARY)(cmd.o): $(HEADER1)/menudefs.h
$(LIBRARY)(cmd.o): $(HEADER1)/moremacros.h
$(LIBRARY)(cmd.o): $(HEADER1)/sizes.h
$(LIBRARY)(cmd.o): $(HEADER1)/slk.h
$(LIBRARY)(cmd.o): $(HEADER1)/terror.h
$(LIBRARY)(cmd.o): $(HEADER1)/token.h
$(LIBRARY)(cmd.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(cmd.o): $(HEADER1)/wish.h
$(LIBRARY)(cmd.o): ./fm_mn_par.h
$(LIBRARY)(cmd.o): cmd.c

$(LIBRARY)(detab.o): $(HEADER1)/detabdefs.h
$(LIBRARY)(detab.o): $(HEADER1)/typetab.h
$(LIBRARY)(detab.o): $(HEADER1)/wish.h
$(LIBRARY)(detab.o): detab.c

$(LIBRARY)(detect.o): $(HEADER1)/detabdefs.h
$(LIBRARY)(detect.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(detect.o): $(HEADER1)/parse.h
$(LIBRARY)(detect.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(detect.o): $(HEADER1)/sizes.h
$(LIBRARY)(detect.o): $(HEADER1)/typetab.h
$(LIBRARY)(detect.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(detect.o): $(HEADER1)/wish.h
$(LIBRARY)(detect.o): detect.c

$(LIBRARY)(dispfuncs.o): $(HEADER1)/moremacros.h
$(LIBRARY)(dispfuncs.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(dispfuncs.o): $(HEADER1)/terror.h
$(LIBRARY)(dispfuncs.o): $(HEADER1)/typetab.h
$(LIBRARY)(dispfuncs.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(dispfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(dispfuncs.o): dispfuncs.c

$(LIBRARY)(evstr.o): $(HEADER1)/eval.h
$(LIBRARY)(evstr.o): $(HEADER1)/interrupt.h
$(LIBRARY)(evstr.o): $(HEADER1)/moremacros.h
$(LIBRARY)(evstr.o): $(HEADER1)/terror.h
$(LIBRARY)(evstr.o): $(HEADER1)/token.h
$(LIBRARY)(evstr.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(evstr.o): $(HEADER1)/wish.h
$(LIBRARY)(evstr.o): ./fm_mn_par.h
$(LIBRARY)(evstr.o): evstr.c

$(LIBRARY)(externoot.o): $(HEADER1)/sizes.h
$(LIBRARY)(externoot.o): externoot.c

$(LIBRARY)(fm_mn_par.o): $(HEADER1)/actrec.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/eval.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/moremacros.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/terror.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/token.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fm_mn_par.o): $(HEADER1)/wish.h
$(LIBRARY)(fm_mn_par.o): ./fm_mn_par.h
$(LIBRARY)(fm_mn_par.o): fm_mn_par.c

$(LIBRARY)(getval.o): $(HEADER1)/ctl.h
$(LIBRARY)(getval.o): $(HEADER1)/eval.h
$(LIBRARY)(getval.o): $(HEADER1)/form.h
$(LIBRARY)(getval.o): $(HEADER1)/interrupt.h
$(LIBRARY)(getval.o): $(HEADER1)/moremacros.h
$(LIBRARY)(getval.o): $(HEADER1)/terror.h
$(LIBRARY)(getval.o): $(HEADER1)/token.h
$(LIBRARY)(getval.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(getval.o): $(HEADER1)/winp.h
$(LIBRARY)(getval.o): $(HEADER1)/wish.h
$(LIBRARY)(getval.o): ./fm_mn_par.h
$(LIBRARY)(getval.o): ./objform.h
$(LIBRARY)(getval.o): getval.c

$(LIBRARY)(helptext.o): helptext.c

$(LIBRARY)(if_ascii.o): $(HEADER1)/but.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/mio.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/obj.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/procdefs.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/sizes.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/retcds.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/typetab.h
$(LIBRARY)(if_ascii.o): $(HEADER1)/wish.h
$(LIBRARY)(if_ascii.o): if_ascii.c

$(LIBRARY)(if_dir.o): $(HEADER1)/actrec.h
$(LIBRARY)(if_dir.o): $(HEADER1)/ctl.h
$(LIBRARY)(if_dir.o): $(HEADER1)/menudefs.h
$(LIBRARY)(if_dir.o): $(HEADER1)/message.h
$(LIBRARY)(if_dir.o): $(HEADER1)/moremacros.h
$(LIBRARY)(if_dir.o): $(HEADER1)/sizes.h
$(LIBRARY)(if_dir.o): $(HEADER1)/slk.h
$(LIBRARY)(if_dir.o): $(HEADER1)/terror.h
$(LIBRARY)(if_dir.o): $(HEADER1)/token.h
$(LIBRARY)(if_dir.o): $(HEADER1)/typetab.h
$(LIBRARY)(if_dir.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(if_dir.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(if_dir.o): $(HEADER1)/wish.h
$(LIBRARY)(if_dir.o): if_dir.c

$(LIBRARY)(if_exec.o): $(HEADER1)/terror.h
$(LIBRARY)(if_exec.o): $(HEADER1)/wish.h
$(LIBRARY)(if_exec.o): if_exec.c

$(LIBRARY)(if_form.o): $(HEADER1)/actrec.h
$(LIBRARY)(if_form.o): $(HEADER1)/ctl.h
$(LIBRARY)(if_form.o): $(HEADER1)/eval.h
$(LIBRARY)(if_form.o): $(HEADER1)/form.h
$(LIBRARY)(if_form.o): $(HEADER1)/interrupt.h
$(LIBRARY)(if_form.o): $(HEADER1)/menudefs.h
$(LIBRARY)(if_form.o): $(HEADER1)/message.h
$(LIBRARY)(if_form.o): $(HEADER1)/moremacros.h
$(LIBRARY)(if_form.o): $(HEADER1)/slk.h
$(LIBRARY)(if_form.o): $(HEADER1)/terror.h
$(LIBRARY)(if_form.o): $(HEADER1)/token.h
$(LIBRARY)(if_form.o): $(HEADER1)/typetab.h
$(LIBRARY)(if_form.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(if_form.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(if_form.o): $(HEADER1)/winp.h
$(LIBRARY)(if_form.o): $(HEADER1)/wish.h
$(LIBRARY)(if_form.o): $(HEADER1)/sizes.h
$(LIBRARY)(if_form.o): ./fm_mn_par.h
$(LIBRARY)(if_form.o): ./objform.h
$(LIBRARY)(if_form.o): if_form.c

$(LIBRARY)(if_help.o): $(HEADER1)/actrec.h
$(LIBRARY)(if_help.o): $(HEADER1)/ctl.h
$(LIBRARY)(if_help.o): $(HEADER1)/form.h
$(LIBRARY)(if_help.o): $(HEADER1)/interrupt.h
$(LIBRARY)(if_help.o): $(HEADER1)/message.h
$(LIBRARY)(if_help.o): $(HEADER1)/moremacros.h
$(LIBRARY)(if_help.o): $(HEADER1)/slk.h
$(LIBRARY)(if_help.o): $(HEADER1)/terror.h
$(LIBRARY)(if_help.o): $(HEADER1)/token.h
$(LIBRARY)(if_help.o): $(HEADER1)/typetab.h
$(LIBRARY)(if_help.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(if_help.o): $(HEADER1)/vt.h
$(LIBRARY)(if_help.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(if_help.o): $(HEADER1)/winp.h
$(LIBRARY)(if_help.o): $(HEADER1)/wish.h
$(LIBRARY)(if_help.o): $(HEADER1)/sizes.h
$(LIBRARY)(if_help.o): ./fm_mn_par.h
$(LIBRARY)(if_help.o): ./objhelp.h
$(LIBRARY)(if_help.o): if_help.c

$(LIBRARY)(if_init.o): $(HEADER1)/attrs.h
$(LIBRARY)(if_init.o): $(HEADER1)/color_pair.h
$(LIBRARY)(if_init.o): $(HEADER1)/ctl.h
$(LIBRARY)(if_init.o): $(HEADER1)/interrupt.h
$(LIBRARY)(if_init.o): $(HEADER1)/moremacros.h
$(LIBRARY)(if_init.o): $(HEADER1)/slk.h
$(LIBRARY)(if_init.o): $(HEADER1)/terror.h
$(LIBRARY)(if_init.o): $(HEADER1)/token.h
$(LIBRARY)(if_init.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(if_init.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(if_init.o): $(HEADER1)/wish.h
$(LIBRARY)(if_init.o): ./fm_mn_par.h
$(LIBRARY)(if_init.o): if_init.c

$(LIBRARY)(if_menu.o): $(HEADER1)/actrec.h
$(LIBRARY)(if_menu.o): $(HEADER1)/ctl.h
$(LIBRARY)(if_menu.o): $(HEADER1)/interrupt.h
$(LIBRARY)(if_menu.o): $(HEADER1)/menudefs.h
$(LIBRARY)(if_menu.o): $(HEADER1)/message.h
$(LIBRARY)(if_menu.o): $(HEADER1)/moremacros.h
$(LIBRARY)(if_menu.o): $(HEADER1)/slk.h
$(LIBRARY)(if_menu.o): $(HEADER1)/terror.h
$(LIBRARY)(if_menu.o): $(HEADER1)/token.h
$(LIBRARY)(if_menu.o): $(HEADER1)/typetab.h
$(LIBRARY)(if_menu.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(if_menu.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(if_menu.o): $(HEADER1)/wish.h
$(LIBRARY)(if_menu.o): $(HEADER1)/sizes.h
$(LIBRARY)(if_menu.o): ./fm_mn_par.h
$(LIBRARY)(if_menu.o): ./objmenu.h
$(LIBRARY)(if_menu.o): if_menu.c

$(LIBRARY)(ifuncs.o): $(HEADER1)/but.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/message.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/mio.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/moremacros.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/obj.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/retcds.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/sizes.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/terror.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/token.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/typetab.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/windefs.h
$(LIBRARY)(ifuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(ifuncs.o): ifuncs.c

$(LIBRARY)(interrupt.o): $(HEADER1)/wish.h
$(LIBRARY)(interrupt.o): interrupt.c

$(LIBRARY)(is_objtype.o): $(HEADER1)/detabdefs.h
$(LIBRARY)(is_objtype.o): $(HEADER1)/terror.h
$(LIBRARY)(is_objtype.o): $(HEADER1)/typetab.h
$(LIBRARY)(is_objtype.o): $(HEADER1)/wish.h
$(LIBRARY)(is_objtype.o): is_objtype.c

$(LIBRARY)(make_obj.o): $(HEADER1)/mio.h
$(LIBRARY)(make_obj.o): $(HEADER1)/moremacros.h
$(LIBRARY)(make_obj.o): $(HEADER1)/obj.h
$(LIBRARY)(make_obj.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(make_obj.o): $(HEADER1)/sizes.h
$(LIBRARY)(make_obj.o): $(HEADER1)/typetab.h
$(LIBRARY)(make_obj.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(make_obj.o): $(HEADER1)/wish.h
$(LIBRARY)(make_obj.o): make_obj.c

$(LIBRARY)(misc.o): $(HEADER1)/moremacros.h
$(LIBRARY)(misc.o): $(HEADER1)/wish.h
$(LIBRARY)(misc.o): misc.c

$(LIBRARY)(namecheck.o): $(HEADER1)/message.h
$(LIBRARY)(namecheck.o): $(HEADER1)/mio.h
$(LIBRARY)(namecheck.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(namecheck.o): $(HEADER1)/typetab.h
$(LIBRARY)(namecheck.o): $(HEADER1)/wish.h
$(LIBRARY)(namecheck.o): namecheck.c

$(LIBRARY)(nextpart.o): $(HEADER1)/but.h
$(LIBRARY)(nextpart.o): $(HEADER1)/mio.h
$(LIBRARY)(nextpart.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(nextpart.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(nextpart.o): $(HEADER1)/typetab.h
$(LIBRARY)(nextpart.o): $(HEADER1)/wish.h
$(LIBRARY)(nextpart.o): nextpart.c

$(LIBRARY)(obj_to_opt.o): $(HEADER1)/but.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/mio.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/typetab.h
$(LIBRARY)(obj_to_opt.o): $(HEADER1)/wish.h
$(LIBRARY)(obj_to_opt.o): obj_to_opt.c

$(LIBRARY)(obj_to_par.o): $(HEADER1)/but.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/mio.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/typetab.h
$(LIBRARY)(obj_to_par.o): $(HEADER1)/wish.h
$(LIBRARY)(obj_to_par.o): obj_to_par.c

$(LIBRARY)(odftread.o): $(HEADER1)/detabdefs.h
$(LIBRARY)(odftread.o): $(HEADER1)/mio.h
$(LIBRARY)(odftread.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(odftread.o): $(HEADER1)/retcds.h
$(LIBRARY)(odftread.o): $(HEADER1)/sizes.h
$(LIBRARY)(odftread.o): $(HEADER1)/terror.h
$(LIBRARY)(odftread.o): $(HEADER1)/typetab.h
$(LIBRARY)(odftread.o): $(HEADER1)/wish.h
$(LIBRARY)(odftread.o): odftread.c

$(LIBRARY)(odikey.o): $(HEADER1)/moremacros.h
$(LIBRARY)(odikey.o): $(HEADER1)/sizes.h
$(LIBRARY)(odikey.o): $(HEADER1)/typetab.h
$(LIBRARY)(odikey.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(odikey.o): $(HEADER1)/wish.h
$(LIBRARY)(odikey.o): odikey.c

$(LIBRARY)(oh_init.o): $(HEADER1)/typetab.h
$(LIBRARY)(oh_init.o): $(HEADER1)/wish.h
$(LIBRARY)(oh_init.o): oh_init.c

$(LIBRARY)(one_menfun.o): $(HEADER1)/but.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/mio.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/moremacros.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/obj.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/sizes.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/terror.h
$(LIBRARY)(one_menfun.o): $(HEADER1)/wish.h
$(LIBRARY)(one_menfun.o): one_menfun.c

$(LIBRARY)(ootpart.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(ootpart.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(ootpart.o): $(HEADER1)/typetab.h
$(LIBRARY)(ootpart.o): $(HEADER1)/wish.h
$(LIBRARY)(ootpart.o): ootpart.c

$(LIBRARY)(ootread.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(ootread.o): $(HEADER1)/mess.h
$(LIBRARY)(ootread.o): $(HEADER1)/mio.h
$(LIBRARY)(ootread.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(ootread.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(ootread.o): $(HEADER1)/sizes.h
$(LIBRARY)(ootread.o): $(HEADER1)/terror.h
$(LIBRARY)(ootread.o): $(HEADER1)/typetab.h
$(LIBRARY)(ootread.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(ootread.o): $(HEADER1)/wish.h
$(LIBRARY)(ootread.o): ootread.c

$(LIBRARY)(opt_rename.o): $(HEADER1)/but.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/mio.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/sizes.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/typetab.h
$(LIBRARY)(opt_rename.o): $(HEADER1)/wish.h
$(LIBRARY)(opt_rename.o): opt_rename.c

$(LIBRARY)(optab.o): $(HEADER1)/but.h
$(LIBRARY)(optab.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(optab.o): $(HEADER1)/mio.h
$(LIBRARY)(optab.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(optab.o): $(HEADER1)/typetab.h
$(LIBRARY)(optab.o): $(HEADER1)/wish.h
$(LIBRARY)(optab.o): optab.c

$(LIBRARY)(optabfuncs.o): $(HEADER1)/but.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/mio.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/typetab.h
$(LIBRARY)(optabfuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(optabfuncs.o): optabfuncs.c

$(LIBRARY)(ott_mv.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(ott_mv.o): $(HEADER1)/sizes.h
$(LIBRARY)(ott_mv.o): $(HEADER1)/typetab.h
$(LIBRARY)(ott_mv.o): $(HEADER1)/wish.h
$(LIBRARY)(ott_mv.o): ott_mv.c

$(LIBRARY)(partab.o): $(HEADER1)/but.h
$(LIBRARY)(partab.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(partab.o): $(HEADER1)/mio.h
$(LIBRARY)(partab.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(partab.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(partab.o): $(HEADER1)/typetab.h
$(LIBRARY)(partab.o): $(HEADER1)/wish.h
$(LIBRARY)(partab.o): partab.c

$(LIBRARY)(partabfunc.o): $(HEADER1)/but.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/ifuncdefs.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/mio.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/sizes.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/typetab.h
$(LIBRARY)(partabfunc.o): $(HEADER1)/wish.h
$(LIBRARY)(partabfunc.o): partabfunc.c

$(LIBRARY)(path_to_vp.o): $(HEADER1)/obj.h
$(LIBRARY)(path_to_vp.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(path_to_vp.o): $(HEADER1)/sizes.h
$(LIBRARY)(path_to_vp.o): $(HEADER1)/typetab.h
$(LIBRARY)(path_to_vp.o): $(HEADER1)/wish.h
$(LIBRARY)(path_to_vp.o): path_to_vp.c

$(LIBRARY)(pathott.o): $(HEADER1)/typetab.h
$(LIBRARY)(pathott.o): $(HEADER1)/sizes.h
$(LIBRARY)(pathott.o): pathott.c

$(LIBRARY)(pathtitle.o): $(HEADER1)/wish.h
$(LIBRARY)(pathtitle.o): pathtitle.c

$(LIBRARY)(pathfstype.o): pathfstype.c

$(LIBRARY)(scram.o): $(HEADER1)/exception.h
$(LIBRARY)(scram.o): $(HEADER1)/moremacros.h
$(LIBRARY)(scram.o): $(HEADER1)/obj.h
$(LIBRARY)(scram.o): $(HEADER1)/parse.h
$(LIBRARY)(scram.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(scram.o): $(HEADER1)/sizes.h
$(LIBRARY)(scram.o): $(HEADER1)/retcds.h
$(LIBRARY)(scram.o): $(HEADER1)/terror.h
$(LIBRARY)(scram.o): $(HEADER1)/token.h
$(LIBRARY)(scram.o): $(HEADER1)/typetab.h
$(LIBRARY)(scram.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(scram.o): $(HEADER1)/winp.h
$(LIBRARY)(scram.o): $(HEADER1)/wish.h
$(LIBRARY)(scram.o): scram.c

$(LIBRARY)(slk.o): $(HEADER1)/ctl.h
$(LIBRARY)(slk.o): $(HEADER1)/interrupt.h
$(LIBRARY)(slk.o): $(HEADER1)/moremacros.h
$(LIBRARY)(slk.o): $(HEADER1)/slk.h
$(LIBRARY)(slk.o): $(HEADER1)/token.h
$(LIBRARY)(slk.o): $(HEADER1)/wish.h
$(LIBRARY)(slk.o): ./fm_mn_par.h
$(LIBRARY)(slk.o): slk.c

$(LIBRARY)(suffuncs.o): suffuncs.c

$(LIBRARY)(typefuncs.o): $(HEADER1)/mio.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/moremacros.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/sizes.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/typetab.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(typefuncs.o): $(HEADER1)/wish.h
$(LIBRARY)(typefuncs.o): typefuncs.c

$(LIBRARY)(typetab.o): $(HEADER1)/mio.h
$(LIBRARY)(typetab.o): $(HEADER1)/moremacros.h
$(LIBRARY)(typetab.o): $(HEADER1)/partabdefs.h
$(LIBRARY)(typetab.o): $(HEADER1)/sizes.h
$(LIBRARY)(typetab.o): $(HEADER1)/typetab.h
$(LIBRARY)(typetab.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(typetab.o): $(HEADER1)/wish.h
$(LIBRARY)(typetab.o): typetab.c

.c.a:
	$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets #####

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
