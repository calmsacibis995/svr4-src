;	Copyright (c) 1987  Intel Corporation	
;	All Rights Reserved	

;	INTEL CORPORATION PROPRIETARY INFORMATION	

;	This software is supplied to AT & T under the terms of a license    
;	agreement with Intel Corporation and may not be copied nor         
;	disclosed except in accordance with the terms of that agreement.  	

; ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/example.d/example.csd	1.3"

; This is the SUBMIT file for building the example line discipline
;program to run under CCI as a line discipline job on the 186/410

plm86 example.p86 

;Link the object to the interface libraries with the BIND option
;to produce a Load Time Locatable (LTL) image. 

;NOTE: The Nucleus interface libraries (RPIFC.LIB and RPIFL.LIB)
;are the standard RMX I.7 libraries with the MBII calls (rq$create$port,
;rq$delete$port, rq$set$interconnect, and rq$get$interconnect)
;removed. These calls now exist in the message passing interface
;libraries (MPIFC.LIB and MPIFL.LIB). If the standard RMX I.7 Nucleus
;libraries are used, the message passing libraries should be linked
;BEFORE the Nucleus libraries.

link86 &
example.obj, &
:lang:plm86.lib, &
/rmx86/lib/rpifc.lib, &
/rmx86/lib/mpifc.lib &
to example bind mempool(10000,0b0000h) & 
segsize(stack(+2000)) &
FASTLOAD oc(purge)
