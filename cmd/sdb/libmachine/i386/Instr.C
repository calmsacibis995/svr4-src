#ident	"@(#)sdb:libmachine/i386/Instr.C	1.8"
#include 	<stdio.h>
#include	<string.h>
#include	"Process.h"
#include 	"Instr.h"
#include	"dis.h"
#include	"Reg1.h"
#include	"Interface.h"

extern int debugflag;

int errlev = 0;			/* to keep track of errors encountered during	*/
						/* the disassembly, probably due to being out	*/
						/* of sync.					*/

static	char	operand[3][OPLEN];	/* to store operands as they	*/
									/* are encountered		*/
static	char	symarr[3][OPLEN];
static  char    *overreg;	/* save the segment override register if any    */
static  int 	data16;		/* 16- or 32-bit data */
static  int		addr16;		/* 16- or 32-bit addressing */
static  char	buf[256];
extern  void	printline(char *);


/*
 *	void get_modrm_byte (mode, reg, r_m)
 *
 *	Get the byte following the op code and separate it into the
 *	mode, register, and r/m fields.
 * Scale-Index-Bytes have a similar format.
 */

void
Instr::get_modrm_byte(unsigned *mode, unsigned *reg, unsigned *r_m)
{
	curbyte = get_byte() & 0377;

	*r_m = curbyte & 0x7; /* r/m field from curbyte */
	*reg = curbyte >> 3 & 0x7; /* register field from curbyte */
	*mode = curbyte >> 6 & 0x3; /* mode field from curbyte */
}

/*
 *	void check_override (opindex)
 *
 *	Check to see if there is a segment override prefix pending.
 *	If so, print it in the current 'operand' location and set
 *	the override flag back to false.
 */

void
Instr::check_override(int opindex)
{
	if (overreg) {
		(void) ::sprintf(operand[opindex],"%s",overreg);
		(void) ::sprintf(symarr[opindex],"%s",overreg);
	}
	overreg = (char *) 0;
}

/*
 *	void displacement (no_bytes, opindex, value)
 *
 *	Get and print in the 'operand' array a one, two or four
 *	byte displacement from a register.
 */

void
Instr::displacement(int no_bytes,int opindex,long  *value)
{
	char	temp[(NCPS*2)+1];

	getbytes(no_bytes, temp, value);
	check_override(opindex);
	(void) ::sprintf(operand[opindex],"%s%s",operand[opindex],temp);
}
void
Instr::get_operand(unsigned mode, unsigned r_m,int wbit,int opindex)
{
	extern	char	dispsize16[8][4];	/* tables.c */
	extern	char	dispsize32[8][4];
	extern	char	*regname16[4][8];
	extern	char	*regname32[4][8];
	extern	char	*indexname[8];
	extern	char 	*REG16[8][2];	  /* in tables.c */
	extern	char 	*REG32[8][2];	  /* in tables.c */
	extern	char	*scale_factor[4];	  /* in tables.c */
	int dispsize;   /* size of displacement in bytes */
	long dispvalue;  /* value of the displacement */
	char *resultreg; /* representation of index(es) */
	char *format;   /* output format of result */
	int s_i_b;      /* flag presence of scale-index-byte */
	unsigned ss;    /* scale-factor from opcode */
	unsigned index; /* index register number */
	unsigned base;  /* base register number */
	char indexbuffer[16]; /* char representation of index(es) */

	/* if symbolic representation, skip override prefix, if any */
	check_override(opindex);

	/* check for the presence of the s-i-b byte */
	if (r_m==REG_ESP && mode!=REG_ONLY && !addr16) {
		s_i_b = TRUE;
		get_modrm_byte(&ss, &index, &base);
	}
	else
		s_i_b = FALSE;

	if (addr16)
		dispsize = dispsize16[r_m][mode];
	else
		dispsize = dispsize32[r_m][mode];

	if (s_i_b && mode==0 && base==REG_EBP) dispsize = 4;

	if (dispsize != 0)
		displacement(dispsize, opindex, &dispvalue);

	if (s_i_b) {
		register char *basereg = regname32[mode][base];
		(void) ::sprintf(indexbuffer, "%s%s,%s", basereg,
			indexname[index], scale_factor[ss]);
		resultreg = indexbuffer;
		format = "%s(%s)";
	}
	else { /* no s-i-b */
		if (mode == REG_ONLY) {
			format = "%s%s";
			if (data16)
				resultreg = REG16[r_m][wbit] ;
			else
				resultreg = REG32[r_m][wbit] ;
		}
		else { /* Modes 00, 01, or 10 */
			if (addr16)
				resultreg = regname16[mode][r_m];
			else
				resultreg = regname32[mode][r_m];
			if (r_m ==REG_EBP && mode == 0) { /* displacement only */
				format = "%s";
			}
			else { /* Modes 00, 01, or 10, not displacement only, and no s-i-b */
				format = "%s(%s)";
			}
		}
	}
	(void) ::sprintf(operand[opindex],format,operand[opindex], resultreg);
}

/*
** getbytes() reads no_bytes from a file and converts them into destbuf.
** A sign-extended value is placed into destvalue if it is non-null.
*/
void
Instr::getbytes(int no_bytes, char *destbuf,long *destvalue)
{
	int  j;
	char *format;
	unsigned long shiftbuf = 0;

	for (j=0; j<no_bytes; j++) {
		get_byte();
		shiftbuf |= (long) curbyte << (8*j);
	}

	switch(no_bytes) {
		case 1:
			format = "0x%.2lx";
			if (destvalue)
				*destvalue =
					(shiftbuf & 0x80) ?
					shiftbuf | ~0xffL : shiftbuf & 0xffL;
			break;
		case 2:
			format = "0x%.4lx";
			if (destvalue) *destvalue = (short) shiftbuf;
			break;
		case 4:
			format = "0x%.8lx";
			if (destvalue) *destvalue = shiftbuf;
			break;
	}
	::sprintf(destbuf,format,shiftbuf);
}

/*
 *	void imm_data (no_bytes, opindex)
 *
 *	Determine if 1, 2 or 4 bytes of immediate data are needed, then
 *	get and print them.
 */

void
Instr::imm_data(int no_bytes,int opindex)
{
	long value;
	int len = strlen(operand[opindex]);

	operand[opindex][len] = '$';
	getbytes(no_bytes, &operand[opindex][len+1], &value);
}


/*
 *	get_opcode (high, low)
 *	Get the next byte and separate the op code into the high and
 *	low nibbles.
 */

unsigned short
Instr::get_opcode(unsigned * high, unsigned  * low)
{
	curbyte = get_byte() & 0377;
	*low = curbyte & 0xf;  				/* ----xxxx low 4 bits */
	*high = curbyte >> 4 & 0xf;  		/* xxxx---- bits 7 to 4 */
	opcode = byte[0] & 0377;
	return curbyte;
}

