/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/enetdrv/mhost.c	1.3.1.1"

/* MODULE:			 mhost.c
 *
 * PURPOSE:	Routines herein implement the LCI Consumer-side of the
 *			named-buffer manager
 *
 * MODIFICATIONS:
 *
 *	I000	6/28/88		vish		Intel
 *		Original code.
 *	I001	07/06/89	rjf		Intel
 *		Added EDL support.
 *	I002 	08/09/89	rjf,vish	Intel
 *		Added support for user data on connect & accept.
 *
 */

#include "sys/enet.h"
#include "sys/lcidef.h"

#define MAX_INDEX 255
#define TRUE    1
#define FALSE    0

extern unsigned char ics_myslotid();

typedef struct 
{
    BYTE  in_use;
    BYTE  next_idx;         /* 0xff is the last; Used for NMF & EDL */
    WORD  num_blks;  /* Denotes number of blocks the name points to */
    dword *original_blk_ptr;
    union  
    {
          dword original_buf_addr;
          char *multi_blk_ptr;
    }  blk_type;
} NTABLE;

typedef struct 
{
    BYTE blk_addr[4];
    BYTE  blk_len[2];
} BLKADR;

NTABLE name_table_struct[256];

       
BYTE cur_index;
       
BYTE get_free_table_entry();
dword encode();

BYTE register_RB(rb_ptr)
crbh  *rb_ptr;
{
     /* This routine translates all the addresses in the RB to names and
      creates name table entries */
#ifdef EDL
      BYTE idx, ina_opcode, nidx, oidx;			/* I002 */
#else
      BYTE idx, ina_opcode;				/* I002 */
#endif
      WORD num_blks;
      dword *blk_start_addr;
      
#ifdef EDL
      oidx = idx = get_free_table_entry();
#else
      idx = get_free_table_entry();
#endif
      if (idx == 0xff)
		cmn_err(CE_WARN, "No name table entry\n");
/**	cmn_err(CE_NOTE, "register rb ptr = %x\n", rb_ptr); **/

      ina_opcode = ((crbh *)rb_ptr)->c_opcode;		/* I002 */
      switch (((crbh *)rb_ptr) -> c_subsys)
      {
          case SUB_VC :
		/* I002 Start */
    		if ((ina_opcode >= (unsigned char)OP_SCR) &&
				(ina_opcode <= (unsigned char)OP_ACR)) {
                     blk_start_addr = (dword *)(((struct crrb *)rb_ptr) -> cr_u_buf);
                     write_single_blk_NB(idx, blk_start_addr);
		     }
		else {
		/* I002 End */
                     num_blks = ((struct vcrb *)rb_ptr) -> vc_nblks;
                     blk_start_addr = (dword *)(((struct vcrb *)rb_ptr) ->vc_bufp);
                     if (num_blks == 1)
                         write_single_blk_NB(idx, blk_start_addr);
                     else
                         write_multi_blk_NB(idx, num_blks, blk_start_addr);
		     }
                name_table_struct[idx].next_idx = 0xff;
                break;
          case SUB_DG :
                num_blks = ((struct drb *)rb_ptr) -> dr_nblks;
                blk_start_addr = (dword *)(((struct drb *)rb_ptr)->dr_bufp);
                if (num_blks == 1)
                     write_single_blk_NB(idx, blk_start_addr);
                else
                     write_multi_blk_NB(idx, num_blks, blk_start_addr);
                name_table_struct[idx].next_idx = 0xff;
                break;
#ifdef EDL
          case SUB_EDL :
	    switch (((crbh *)rb_ptr) -> c_opcode)
	    {
	      case OP_EDL_RAWTRAN:
                num_blks = 1;
                blk_start_addr =
			(dword *)(((struct tranrb *)rb_ptr)->tr_buffer_ptr);
		do
		{
                    write_single_blk_NB(idx, blk_start_addr++);
		    if (--num_blks > 0)
		    {
		        nidx = get_free_table_entry();
		        if (nidx == 0xff)
		        {
		            cmn_err(CE_WARN, "No free name table entry\n");
			    break;
		        }
                        name_table_struct[idx].next_idx = nidx;
		        idx = nidx;
		    }
		}
		while (num_blks > 0);
                name_table_struct[idx].next_idx = 0xff;
                break;

	      case OP_EDL_RAWRECV:
                num_blks = ((struct postrb *)rb_ptr)->num_blks;
                blk_start_addr =
			(dword *)(((struct postrb *)rb_ptr)->buffer_ptr);
		do
		{
                    write_single_blk_NB(idx, blk_start_addr++);
		    if (--num_blks > 0)
		    {
		        nidx = get_free_table_entry();
		        if (nidx == 0xff)
		        {
		            cmn_err(CE_WARN, "No free name table entry\n");
			    break;
		        }
                        name_table_struct[idx].next_idx = nidx;
		        idx = nidx;
		    }
		}
		while (num_blks > 0);
                name_table_struct[idx].next_idx = 0xff;
                break;
	    }
            break;
#endif
#ifdef NS
          case SUB_NS :
                blk_start_addr = (dword *)(((struct nsrb *)rb_ptr)->ns_nabufp);
                write_single_blk_NB(idx, blk_start_addr);
		if ((nidx = get_free_table_entry()) == 0xff)
		{
			cmn_err(CE_WARN, "No free name table entry\n");
			break;
		}
                name_table_struct[idx].next_idx = nidx;
		idx = nidx;
                blk_start_addr = (dword *)(((struct nsrb *)rb_ptr)->ns_vabufp);
                write_single_blk_NB(idx, blk_start_addr);
		if ((nidx = get_free_table_entry()) == 0xff)
		{
			cmn_err(CE_WARN, "No free name table entry\n");
			break;
		}
		name_table_struct[idx].next_idx = nidx;
		idx = nidx;
                blk_start_addr = (dword *)(((struct nsrb *)rb_ptr)->ns_exbufp);
                write_single_blk_NB(idx, blk_start_addr);
                name_table_struct[idx].next_idx = 0xff;
                break;
          case SUB_NMF :
                blk_start_addr = (dword *)(((struct nmfrb *)rb_ptr)->nmf_respbufp);
                write_single_blk_NB(idx, blk_start_addr);
		if ((nidx = get_free_table_entry()) == 0xff)
		{
			cmn_err(CE_WARN, "No free name table entry\n");
			break;
		}
                name_table_struct[idx].next_idx = nidx;
		idx = nidx;
                blk_start_addr = (dword *)(((struct nmfrb *)rb_ptr)->nmf_cmdbufp);
                write_single_blk_NB(idx, blk_start_addr);
                name_table_struct[idx].next_idx = 0xff;
                break;
#endif
      }
/***
	cmn_err(CE_NOTE, "Registered name %d idx ", idx);
***/
#ifdef EDL
      return(oidx);
#else
      return(idx);
#endif

}

