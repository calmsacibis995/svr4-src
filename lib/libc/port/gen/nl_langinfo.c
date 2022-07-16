/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/nl_langinfo.c	1.7"

#ifdef __STDC__
	#pragma weak nl_langinfo = _nl_langinfo
#endif
#include "synonyms.h"
#include <stdlib.h>
#include <limits.h>
#include <nl_types.h>
#include <langinfo.h>
#include <locale.h>
#include <time.h>
#include <string.h>

#define MAX 128

#ifdef __STDC__
extern char *gettxt(const char *, const char *);
#else
extern char *gettxt();
#endif
extern size_t strftime();
extern struct lconv *localeconv();

static char *old_locale;

char *
nl_langinfo( item )
nl_item      item;
{
struct tm tm;
static char *buf;
char buf2[MAX];
struct lconv *currency;
char *s;
int size;
const char *rptr;

	if (!buf && (buf = malloc(MAX)) == NULL)
		return "";

	switch (item) {
		/*
		 * The seven days of the week in their full beauty
		 */

		case DAY_1 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=0;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Sunday";
			break;

		case DAY_2 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=1;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Monday";
			break;

		case DAY_3 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=2;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Tuesday";
			break;

		case DAY_4 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=3;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Wednesday";
			break;

		case DAY_5 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=4;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Thursday";
			break;

		case DAY_6 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=5;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Friday";
			break;

		case DAY_7 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=6;
			size = strftime(buf,MAX,"%A",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Saturday";
			break;


		/*
		 * The abbreviated seven days of the week
		 */
		case ABDAY_1 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=0;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Sun";
			break;

		case ABDAY_2 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=1;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Mon";
			break;

		case ABDAY_3 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=2;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Tue";
			break;

		case ABDAY_4 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=3;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Wed";
			break;

		case ABDAY_5 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=4;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Thur";
			break;

		case ABDAY_6 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=5;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Fri";
			break;

		case ABDAY_7 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_wday=6;
			size = strftime(buf,MAX,"%a",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Sat";
			break;



		/*
		 * The full names of the twelve months...
		 */
		case MON_1 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=0;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "January";
			break;

		case MON_2 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=1;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Feburary";
			break;

		case MON_3 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=2;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "March";
			break;

		case MON_4 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=3;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "April";
			break;

		case MON_5 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=4;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "May";
			break;

		case MON_6 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=5;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "June";
			break;

		case MON_7 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=6;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "July";
			break;

		case MON_8 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=7;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "August";
			break;

		case MON_9 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=8;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "September";
			break;

		case MON_10 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=9;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "October";
			break;

		case MON_11 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=10;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "November";
			break;

		case MON_12 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=11;
			size = strftime(buf,MAX,"%B",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "December";
			break;

		/*
		 * ... and their abbreviated form
		 */
		case ABMON_1 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=0;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Jan";
			break;

		case ABMON_2 :
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=1;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Feb";
			break;

		case ABMON_3 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=2;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Mar";
			break;

		case ABMON_4 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=3;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Apr";
			break;

		case ABMON_5 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=4;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "May";
			break;

		case ABMON_6 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=5;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Jun";
			break;

		case ABMON_7 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=6;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Jul";
			break;

		case ABMON_8 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=7;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Aug";
			break;

		case ABMON_9 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=8;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Sep";
			break;

		case ABMON_10 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=9;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Oct";
			break;

		case ABMON_11 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=10;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Nov";
			break;

		case ABMON_12 : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_mon=11;
			size = strftime(buf,MAX,"%b",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "Dec";
			break;

		case AM_STR : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_hour=1;
			size = strftime(buf,MAX,"%p",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "AM";
			break;

		case PM_STR : 
			memset((void*)&tm,sizeof (struct tm),0);
			tm.tm_hour=13;
			size = strftime(buf,MAX,"%p",&tm);
			if (size)
				rptr = (const char *) buf;
			else
				rptr = "PM";
			break;


		/*
		 * plus some special strings you might need to know
		 */

		case RADIXCHAR :
		case THOUSEP :
		case CRNCYSTR :

			currency = localeconv();
			switch (item) {

				case THOUSEP :
					return currency->thousands_sep;

				case RADIXCHAR :
					return currency->decimal_point;

				case CRNCYSTR : 
					if (currency->p_cs_precedes == CHAR_MAX || *(currency->currency_symbol) == '\0')
						return "";
					if (currency->p_cs_precedes == 1)
						buf[0] = '-';
					else
						buf[0] = '+';
					strcpy(&buf[1], currency->currency_symbol);
					return buf;
			}
			break;

		/*
		 * Default string used to format date and time
		 *	e.g. Sunday, August 24 21:08:38 MET 1986
		 */

		case T_FMT :
			old_locale = setlocale(LC_MESSAGES,(char*)NULL);
			strcpy(buf,old_locale);
			(void)setlocale(LC_MESSAGES,setlocale(LC_TIME,(char*)NULL));
			s = gettxt("Xopen_info:1","%H:%M:%S");
			setlocale(LC_MESSAGES,buf);
			if (strcmp(s,"Message not found!!\n"))
				rptr = (const char *) s;
			else 
				rptr = "%H:%M:%S";
			break;

		case D_FMT :
			old_locale = setlocale(LC_MESSAGES,(char*)NULL);
			strcpy(buf,old_locale);
			(void)setlocale(LC_MESSAGES,setlocale(LC_TIME,(char*)NULL));
			s = gettxt("Xopen_info:2","%m/%d/%y");
			setlocale(LC_MESSAGES,buf);
			if (strcmp(s,"Message not found!!\n"))
				rptr = (const char *) s;
			else 
				rptr = "%m/%d/%y";
			break;

		case D_T_FMT :
			old_locale = setlocale(LC_MESSAGES,(char*)NULL);
			strcpy(buf,old_locale);
			(void)setlocale(LC_MESSAGES,setlocale(LC_TIME,(char*)NULL));
			s = gettxt("Xopen_info:3","%a %b %d %H:%M:%S %Y");
			setlocale(LC_MESSAGES,buf);
			if (strcmp(s,"Message not found!!\n"))
				rptr = (const char *) s;
			else 
				rptr = "%a %b %d %H:%M:%S %Y";
			break;

		case YESSTR :
			old_locale = setlocale(LC_MESSAGES,(char*)NULL);
			strcpy(buf,old_locale);
			old_locale=setlocale(LC_ALL,(char*)NULL);
			if (*old_locale == '/') {
				/*
				 * composite locale
				 */
				old_locale++;
				s = buf2;
				while (*old_locale != '/')
					*s++ = *old_locale++;
				*s = '\0';
			} else
				strcpy(buf2,old_locale);
			old_locale = setlocale(LC_MESSAGES,buf2);
			s = gettxt("Xopen_info:4","yes");
			setlocale(LC_MESSAGES,buf);
			if (strcmp(s,"Message not found!!\n"))
				rptr = (const char *) s;
			else 
				rptr = "yes";
			break;

		case NOSTR :
			old_locale = setlocale(LC_MESSAGES,(char*)NULL);
			strcpy(buf,old_locale);
			old_locale=setlocale(LC_ALL,(char*)NULL);
			if (*old_locale == '/') {
				/*
				 * composite locale
				 */
				old_locale++;
				s = buf2;
				while (*old_locale != '/')
					*s++ = *old_locale++;
				*s = '\0';
			} else
				strcpy(buf2,old_locale);
			old_locale = setlocale(LC_MESSAGES,buf2);
			(void)setlocale(LC_MESSAGES,old_locale);
			s = gettxt("Xopen_info:5","no");
			setlocale(LC_MESSAGES,buf);
			if (strcmp(s,"Message not found!!\n"))
				rptr = (const char *) s;
			else 
				rptr = "no";
			break;

		default :
			rptr = "";
			break;

	    }

	return (char *) rptr;
}