//
// get text, replace breakpints with original text.
//
int
Instr::get_text_nobkpt( Iaddr start)
{
	
	if ( process == 0 )
		return 0;
	else if ( start == 0 )
		return 0;
	else {
		char *oldtext;
		int next, cnt; 

		addr = start;
		if (process->read(addr, NLINE, (char *) byte ) <= 0 )
			return 0;
		byte_cnt = 0;
		//
		// if there are any breakpoints in byte[] replace them
		// with the original text
		//
		cnt = 0;
		while ( cnt < NLINE) {
			oldtext = process->text_nobkpt(start + cnt);
			if ( oldtext  != 0 )  
				byte[cnt] = *oldtext;
			cnt++;
		}
		return 1;
	}
	
}
//
// 	get_text reads the NLINE no of bytes from the address addr
//	NLINE is the max. no of characters in an assembly instruction
//
int
Instr::get_text(Iaddr start)
{
	if ( process == 0 )
		return 0;
	else if ( start == 0 )
		return 0;
	else {
		addr = start;
		byte_cnt = 0;
		return ( process->read(addr, NLINE, (char *) byte ) > 0 );
	}
}

Instr::Instr( Process *p)
{
	process = p;
}

Instr::make_mnemonic(char * mnemonic)
{
	if (mnemonic != 0 ){
		::strcpy(buf,"\t");
		::strcat(buf, mnemonic);
	}
	else{
		printe("Interal error: ");
		printe("null first argument to the Instr::make_mnemonic()\n");
		return 0;
	}
}

//
//	if the current instr is CALL, return the address of the following instr
//

Iaddr
Instr::retaddr(Iaddr addr)
{
	unsigned op1, op2;
	unsigned short op;

	if (get_text(addr) == 0) {
		printe("cannot get_text from %#x\n",addr);
		return 0;
	}
		
	op =  byte[0];
	if ( (op != CALL) && (op != LCALL) && (op != ICALL) )  {
		return 0;
	}
	else {
		byte_cnt = 0;
		deasm1(addr);
		return ( addr + (Iaddr) byte_cnt );
	}
}

//
// is last instruction "CALL" ?
//
int
Instr::iscall(Iaddr addr)
{	

	unsigned short op;

	for (int i = 3; i <= 7; i += 2) // repeat for 3, 5, 7 bytes
	{
		if (get_text_nobkpt(addr - i) == 0)
			continue;
		op =  byte[0];
		if ( (op == CALL) || (op == LCALL) || (op == ICALL) )
			return 1;
	}
	return 0;
}
	

Iaddr
Instr::next_instr( Iaddr addr )
{
	byte_cnt = 0;
	if ( deasm1(addr)  == 0 ) 
		return 0;
	return ( addr + (Iaddr) byte_cnt );
}

int 
Instr::is_bkpt( Iaddr addr )
{
	unsigned low, high;

	if (get_text(addr) == 0) {
		printe("cannot get_text from %#x\n",addr);
		return 0;
	}
	if ( get_opcode(&high, &low) == 0xCC )
		return 1;
	else
		return 0;
}

char
Instr::get_byte()
{
	char ch;
	
	ch = byte[byte_cnt++];
	curbyte = ch & 0377;
	return ch;
}

/*
	Instr::compoff() will compute the location to which control is to be 
	transferred. 'lng' is the number indicating the jump amount
	(already in proper form, meaning masked and negated if necessary)
	and temp is a character array which already has the actual
	jump amount. The result computed here will go at the end of 'tmp'
*/

char *
Instr::compoff(Iaddr lng, char *temp)
{

	Symbol sym;
	char * name;

    lng += addr + byte_cnt;
    ::sprintf(temp,"%s <%lx>",temp,lng);
	sym = process->find_entry(lng) ;
	if ( ! ( name = process->symbol_name( sym ) ) )
		name = 0;
	return name;
}

//
// if at a breakpoint pc is point to the next instruction.
// it needs to be pointing at the location of the 'int 3' instruction
//
Iaddr
Instr::adjust_pc()
{
    Iaddr pc, newpc;
    Itype data;

    pc = process->getreg(REG_PC);
    newpc = pc - 1;
    if ( get_text(newpc) && (byte[0] == 0xCC) ) {
        data.iaddr = newpc;
        process->writereg(REG_PC, Saddr, data ); 
        return newpc;
    }
    return pc;
}

void
printline(char *ch)
{
/*
	char temp[NLINE];
	int length;

	length = ::strlen(ch);

	if ( length > NLINE ){
		::printe("the length of the assembly instr is too much\n");
		length = NLINE;
	}
	(void) ::strncpy(temp, ch, length);
	temp[length] = '\0';
*/
}


//
// get number of arguments
//
int 
Instr::nargbytes(Iaddr addr)
{

	if ( ! get_text_nobkpt(addr) ) {		 // read instr into bye[]
        	printe("Can't get_text address 0x%x\n",addr);
        	return 0;
	}


	if ( (byte[0] == ADDLimm8) && ( byte[1] == toESP) ) {
		return 	( byte[2] );
	}
	//
	// popl %ecx might be used to adjust the stack after the call
	//
	else if ( byte[0] == POPLecx )
		return 4;
	else
		return 0;
	
}
//
// look for function prolog.
// If skipflag is set, return the address of the first instruction past the
// prolog  if there is one , or pc unchanged if there isn't.
// If skipflag is not set, return non zero if there is a prolog,zero if there isn't.
//
Iaddr
Instr::fcn_prolog(Iaddr pc, short skipflag)
{
	Iaddr *addrptr;
	Iaddr initpc = pc;
	Iaddr retval = pc;
	short jmp_to_prolog = 0;

	if ( ! get_text_nobkpt(pc) ) 		 // read instr into byte[]
        	printe("Can't get_text address 0x%x\n",pc);
	if (byte[0] == JMPrel8) {
		jmp_to_prolog = 1;
		retval += 2;
		pc = retval + byte[1];
		get_text_nobkpt(pc);
	}
	else if (byte[0] == JMPrel32) {
		jmp_to_prolog = 1;
		addrptr = (Iaddr*) &byte[1];
		retval += 5;
		pc = retval + (*addrptr);
		get_text_nobkpt(pc);
	}
	
	if ( (byte[0] == PUSHLebp) &&
	     (byte[1] == MOVLrr)   &&
	     (byte[2] == ESPEBP) ) {
		int nopcnt = 0;

		if (jmp_to_prolog == 0) 
			return ( retval + 3 );
		//
		// if there is a jump to prolog, it might be
		// followed by NOPs for alighnment. skip them.
		//
		get_text_nobkpt(retval);
		while ( byte[nopcnt] == NOP ) 
			nopcnt++;
		return (retval + nopcnt);
	}
	else if (skipflag)
		return initpc;
	else
		return 0;
}