dword Xlate_name(name)
dword name;
{
     BYTE idx;
     WORD offset, totlen, lensofar;
     NTABLE *name_ptr;
     BLKADR *blk_ptr;
     
     /* This routine is called to translate the encoded address into
     real dword addresses */

     idx = *((BYTE *)&name + 2);
     offset = (WORD)name ;
     name_ptr = &name_table_struct[idx];
     if (name_ptr -> num_blks == 1)
          return (name_ptr -> blk_type.original_buf_addr + offset);
     else
     {
          /* Indicates multi block case in VC or DG subsystem.
          This case a rare occurrence.
          Traverse the block pointed to by
               name_table_struct(idx).multi_blk_ptr and figure out the
               correct address. */
          blk_ptr = (BLKADR *) (name_ptr -> blk_type.multi_blk_ptr);
          lensofar = 0;          
          totlen = *(WORD *)(blk_ptr -> blk_len);
          for(;;)
          {
               if ( (offset >= lensofar) && (offset < totlen) )
                    return(*(dword *)(blk_ptr -> blk_addr) + offset - lensofar);
               else
               {
                    blk_ptr ++;
                    lensofar = totlen;
                    totlen = *(WORD *)(blk_ptr -> blk_len) + totlen;
               }
          }
     }
}

unsigned char de_register(index)
unsigned char index;
{
     NTABLE    *name_ptr;
     BYTE idx;

     name_ptr = &name_table_struct[index];
     if (name_ptr -> num_blks == 1)
     {
          for(;;)
          {
               *(name_ptr -> original_blk_ptr) =
                         name_ptr -> blk_type.original_buf_addr;
               idx = name_ptr -> next_idx;
               reset_name_entry (index);
               index = idx;
               if (index == 0xff) break;
#ifdef EDL
     	       name_ptr = &name_table_struct[index];
#endif
          }
     }
               
     else  /* VC or DG subsystem with multi block buffers */
     {
	  cmn_err(CE_NOTE, "Deallocating multi block\n");
	  /*****
          Restore all the original addresses pointed to by
               multi_blk_ptr 
          movb ((char *)(name_ptr -> original_blk_ptr),
               name_ptr -> blk_type.multi_blk_ptr,
               (name_ptr -> num_blks * 6));
          deallocate(name_ptr -> blk_type.multi_blk_ptr);
          release_name_entry (index);  *****/
     }
     return (0);

}          