//
// look for function prolog.
// If skipflag is set, return the address of the first instruction past the
// prolog  if there is one , or pc unchanged if there isn't.
// If skipflag is not set, return non zero if there is a prolog,zero if there isn't.
//
// 	for register variables, if there are more than 3 register variables
// 	then compiler puts the first three variables in the registers
//	and rest of the variables on the stack. save_reg here is used for
//	storing the information about register variables. save_reg[1-3]
//	is used for storing the registers(edi, ebx, esi) saved on stack.
//	save_reg[0] contains the offset from esp corresponding to the
//	remaining no. of register variables. 
//
Iaddr
Instr::fcn_prolog_sr(Iaddr pc, short skipflag, RegRef save_reg[])
{
	Iaddr *addrptr;
	Iaddr initpc = pc;
	Iaddr retval = pc;
	int offset = 0;
	int idx = 0;
	int i = 0, k;

	if ( ! get_text_nobkpt(pc) ) { 		 // read instr into byte[]
        	printe("Can't get_text address 0x%x\n",pc);
		return 0;
	}
	if (byte[0] == JMPrel8) {
		retval += 2;
		pc = retval + byte[1];
		get_text_nobkpt(pc);
	}
	else if (byte[0] == JMPrel32) {
		addrptr = (Iaddr*) &byte[1];
		if (debugflag)
			printf("jmp operand = 0x%x\n", *addrptr);
		retval += 5;
		pc = retval + (*addrptr);
		get_text_nobkpt(pc);
	}
	
	if ( (byte[0] == PUSHLebp) &&
	     (byte[1] == MOVLrr)   &&
	     (byte[2] == ESPEBP) ) {
			idx = 3;
			if (  byte[3]  == PUSHLeax ) {
				 offset += 4;
				 idx++; 
			}
			else if ( byte[3] == SUBLimm8 ) {
				 offset +=  byte[5];
				 idx += 3;
			}
			else if ( byte[3] == SUBLimm32 ) {
				 addrptr = (Iaddr *) &byte[5];
				 offset  += *addrptr;
				 idx += 6;
			}
		
		
			save_reg[0] = offset;
			//
			// get saved registers. 
			// can be edi, esi and ebx
			//
			k = 1;
			for (i = 0; i < 3; i++ )
				if ( byte[ idx+i ] == PUSHLedi ) 
						save_reg[k++] = REG_EDI;
				else if ( byte[ idx+i ] == PUSHLesi ) 
						save_reg[k++] = REG_ESI;
				else if ( byte[ idx+i ] == PUSHLebx ) 
						save_reg[k++] = REG_EBX;
				else 
					break;

			//
			// skip NOPs
			//
			int nopcnt = 0;

			if (!get_text_nobkpt(retval)) {
				printe("cannot get_text %#x\n", retval);
				return 0;
			}
			while ( byte[nopcnt] == NOP ) 
				nopcnt++;
			return (retval + nopcnt);
	}
	else if (skipflag)
		return initpc;
	else
		return 0;
}


/*
 *	Instr::deasm(int ) 
 *
 *	disassemble a text section
 */

char  	mneu[1028];		// array to store mnemonic code for output
	
char *
Instr::deasm(Iaddr pcaddr, int symbolic)
{
/* the following arrays are contained in tables.c   */
    extern  struct  instable    distable[16][16];
    extern  struct  instable    op0F[13][16];
    extern  struct  instable    opFP1n2[8][8];
	extern  struct  instable    opFP3[8][8];
    extern  struct  instable    opFP4[4][8];
    extern  struct  instable    opFP5[8];

	extern  char    *REG16[8][2];
    extern  char    *REG32[8][2];
    extern  char    *SEGREG[6];
    extern  char    *DEBUGREG[8];
    extern  char    *CONTROLREG[8];
    extern  char    *TESTREG[8];


	struct instable *dp;
	unsigned mode, reg, r_m;
	int wbit, vbit;

	/* nibbles of the opcode */
	unsigned opcode1, opcode2, opcode3, opcode4, opcode5;

	char 	*reg_name, *sym_name;
	char 	mnemonic[OPLEN];
	extern	char  mneu[];		// array to store mnemonic code for output
	char	temp[1028];
	int 	got_modrm_byte;
	unsigned short tmpshort;

	mnemonic[0] = '\0';
	mneu[0] = '\0';
	operand[0][0] = '\0';
	operand[1][0] = '\0';
	operand[2][0] = '\0';
	symarr[0][0] = '\0';
	symarr[1][0] = '\0';
	symarr[2][0] = '\0';
	overreg = (char *) 0;
	data16 = addr16 = 0;
	
	if ( get_text_nobkpt(pcaddr) == 0 ) {
		printe("cannot get_text from %#x\n",pcaddr);
		return 0;
	}

	Symbol sym, sym1;
	Iaddr  offset;
	char *name;
	
	sym = process->find_entry(pcaddr);
	offset = pcaddr - ( (Iaddr) sym.pc(an_lopc) );
	name = process->symbol_name( sym );

	if ( name != 0 )
		::sprintf(mneu, "\t(%s+%d:)\t  ", name, offset);
	else
		::sprintf(mneu, "\t(..............)\t  ");

	/*
	** As long as there is a prefix, the default segment register,
	** addressing-mode, or data-mode in the instruction will be overridden.
	** This may be more general than the chip actually is.
	*/
	for(;;) {
		get_opcode(&opcode1, &opcode2);
		dp = &distable[opcode1][opcode2];

		if ( dp->adr_mode == PREFIX ) 
			::strcat(mnemonic,dp->name);
		else if ( dp->adr_mode == AM ) addr16 = !addr16;
		else if ( dp->adr_mode == DM ) data16 = !data16;
		else if ( dp->adr_mode == OVERRIDE ) overreg = dp->name;
		else break;
	}

	/* some 386 instructions have 2 bytes of opcode before the mod_r/m */
	/* byte so we need to perform a table indirection.	      */
	if (dp->indirect == (struct instable *) op0F) {
		get_opcode(&opcode4,&opcode5);
		if (opcode4 > 12)  {
			::sprintf(mneu,"***** Error - bad opcode\n");
			errlev++;
			return (char *) 0;
		}
		dp = &op0F[opcode4][opcode5];
	}

	got_modrm_byte = 0;
	if (dp->indirect != TERM) {
		/* This must have been an opcode for which several
		 * instructions exist.  The opcode3 field further get_texts
		 * the instruction.
		 */
		got_modrm_byte = 1;
		get_modrm_byte(&mode, &opcode3, &r_m);
		/*
		 * get_text 287 instructions (D8-DF) from opcodeN
		 */
		if (opcode1 == 0xD && opcode2 >= 0x8) {
			/* instruction form 5 */
			if (opcode2 == 0xB && mode == 0x3 && opcode3 == 4)
				dp = &opFP5[r_m];
			else if (opcode2 == 0xB && mode == 0x3 && opcode3 > 4) {
				::sprintf(mneu,"***** Error - bad opcode\n");
				errlev++;
				return (char *) 0;
			}
			/* instruction form 4 */
			else if (opcode2==0x9 && mode==0x3 && opcode3 >= 4)
				dp = &opFP4[opcode3-4][r_m];
			/* instruction form 3 */
			else if (mode == 0x3)
				dp = &opFP3[opcode2-8][opcode3];
			/* instruction form 1 and 2 */
			else dp = &opFP1n2[opcode2-8][opcode3];
		}
		else
			dp = dp->indirect + opcode3;
				/* now dp points the proper subget_text table entry */
	}

	if (dp->indirect != TERM) {
		::sprintf(mneu,"***** Error - bad opcode\n");
		errlev++;
	 	return (char *) 0; 
	}

	/* print the mnemonic */
	if ( dp->adr_mode != CBW  && dp->adr_mode != CWD ) {
		(void) ::strcat(mnemonic,dp->name);  
		if (dp->suffix)
			(void) ::strcat(mnemonic, (data16? "w" : "l") );
		(void) ::sprintf(mneu, (strlen(mnemonic)<7 ? "%s %-7s" : "%s %-7s "),
		               mneu, mnemonic);
		make_mnemonic( mneu );
	}

	/*
	 * Each instruction has a particular instruction syntax format
	 * stored in the disassembly tables.  The assignment of formats
	 * to instructions was made by the author.  Individual formats
	 * are explained as they are encountered in the following
	 * switch construct.
	 */

	switch (dp->adr_mode) {

// movsbl movsbw (0x0FBE) or movswl (0x0FBF)
// movzbl movzbw (0x0FB6) or mobzwl (0x0FB7) 
// wbit lives in 2nd byte, note that operands are different sized 
	case MOVZ:
		if ( ! got_modrm_byte )
			get_modrm_byte(&mode, &reg, &r_m);
		if ( data16 )
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];

		wbit = WBIT(opcode5);
		data16 = 1;
		get_operand(mode, r_m, wbit, 0);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		::printline(mneu); /*dbg*/
		::sprintf(mneu, "%s\t[ %s, %s ]",mneu, operand[0], reg_name );
		return mneu;

	/* imul instruction, with either 8-bit or longer immediate */
	case IMUL:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
		/* opcode 0x6B for byte, sign-extended displacement, 0x69 for word(s)*/
		imm_data( OPSIZE(data16,opcode2 == 0x9), 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) ::sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],operand[1],reg_name);
		::printline(mneu); /*dbg*/
		::sprintf( mneu,"%s\t[ %s, %s, %s ]",mneu, operand[0], operand[1], reg_name);
		return mneu;

	/* memory or register operand to register, with 'w' bit	*/
	case MRw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu, operand[0],reg_name);
		return mneu;

	/* register to memory or register operand, with 'w' bit	*/
	/* arpl happens to fit here also because it is odd */
	case RMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu,reg_name,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]", mneu,reg_name,operand[0]);
		return mneu;

	/* Double shift. Has immediate operand specifying the shift. */
	case DSHIFT:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		imm_data(1, 0);
		::sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],reg_name,operand[1]);
		::sprintf(mneu,"%s\t[ %s, %s, %s]",mneu,operand[0],reg_name,operand[1]);
		return mneu;

	/* Double shift. With no immediate operand, specifies using %cl. */
	case DSHIFTcl:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		::sprintf(mneu,"%s%s,%s",mneu,reg_name,operand[0]);
		::printline(mneu); /*dbg*/
		::sprintf(mneu,"%s\t[ %s, %s ]",mneu,reg_name,operand[0]);
		return mneu;

	/* immediate to memory or register operand */
	case IMlw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 1);
		/* A long immediate is expected for opcode 0x81, not 0x80 nor 0x83 */
		imm_data(OPSIZE(data16,opcode2 == 1), 0);
		::sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		::printline(mneu); /*dbg*/
		::sprintf(mneu,"%s\t[ %s, %s ]",mneu,operand[0],operand[1]);
		return mneu;

	/* immediate to memory or register operand with the	*/
	/* 'w' bit present					*/
	case IMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 1);
		imm_data(OPSIZE(data16,wbit), 0);
		::sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		::printline(mneu); /*dbg*/
		::sprintf(mneu,"%s\t[ %s, %s ]",mneu, operand[0],operand[1]);
		return mneu;

	/* immediate to register with register in low 3 bits	*/
	/* of op code						*/
	case IR:
		wbit = opcode2 >>3 & 0x1; /* w-bit here (with regs) is bit 3 */
		reg = REGNO(opcode2);
		imm_data( OPSIZE(data16,wbit), 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu, operand[0],reg_name);
		return mneu;

	/* memory operand to accumulator			*/
	case OA:
		wbit = WBIT(opcode2);
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( data16 ? REG16 : REG32 )[0][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu,operand[0],reg_name);
		return mneu;

	/* accumulator to memory operand			*/
	case AO:
		wbit = WBIT(opcode2);
		
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( addr16 ? REG16 : REG32 )[0][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu, reg_name, operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]", mneu, reg_name, operand[0]);
		return mneu;

	/* memory or register operand to segment register	*/
	case MS:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],SEGREG[reg]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu,operand[0],SEGREG[reg]);
		return mneu;

	/* segment register to memory or register operand	*/
	case SM:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,SEGREG[reg],operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu,SEGREG[reg],operand[0]);
		return mneu;

	/* rotate or shift instrutions, which may shift by 1 or */
	/* consult the cl register, depending on the 'v' bit	*/
	case Mv:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		(void) ::sprintf(mneu,"%s%s%s",mneu, reg_name, operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s%s ]",mneu, reg_name, operand[0]);
		return mneu;

	/* immediate rotate or shift instrutions, which may or */
	/* may not consult the cl register, depending on the 'v' bit	*/
	case MvI:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		imm_data(1,1);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		(void) ::sprintf(mneu,"%s%s,%s%s",mneu,operand[1], reg_name, operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s%s ]",mneu, operand[1], reg_name, operand[0]);
		return mneu;

	case MIb:
		get_operand(mode, r_m, LONGOPERAND, 0);
		imm_data(1,1);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[1], operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"$s\t[ %s, %s ]",mneu,operand[1], operand[0]);
		return mneu;

	/* single memory or register operand with 'w' bit present*/
	case Mw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu, operand[0]);
		return mneu;

	/* single memory or register operand			*/
	case M:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/

		char *glbl = operand[0];
		char *glb_name = 0;

		sym1 = process->find_entry( ( (Iaddr )strtol(glbl, &glbl,0)) );
		if ( ! sym1.isnull() )
			glb_name = process->symbol_name(sym1);
		if ( ( glb_name != 0) && ( *glbl != '(' ) ) 
			(void) ::sprintf(mneu,"%s\t[ %s ]",mneu, glb_name ); 
		else
			(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,operand[0]);
		return mneu;

	case SREG: /* special register */
		get_modrm_byte(&mode, &reg, &r_m);
		vbit = 0;
		switch (opcode5) {
		case 2:
			vbit = 1;
			/* fall thru */
		case 0: 
			reg_name = CONTROLREG[reg];
			break;
		case 3:
			vbit = 1;
			/* fall thru */
		case 1:
			reg_name = DEBUGREG[reg];
			break;
		case 6:
			vbit = 1;
			/* fall thru */
		case 4:
			reg_name = TESTREG[reg];
			break;
		}
		strcpy(operand[0], REG32[r_m][1]);

		if (vbit)
		{
			strcpy(operand[0], reg_name);
			reg_name = REG32[r_m][1];
		}
		
		(void) ::sprintf(mneu, "%s%s,%s",mneu, reg_name, operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu, reg_name, operand[0]);
		return mneu;

	/* single register operand with register in the low 3	*/
	/* bits of op code					*/
	case R:
		reg = REGNO(opcode2);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) ::sprintf(mneu,"%s%s",mneu,reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,reg_name);
		return mneu;

	/* register to accumulator with register in the low 3	*/
	/* bits of op code, xchg instructions                   */
	case RA: {
		char *eprefix;
		reg = REGNO(opcode2);
		if (data16) {
			eprefix = "";
			reg_name = REG16[reg][LONGOPERAND];
		}
		else {
			eprefix = "e";
			reg_name = REG32[reg][LONGOPERAND];
		}
		(void) ::sprintf(mneu,"%s%s,%%%sax", mneu,reg_name,eprefix);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %%%sax ]",mneu,reg_name,eprefix);
		return mneu;
	}

	/* single segment register operand, with register in	*/
	/* bits 3-4 of op code					*/
	case SEG:
		reg = curbyte >> 3 & 0x3; /* segment register */
		(void) ::sprintf(mneu,"%s%s",mneu,SEGREG[reg]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,SEGREG[reg]);
		return mneu;

	/* single segment register operand, with register in	*/
	/* bits 3-5 of op code					*/
	case LSEG:
		reg = curbyte >> 3 & 0x7; /* long seg reg from opcode */
		(void) ::sprintf(mneu,"%s%s",mneu,SEGREG[reg]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,SEGREG[reg]);
		return mneu;

	/* memory or register operand to register		*/
	case MR:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu, operand[0],reg_name);
		return mneu;

	/* immediate operand to accumulator			*/
	case IA: {
		int no_bytes = OPSIZE(data16,WBIT(opcode2));
		switch(no_bytes) {
			case 1: reg_name = "%al"; break;
			case 2: reg_name = "%ax"; break;
			case 4: reg_name = "%eax"; break;
		}
		imm_data(no_bytes, 0);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0], reg_name) ;
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu, operand[0], reg_name) ;
		return mneu;
	}
	/* memory or register operand to accumulator		*/
	case MA:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		reg_name = ( data16 ? REG16 : REG32) [0][wbit];
		(void) ::sprintf(mneu,"%s%s,%s",mneu, operand[0], reg_name );
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]", mneu,operand[0], reg_name );
		return mneu;

	/* si register to di register				*/
	case SD:
		check_override(0);
		(void) ::sprintf(mneu,"%s%s(%%%ssi),(%%%sdi)",mneu,operand[0],
			addr16? "" : "e" , addr16? "" : "e");
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s(%%%ssi), (%%%sdi) ]",mneu, operand[0],
			addr16? "" : "e" , addr16? "" : "e");
		return mneu;

	/* accumulator to di register				*/
	case AD:
		wbit = WBIT(opcode2);
		check_override(0);
		reg_name = (data16 ? REG16 : REG32) [0][wbit] ;
		(void) ::sprintf(mneu,"%s%s,%s(%%%sdi)",mneu, reg_name, operand[0],
			addr16? "" : "e");
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s(%%%sdi) ]",mneu, reg_name, operand[0],
			addr16? "" : "e");
		return mneu;

	/* si register to accumulator				*/
	case SA:
		wbit = WBIT(opcode2);
		check_override(0);
		reg_name = (addr16 ? REG16 : REG32) [0][wbit] ;
		(void) ::sprintf(mneu,"%s%s(%%%ssi),%s",mneu,operand[0],
			addr16? "" : "e", reg_name);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s(%%%ssi), %s",mneu, operand[0],
			addr16? "" : "e", reg_name);
		return mneu;

	/* single operand, a 16/32 bit displacement		*/
	/* added to current offset by 'compoff'			*/
	case D:
		displacement(OPSIZE(data16,LONGOPERAND), 0, &lngval);
		sym_name = compoff(lngval, operand[1]);
		(void) ::sprintf(mneu,"%s+%s%s",mneu,operand[0],
			(lngval == 0) ? "" : operand[1]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu, 
			( sym_name == 0) ? operand[1] : sym_name );
		return mneu;

	/* indirect to memory or register operand		*/
	case INM:
		get_operand(mode, r_m, LONGOPERAND, 0);
		(void) ::sprintf(mneu,"%s*%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ *%s ]",mneu,operand[0]);
		return mneu;

	/* for long jumps and long calls -- a new code segment   */
	/* register and an offset in IP -- stored in object      */
	/* code in reverse order                                 */
	case SO:
		displacement(OPSIZE(addr16,LONGOPERAND), 1,&lngval);
		/* will now get segment operand*/
		displacement(2, 0,&lngval);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu,operand[0],operand[1]);
		return mneu;

	/* jmp/call. single operand, 8 bit displacement.	*/
	/* added to current EIP in 'compoff'			*/
	case BD:
		displacement(1, 0, &lngval);
		sym_name = compoff(lngval, operand[1]);
		(void) ::sprintf(mneu,"%s+%s%s",mneu, operand[0],
			(lngval == 0) ? "" : operand[1]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu, 
			( sym_name == 0) ? operand[1] : sym_name );
		return mneu;

	/* single 32/16 bit immediate operand			*/
	case I:
		imm_data(OPSIZE(data16,LONGOPERAND), 0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,operand[0]);
		return mneu;

	/* single 8 bit immediate operand			*/
	case Ib:
		imm_data(1, 0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu, operand[0]);
		return mneu;

	case ENTER:
		imm_data(2,0);
		imm_data(1,1);
		(void) ::sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s, %s ]",mneu,operand[0],operand[1]);
		return mneu;

	/* 16-bit immediate operand */
	case RET:
		imm_data(2,0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,operand[0]);
		return mneu;

	/* single 8 bit port operand				*/
	case P:
		check_override(0);
		imm_data(1, 0);
		(void) ::sprintf(mneu,"%s%s",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s ]",mneu,operand[0]);
		return mneu;

	/* single operand, dx register (variable port instruction)*/
	case V:
		check_override(0);
		(void) ::sprintf(mneu,"%s%s(%%dx)",mneu,operand[0]);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %s(%%dx) ]",mneu,operand[0]);
		return mneu;

	/* The int instruction, which has two forms: int 3 (breakpoint) or  */
	/* int n, where n is indicated in the subsequent byte (format Ib).  */
	/* The int 3 instruction (opcode 0xCC), where, although the 3 looks */
	/* like an operand, it is implied by the opcode. It must be converted */
	/* to the correct base and output. */
	case INT3:
/*		convert(3, temp, LEAD); */
		(void) ::sprintf(temp,"0x%x", 3);
		(void) ::sprintf(mneu,"%s$%s",mneu,temp);
		(void) ::sprintf(mneu,"%s\t[ %s ]" ,mneu,temp);
		return mneu;

	/* an unused byte must be discarded			*/
	case U:
		get_byte();
		return mneu;

	case CBW:
		if (data16)
			(void) strcat(mneu,"cbtw");
		else
			(void) strcat(mneu,"cwtl");
		::printline(mneu); /*dbg*/
		::sprintf( mneu,"%s\t[]",mneu );
		return mneu;

	case CWD:
		if (data16)
			(void) strcat(mneu,"cwtd");
		else
			(void) strcat(mneu,"cltd");
		::printline(mneu); /*dbg*/
		::sprintf( mneu,"%s\t[]", mneu );
		return mneu;

	/* no disassembly, the mnemonic was all there was	*/
	/* so go on						*/
	case GO_ON:
		::sprintf(mneu, "%s\t%s",mneu, "[]");
		return mneu;

	/* Special byte indicating the beginning of a 			*/
	/* jump table has been seen. The jump table addresses	*/
	/* will be printed until the address 0xffff which		*/
	/* indicates the end of the jump table is read.			*/
	case JTAB:
		(void) ::sprintf(mneu,"***JUMP TABLE BEGINNING***");
		printx("%s",mneu);
		addr = addr + (Iaddr) byte_cnt;
		process->read(addr, 1, (char *) byte);
		curbyte = byte[0]; 
		if (curbyte == FILL) {
			(void) ::sprintf(mneu,"FILL BYTE FOR ALIGNMENT");
			printx("%s\t",mneu);
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			curbyte = byte[0]; 
		}
		else printx("\t");

		tmpshort = curbyte;
		addr = addr + (Iaddr) 1;
		process->read(addr, 1, (char *) byte);
		curbyte = byte[0]; 
		
		(void) ::sprintf(mneu,"");
		while ((curbyte != 0x00ff) || (tmpshort != 0x00ff)) {
			printx("%s\t",mneu);
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			curbyte = byte[0]; 
			tmpshort = curbyte;
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			curbyte = byte[0]; 
			
			if ( debugflag )
			{
				::printf("\nInstr.C: JTAB:\n");
				::printf("\tcurbyte = %#x, \ttmpshort = %#x\n", curbyte, tmpshort);
			}
		}
		(void) ::sprintf(mneu,"***JUMP TABLE END***");
		::printline(mneu); /*dbg*/
		::strcat( mneu,"\t[]" );
		return mneu;

	/* float reg */
	case F:
		(void) ::sprintf(mneu,"%s%%st(%1.1d)",mneu,r_m);
		::printline(mneu); /*dbg*/
		(void) ::sprintf(mneu,"%s\t[ %%st(%1.1d) ]", mneu,r_m);
		return mneu;

	/* float reg to float reg, with ret bit present */
	case FF:
		if ( opcode2 >> 2 & 0x1 ) {
			/* return result bit for 287 instructions	*/
			/* st -> st(i) */
			(void) ::sprintf(mneu,"%s%%st,%%st(%1.1d)",mneu,r_m);
			(void) ::sprintf(mneu,"%s\t[ %%st, %%st(%1.1d) ]",mneu,r_m);
		}
		else {
			/* st(i) -> st */
			(void) ::sprintf(mneu,"%s%%st(%1.1d),%%st",mneu,r_m);
			(void) ::sprintf(mneu,"%s\t[ %%st(%1.1d),%%st ]",mneu,r_m);
		}
		::printline(mneu); /*dbg*/
		return mneu;

	/* an invalid op code */
	case AM:
	case DM:
	case OVERRIDE:
	case PREFIX:
	case UNKNOWN:
		::sprintf(mneu,"***** Error - bad opcode\n");
		errlev++;
		return mneu;

	default:
		printe("dis bug: notify implementor:");
		printe(" case from instruction table not found");
		exit(4);
		break;
	} /* end switch */

	if (errlev >= MAXERRS) {
		printe("dis: section probably not text section\n");
		printe("\tdisassembly terminated\n");
		exit(4);
	}
}