write_single_blk_NB(idx, blk_start_addr)
BYTE idx;
dword *blk_start_addr;
{
     NTABLE *name_ptr;

     name_ptr = &name_table_struct[idx];
     name_ptr -> num_blks = 1;
     name_ptr -> original_blk_ptr = blk_start_addr;
     name_ptr -> blk_type.original_buf_addr = *blk_start_addr;
     name_ptr -> next_idx = 0xff;
     *blk_start_addr = encode(idx, 0);

}

write_multi_blk_NB(idx, num_blks, blk_start_addr)
BYTE idx;
WORD num_blks;
dword *blk_start_addr;
{
     BLKADR *blk_ptr;
     NTABLE *name_ptr;
     WORD   buf_len, offset, i;

cmn_err(CE_NOTE, "calling multi_blk case\n");
     name_ptr = &name_table_struct[idx];
     blk_ptr = (BLKADR *)blk_start_addr;
     name_ptr -> num_blks = num_blks;
     buf_len = num_blks * 6;
/******
     name_ptr -> blk_type.multi_blk_ptr = (char *)allocate(buf_len);
     memcpy((char *)blk_start_addr, name_ptr -> blk_type.multi_blk_ptr,
          buf_len);
******/
     offset = 0;
     for (i=0; i < num_blks; i++, blk_ptr++)
     {
          *(dword *)(blk_ptr -> blk_addr) = (dword)encode(idx, offset);
          offset +=  *(WORD *)(blk_ptr -> blk_len);
     }
}

dword encode(index, offset)
BYTE index;
ushort offset;
{
          BYTE bytes[4];

    *(ushort *)bytes = offset;
    bytes[2] = index;
    bytes[3] = 0;
    bytes[3] = 0x80 | ics_myslotid();

    return (*(dword *)&bytes[0]);
}

BYTE get_free_table_entry()
{
     BYTE start_index, temp_index, wrap_around = 0;
     NTABLE *name_ptr;

     temp_index = cur_index + 1;
     if (temp_index >= MAX_INDEX)
          temp_index = 0;
     start_index = temp_index;
     name_ptr = &name_table_struct[temp_index];

     /****  Ensure Mutual Exclusion  ****/

     for (;;)
     {
          if (name_ptr -> in_use == FALSE)
          {
               name_ptr -> in_use = TRUE;
               cur_index = temp_index;
               /****   Release the lock   ****/
               return (temp_index);
          }
          else
          {
               temp_index ++;
               name_ptr ++;
               if (temp_index > start_index && wrap_around == TRUE)
               {
                    /****  Release the lock   ***/
                    return (0xff); /**  free entry not found **/
               }
               if (temp_index >= MAX_INDEX)
               {
                    temp_index = 0;
                    name_ptr = &name_table_struct[0];
                    wrap_around = TRUE;
               }
          }
     }
}

reset_name_entry(index)
BYTE index;
{
	NTABLE *name_ptr;

	name_ptr = &name_table_struct[index];
	name_ptr->in_use = FALSE;
	name_ptr->next_idx = 0xff;
}