//
// translate branch table address to the actual function address
//
Iaddr
Instr::brtbl2fcn( Iaddr addr )
{
	Iaddr *addrptr;

	if ( ! get_text_nobkpt(addr) ) { 		 // read instr into byte[]
        	printe("Can't get_text address 0x%x\n",addr);
		return 0;
	}
	if (byte[0] == JMPrel32) {
		addrptr = (Iaddr*) &byte[1];
		return ( addr + (*addrptr) + 5);
	}
	
	return 0;
}

//
// translate  a function address to the adress of the corresponding
// branch table slot
//
Iaddr
Instr::fcn2brtbl( Iaddr addr, int offset )
{
	return ((addr  & 0xffff0000 ) + (offset-1) * 5);
}

int
Instr::deasm1( Iaddr pcaddr)
{
/* the following arrays are contained in tables.c   */
    extern  struct  instable    distable[16][16];
    extern  struct  instable    op0F[13][16];
    extern  struct  instable    opFP1n2[8][8];
	extern  struct  instable    opFP3[8][8];
    extern  struct  instable    opFP4[4][8];
    extern  struct  instable    opFP5[8];

	extern  char    *REG16[8][2];
    extern  char    *REG32[8][2];
    extern  char    *SEGREG[6];
    extern  char    *DEBUGREG[8];
    extern  char    *CONTROLREG[8];
    extern  char    *TESTREG[8];


	struct instable *dp;
	unsigned mode, reg, r_m;
	int wbit, vbit;

	/* nibbles of the opcode */
	unsigned opcode1, opcode2, opcode3, opcode4, opcode5;

	char 	*reg_name;
	char 	mnemonic[OPLEN];
	char  	mneu[1028];		// array to store mnemonic code for output
	char	temp[1028];
	int 	got_modrm_byte;
	unsigned short tmpshort;

	mnemonic[0] = '\0';
	mneu[0] = '\0';
	operand[0][0] = '\0';
	operand[1][0] = '\0';
	operand[2][0] = '\0';
	symarr[0][0] = '\0';
	symarr[1][0] = '\0';
	symarr[2][0] = '\0';
	overreg = (char *) 0;
	data16 = addr16 = 0;
	
	if  ( get_text_nobkpt(pcaddr) == 0 )  {
		printe("cannot get_text from %#x\n",pcaddr);
		return 0;
	}

	/*
	** As long as there is a prefix, the default segment register,
	** addressing-mode, or data-mode in the instruction will be overridden.
	** This may be more general than the chip actually is.
	*/
	for(;;) {
		get_opcode(&opcode1, &opcode2);
		dp = &distable[opcode1][opcode2];

		if ( dp->adr_mode == PREFIX ) 
			::strcat(mnemonic,dp->name);
		else if ( dp->adr_mode == AM ) addr16 = !addr16;
		else if ( dp->adr_mode == DM ) data16 = !data16;
		else if ( dp->adr_mode == OVERRIDE ) overreg = dp->name;
		else break;
	}

	/* some 386 instructions have 2 bytes of opcode before the mod_r/m */
	/* byte so we need to perform a table indirection.	      */
	if (dp->indirect == (struct instable *) op0F) {
		get_opcode(&opcode4,&opcode5);
		if (opcode4 > 12)
			return  0;
		dp = &op0F[opcode4][opcode5];
	}

	got_modrm_byte = 0;
	if (dp->indirect != TERM) {
		/* This must have been an opcode for which several
		 * instructions exist.  The opcode3 field further get_texts
		 * the instruction.
		 */
		got_modrm_byte = 1;
		get_modrm_byte(&mode, &opcode3, &r_m);
		/*
		 * get_text 287 instructions (D8-DF) from opcodeN
		 */
		if (opcode1 == 0xD && opcode2 >= 0x8) {
			/* instruction form 5 */
			if (opcode2 == 0xB && mode == 0x3 && opcode3 == 4)
				dp = &opFP5[r_m];
			else if (opcode2 == 0xB && mode == 0x3 && opcode3 > 4) 
				return 0;
			/* instruction form 4 */
			else if (opcode2==0x9 && mode==0x3 && opcode3 >= 4)
				dp = &opFP4[opcode3-4][r_m];
			/* instruction form 3 */
			else if (mode == 0x3)
				dp = &opFP3[opcode2-8][opcode3];
			/* instruction form 1 and 2 */
			else dp = &opFP1n2[opcode2-8][opcode3];
		}
		else
			dp = dp->indirect + opcode3;
				/* now dp points the proper subget_text table entry */
	}

	if (dp->indirect != TERM)
	 	return 0;

	/*
	 * Each instruction has a particular instruction syntax format
	 * stored in the disassembly tables.  The assignment of formats
	 * to instructins was made by the author.  Individual formats
	 * are explained as they are encountered in the following
	 * switch construct.
	 */

	switch (dp->adr_mode) {
	case MOVZ:
		if ( ! got_modrm_byte )
			get_modrm_byte(&mode, &reg, &r_m);
		if ( data16 )
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];

		wbit = WBIT(opcode5);
		data16 = 1;
		get_operand(mode, r_m, wbit, 0);
		return byte_cnt;

	/* imul instruction, with either 8-bit or longer immediate */
	case IMUL:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
	/* opcode 0x6B for byte, sign-extended displacement, 0x69 for word(s)*/
		imm_data( OPSIZE(data16,opcode2 == 0x9), 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		return byte_cnt;

	/* memory or register operand to register, with 'w' bit	*/
	case MRw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		return byte_cnt;

	/* register to memory or register operand, with 'w' bit	*/
	/* arpl happens to fit here also because it is odd */
	case RMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		return byte_cnt;

	/* Double shift. Has immediate operand specifying the shift. */
	case DSHIFT:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 1);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		imm_data(1, 0);
		return byte_cnt;

	/* Double shift. With no immediate operand, specifies using %cl. */
	case DSHIFTcl:
		get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		return byte_cnt;

	/* immediate to memory or register operand */
	case IMlw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 1);
		/* A long immediate is expected for opcode 0x81, not 0x80 nor 0x83 */
		imm_data(OPSIZE(data16,opcode2 == 1), 0);
		return byte_cnt;

	/* immediate to memory or register operand with the	*/
	/* 'w' bit present					*/
	case IMw:
		wbit = WBIT(opcode2);
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, wbit, 1);
		imm_data(OPSIZE(data16,wbit), 0);
		return byte_cnt;

	/* immediate to register with register in low 3 bits	*/
	/* of op code						*/
	case IR:
		wbit = opcode2 >>3 & 0x1; /* w-bit here (with regs) is bit 3 */
		reg = REGNO(opcode2);
		imm_data( OPSIZE(data16,wbit), 0);
		if (data16)
			reg_name = REG16[reg][wbit];
		else
			reg_name = REG32[reg][wbit];
		return byte_cnt;

	/* memory operand to accumulator			*/
	case OA:
		wbit = WBIT(opcode2);
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( data16 ? REG16 : REG32 )[0][wbit];
		return byte_cnt;

	/* accumulator to memory operand			*/
	case AO:
		wbit = WBIT(opcode2);
		
		displacement(OPSIZE(addr16,LONGOPERAND), 0,&lngval);
		reg_name = ( addr16 ? REG16 : REG32 )[0][wbit];
		return byte_cnt;

	/* memory or register operand to segment register	*/
	case MS:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		return byte_cnt;

	/* segment register to memory or register operand	*/
	case SM:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		return byte_cnt;

	/* rotate or shift instrutions, which may shift by 1 or */
	/* consult the cl register, depending on the 'v' bit	*/
	case Mv:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		return byte_cnt;

	/* immediate rotate or shift instrutions, which may or */
	/* may not consult the cl register, depending on the 'v' bit	*/
	case MvI:
		vbit = VBIT(opcode2);
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		imm_data(1,1);
		/* When vbit is set, register is an operand, otherwise just $0x1 */
		reg_name = vbit ? "%cl," : "" ;
		return byte_cnt;

	case MIb:
		get_operand(mode, r_m, LONGOPERAND, 0);
		imm_data(1,1);
		return byte_cnt;

	/* single memory or register operand with 'w' bit present*/
	case Mw:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		return byte_cnt;

	/* single memory or register operand			*/
	case M:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		return byte_cnt;

	case SREG: /* special register */
		get_modrm_byte(&mode, &reg, &r_m);
		vbit = 0;
		switch (opcode5) {
		case 2:
			vbit = 1;
			/* fall thru */
		case 0: 
			reg_name = CONTROLREG[reg];
			break;
		case 3:
			vbit = 1;
			/* fall thru */
		case 1:
			reg_name = DEBUGREG[reg];
			break;
		case 6:
			vbit = 1;
			/* fall thru */
		case 4:
			reg_name = TESTREG[reg];
			break;
		}
		strcpy(operand[0], REG32[r_m][1]);

		if (vbit)
		{
			strcpy(operand[0], reg_name);
			reg_name = REG32[r_m][1];
		}
		
		return byte_cnt;

	/* single register operand with register in the low 3	*/
	/* bits of op code					*/
	case R:
		reg = REGNO(opcode2);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		return byte_cnt;

	/* register to accumulator with register in the low 3	*/
	/* bits of op code, xchg instructions                   */
	case RA: {
		char *eprefix;
		reg = REGNO(opcode2);
		if (data16) {
			eprefix = "";
			reg_name = REG16[reg][LONGOPERAND];
		}
		else {
			eprefix = "e";
			reg_name = REG32[reg][LONGOPERAND];
		}
		return byte_cnt;
	}

	/* single segment register operand, with register in	*/
	/* bits 3-4 of op code					*/
	case SEG:
		reg = curbyte >> 3 & 0x3; /* segment register */
		return byte_cnt;

	/* single segment register operand, with register in	*/
	/* bits 3-5 of op code					*/
	case LSEG:
		reg = curbyte >> 3 & 0x7; /* long seg reg from opcode */
		return byte_cnt;

	/* memory or register operand to register		*/
	case MR:
		if (!got_modrm_byte)
			get_modrm_byte(&mode, &reg, &r_m);
		get_operand(mode, r_m, LONGOPERAND, 0);
		if (data16)
			reg_name = REG16[reg][LONGOPERAND];
		else
			reg_name = REG32[reg][LONGOPERAND];
		return byte_cnt;

	/* immediate operand to accumulator			*/
	case IA: {
		int no_bytes = OPSIZE(data16,WBIT(opcode2));
		switch(no_bytes) {
			case 1: reg_name = "%al"; break;
			case 2: reg_name = "%ax"; break;
			case 4: reg_name = "%eax"; break;
		}
		imm_data(no_bytes, 0);
		return byte_cnt;
	}
	/* memory or register operand to accumulator		*/
	case MA:
		wbit = WBIT(opcode2);
		get_operand(mode, r_m, wbit, 0);
		reg_name = ( data16 ? REG16 : REG32) [0][wbit];
		return byte_cnt;

	/* si register to di register				*/
	case SD:
		check_override(0);
		return byte_cnt;

	/* accumulator to di register				*/
	case AD:
		wbit = WBIT(opcode2);
		check_override(0);
		return byte_cnt;

	/* si register to accumulator				*/
	case SA:
		wbit = WBIT(opcode2);
		check_override(0);
		reg_name = (addr16 ? REG16 : REG32) [0][wbit] ;
		return byte_cnt;

	/* single operand, a 16/32 bit displacement		*/
	/* added to current offset by 'compoff'			*/
	case D:
		displacement(OPSIZE(data16,LONGOPERAND), 0, &lngval);
		compoff(lngval, operand[1]);
		return byte_cnt;

	/* indirect to memory or register operand		*/
	case INM:
		get_operand(mode, r_m, LONGOPERAND, 0);
		return byte_cnt;

	/* for long jumps and long calls -- a new code segment   */
	/* register and an offset in IP -- stored in object      */
	/* code in reverse order                                 */
	case SO:
		displacement(OPSIZE(addr16,LONGOPERAND), 1,&lngval);
		/* will now get segment operand*/
		displacement(2, 0,&lngval);
		return byte_cnt;

	/* jmp/call. single operand, 8 bit displacement.	*/
	/* added to current EIP in 'compoff'			*/
	case BD:
		displacement(1, 0, &lngval);
		compoff(lngval, operand[1]);
		return byte_cnt;

	/* single 32/16 bit immediate operand			*/
	case I:
		imm_data(OPSIZE(data16,LONGOPERAND), 0);
		return byte_cnt;

	/* single 8 bit immediate operand			*/
	case Ib:
		imm_data(1, 0);
		return byte_cnt;

	case ENTER:
		imm_data(2,0);
		imm_data(1,1);
		return byte_cnt;

	/* 16-bit immediate operand */
	case RET:
		imm_data(2,0);
		return byte_cnt;

	/* single 8 bit port operand				*/
	case P:
		check_override(0);
		imm_data(1, 0);
		return byte_cnt;

	/* single operand, dx register (variable port instruction)*/
	case V:
		check_override(0);
		return byte_cnt;

	case INT3:
		return byte_cnt;

	/* an unused byte must be discarded			*/
	case U:
		get_byte();
		return byte_cnt;

	case CBW:
	case CWD:
	case GO_ON:
		return byte_cnt;
	case JTAB:
		int cnt; // used for counting how many bytes we skipped
			 // in the jump table.
		addr = addr + (Iaddr) byte_cnt;
		cnt = 0;
		process->read(addr, 1, (char *) byte);
		cnt++;
		curbyte = byte[0]; 
		if (curbyte == FILL) {
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			cnt++;
			curbyte = byte[0]; 
		}

		tmpshort = curbyte;
		addr = addr + (Iaddr) 1;
		process->read(addr, 1, (char *) byte);
		cnt++;
		curbyte = byte[0]; 
		
		while ((curbyte != 0x00ff) || (tmpshort != 0x00ff)) {
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			cnt++;
			curbyte = byte[0]; 
			tmpshort = curbyte;
			addr = addr + (Iaddr) 1;
			process->read(addr, 1, (char *) byte);
			cnt++;
			curbyte = byte[0]; 
			
		}
		return (byte_cnt + (--cnt ));

	/* float reg */
	case F:
	case FF:
		return byte_cnt;

	case AM:
	case DM:
	case OVERRIDE:
	case PREFIX:
	case UNKNOWN:
		return byte_cnt;

	default:
		return 0;
	} /* end switch */

}

Iaddr
Instr::jmp_target( Iaddr )	// a stub will do for now ...
{
	return 0;
}
