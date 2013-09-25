/*
 *  ed - standard editor
 *  ~~
 *  Authors: Brian Beattie, Kees Bot, and others
 *
 * Copyright 1987 Brian Beattie Rights Reserved.
 * Permission to copy or distribute granted under the following conditions:
 * 1). No charge may be made other than reasonable charges for reproduction.
 * 2). This notice must remain intact.
 * 3). No further restrictions may be added.
 * 4). Except meaningless ones.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  TurboC mods and cleanup 8/17/88 RAMontante.
 *  Further information (posting headers, etc.) at end of file.
 *  RE stuff replaced with Spencerian version, sundry other bugfix+speedups
 *  Ian Phillipps. Version incremented to "5".
 * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 */

int version = 5;    /* used only in the "set" function, for i.d. */

#include <stdio.h>
/* Regexp is Henry Spencer's package. WARNING: regsub is modified to return
 * a pointer to the \0 after the destination string, and this program refers
 * to the "private" reganch field in the struct regexp.
 */
#include "regexp.h"

#ifdef __TURBOC__
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#define _cleanup()  ;

#endif  /* ifdef __TURBOC__  */


/*
 *  #defines for non-printing ASCII characters
 */
#define NUL 0x00    /* ^@ */
#define EOS 0x00    /* end of string */
#define SOH 0x01    /* ^A */
#define STX 0x02    /* ^B */
#define ETX 0x03    /* ^C */
#define EOT 0x04    /* ^D */
#define ENQ 0x05    /* ^E */
#define ACK 0x06    /* ^F */
#define BEL 0x07    /* ^G */
#define BS  0x08    /* ^H */
#define HT  0x09    /* ^I */
#define LF  0x0a    /* ^J */
#define NL  '\n'
#define VT  0x0b    /* ^K */
#define FF  0x0c    /* ^L */
#define CR  0x0d    /* ^M */
#define SO  0x0e    /* ^N */
#define SI  0x0f    /* ^O */
#define DLE 0x10    /* ^P */
#define DC1 0x11    /* ^Q */
#define DC2 0x12    /* ^R */
#define DC3 0x13    /* ^S */
#define DC4 0x14    /* ^T */
#define NAK 0x15    /* ^U */
#define SYN 0x16    /* ^V */
#define ETB 0x17    /* ^W */
#define CAN 0x18    /* ^X */
#define EM  0x19    /* ^Y */
#define SUB 0x1a    /* ^Z */
#define ESC 0x1b    /* ^[ */
#define FS  0x1c    /* ^\ */
#define GS  0x1d    /* ^] */
#define RS  0x1e    /* ^^ */
#define US  0x1f    /* ^_ */
#define SP  0x20    /* space */
#define DEL 0x7f    /* DEL*/
#define ESCAPE  '\\'


#define TRUE    1
#define FALSE   0
#define ERR     -2
#define FATAL       (ERR-1)
#define CHANGED     (ERR-2)
#define SET_FAIL    (ERR-3)
#define SUB_FAIL    (ERR-4)
#define MEM_FAIL    (ERR-5)


#define BUFFER_SIZE 2048    /* stream-buffer size:  == 1 hd cluster */

#define LINFREE 1   /* entry not in use */
#define LGLOB   2       /* line marked global */

#define MAXLINE 512 /* max number of chars per line */
#define MAXPAT  256 /* max number of chars per replacement pattern */
#define MAXFNAME 256    /* max file name size */


/**  Global variables  **/

struct  line {
    int     l_stat;     /* empty, mark */
    struct line *l_prev;
    struct line *l_next;
    char        l_buff[1];
};
typedef struct line LINE;


int diag = 1;       /* diagnostic-output? flag */
int truncflg = 1;   /* truncate long line flag */
int eightbit = 1;   /* save eighth bit */
int nonascii;   /* count of non-ascii chars read */
int nullchar;   /* count of null chars read */
int truncated;  /* count of lines truncated */
char    fname[MAXFNAME];
int fchanged;   /* file-changed? flag */
int nofname;
int mark['z'-'a'+1];
regexp  *oldpat;

LINE    Line0;
int CurLn = 0;
LINE    *CurPtr = &Line0;   /* CurLn and CurPtr must be kept in step */
int LastLn = 0;
char    inlin[MAXLINE];
int pflag;
int Line1, Line2, nlines;
int nflg;       /* print line number flag */
int lflg;       /* print line in verbose mode */
int pflg;       /* print current line after each command */
char    *inptr;     /* tty input buffer */

struct tbl {
    char    *t_str;
    int *t_ptr;
    int t_val;
} *t, tbl[] = {
    "number",   &nflg,      TRUE,
    "nonumber", &nflg,      FALSE,
    "list",     &lflg,      TRUE,
    "nolist",   &lflg,      FALSE,
    "eightbit", &eightbit,  TRUE,
    "noeightbit",   &eightbit,  FALSE,
    0
};


/*-------------------------------------------------------------------------*/

#ifdef __TURBOC__       /*  prototypes (unneeded?)  */

void    prntln(char *, int, int);
void    putcntl(char , FILE *);
int doprnt(int, int);
LINE    *getptr(int);
int del(int, int);
int ins(char *);
int append(int, int);
int deflt(int, int);
regexp  *optpat(void);
int egets( char*, int, FILE* );
int ckglob( void );
int doglob( void );
int docmd( int );
int dolst( int, int );
int doread( int, char* );
int dowrite( int, int, char*, int );
int find( regexp*, int );
char*   getfn( void );
int getlst( void );
int getnum( int );
int getone( void );
int getrhs( char* );
int join( int, int );
int move( int );
int set( void );
int subst( regexp*, char*, int, int );
int transfer( int );

#else   /* !__TURBOC__ */

extern  char    *strcpy();
extern  int *malloc();
extern  LINE    *getptr();
extern  char    *gettxt();
extern  char    *gettxtl();
extern  char    *catsub();
regexp  *optpat();

#endif  /* __TURBOC__ */


/*________  Macros  ________________________________________________________*/

#ifndef max
#  define max(a,b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#  define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

#ifndef toupper
#  define toupper(c)    ((c >= 'a' && c <= 'z') ? c-32 : c )
#endif

#define nextln(l)   ((l)+1 > LastLn ? 0 : (l)+1)
#define prevln(l)   ((l)-1 < 0 ? LastLn : (l)-1)

#define gettxtl(lin)    ((lin)->l_buff)
#define gettxt(num) (gettxtl( getptr(num) ))

#define getnextptr(p)   ((p)->l_next)
#define getprevptr(p)   ((p)->l_prev)

#define setCurLn( lin ) ( CurPtr = getptr( CurLn = (lin) ) )
#define nextCurLn() ( CurLn = nextln(CurLn), CurPtr = getnextptr( CurPtr ) )
#define prevCurLn() ( CurLn = prevln(CurLn), CurPtr = getprevptr( CurPtr ) )

#define clrbuf()    del(1, LastLn)

#define Skip_White_Space    {while (*inptr==SP || *inptr==HT) inptr++;}

#define relink(a, x, y, b) { (x)->l_prev = (a); (y)->l_next = (b); }


/*________  functions  ______________________________________________________*/


/*  append.c    */

append(line, glob)
int line, glob;
{
    char    lin[MAXLINE];

    if(glob)
        return(ERR);
    setCurLn( line );
    for (;;) {
        char *p = lin;
        if(nflg)
            printf("%6d. ",CurLn+1);
        while( p < &lin[ MAXLINE ] ) {
            int ch = getchar();
            if( ch == EOF )
                return EOF;
            if( ch == '\n' ) {
                *p = EOS; break;
            }
            *p++ = ch;
        }
        if(lin[0] == '.' && lin[1] == '\0')
            return(0);
        if( ins(lin) < 0)
            return( MEM_FAIL );
    }
}

/*  ckglob.c    */

ckglob()
{
    regexp  *glbpat;
    char    c, delim, *lin;
    int num;
    LINE    *ptr;

    c = *inptr;

    if(c != 'g' && c != 'v')
        return(0);
    if (deflt(1, LastLn) < 0)
        return(ERR);

    delim = *++inptr;
    if(delim <= ' ')
        return(ERR);

    glbpat = optpat();
    if(*inptr == delim)
        inptr++;
    ptr = getptr(1);
    for (num=1; num<=LastLn; num++) {
        ptr->l_stat &= ~LGLOB;
        if (Line1 <= num && num <= Line2) {
            lin = gettxtl(ptr);
            if(regexec(glbpat, lin )) {
                if (c=='g') ptr->l_stat |= LGLOB;
            } else {
                if (c=='v') ptr->l_stat |= LGLOB;
            }
        ptr = getnextptr(ptr);
        }
    }
    return(1);
}


/*  deflt.c
 *  Set Line1 & Line2 (the command-range delimiters) if the file is
 *  empty; Test whether they have valid values.
 */

int deflt(def1, def2)
int def1, def2;
{
    if(nlines == 0) {
        Line1 = def1;
        Line2 = def2;
    }
    return ( (Line1>Line2 || Line1<=0) ? ERR : 0 );
}


/*  del.c   */

/* One of the calls to this function tests its return value for an error
 * condition.  But del doesn't return any error value, and it isn't obvious
 * to me what errors might be detectable/reportable.  To silence a warning
 * message, I've added a constant return statement. -- RAM
 * ... It could check to<=LastLn ... igp
 */

del(from, to)
int from, to;
{
    LINE    *first, *last, *next, *tmp;

    if(from < 1)
        from = 1;
    first = getprevptr( getptr( from ) );
    last = getnextptr( getptr( to ) );
    next = first->l_next;
    while(next != last && next != &Line0) {
        tmp = next->l_next;
        free(next);
        next = tmp;
    }
    relink(first, last, first, last);
    LastLn -= (to - from)+1;
    setCurLn( prevln(from) );
    return(0);
}


int dolst(line1, line2)
int line1, line2;
{
    int oldlflg=lflg, p;

    lflg = 1;
    p = doprnt(line1, line2);
    lflg = oldlflg;
    return p;
}


/*  esc.c
 * Map escape sequences into their equivalent symbols.  Returns the
 * correct ASCII character.  If no escape prefix is present then s
 * is untouched and *s is returned, otherwise **s is advanced to point
 * at the escaped character and the translated character is returned.
 */
esc(s)
char    **s;
{
    register int    rval;

    if (**s != ESCAPE) {
        rval = **s;
    } else {
        (*s)++;
        switch(toupper(**s)) {
        case '\000':
            rval = ESCAPE;  break;
        case 'S':
            rval = ' '; break;
        case 'N':
            rval = '\n';    break;
        case 'T':
            rval = '\t';    break;
        case 'B':
            rval = '\b';    break;
        case 'R':
            rval = '\r';    break;
        default:
            rval = **s; break;
        }
    }
    return (rval);
}


/*  doprnt.c    */

int doprnt(from, to)
int from, to;
{
    from = (from < 1) ? 1 : from;
    to = (to > LastLn) ? LastLn : to;

    if(to != 0) {
        setCurLn( from );
        while( CurLn <= to ) {
            prntln( gettxtl( CurPtr ), lflg, (nflg ? CurLn : 0));
            if( CurLn == to )
                break;
            nextCurLn();
        }
    }
    return(0);
}


void prntln(str, vflg, lin)
char    *str;
int vflg, lin;
{
    if(lin)
        printf("%7d ",lin);
    while(*str && *str != NL) {
        if(*str < ' ' || *str >= 0x7f) {
            switch(*str) {
            case '\t':
                if(vflg)
                    putcntl(*str, stdout);
                else
                    putc(*str, stdout);
                break;

            case DEL:
                putc('^', stdout);
                putc('?', stdout);
                break;

            default:
                putcntl(*str, stdout);
                break;
            }
        } else
            putc(*str, stdout);
        str++;
    }
    if(vflg)
        putchar('$');
    putchar('\n');
}


void putcntl(c, stream)
char    c;
FILE    *stream;
{
    putc('^', stream);
    putc((c&31)|'@', stream);
}


/*  egets.c */

egets(str,size,stream)
char    *str;
int size;
FILE    *stream;
{
    int c, count;
    char    *cp;

    for(count = 0, cp = str; size > count;) {
        c = getc(stream);
        if(c == EOF) {
            *cp = EOS;
            if(count)
                puts("[Incomplete last line]");
            return(count);
        }
        else if(c == NL) {
            *cp = EOS;
            return(++count);
        }
        else if (c == 0)
            nullchar++; /* count nulls */
        else {
            if(c > 127) {
                if(!eightbit)       /* if not saving eighth bit */
                    c = c&127;  /* strip eigth bit */
                nonascii++;     /* count it */
            }
            *cp++ = c;  /* not null, keep it */
            count++;
        }
    }
    str[count-1] = EOS;
    if(c != NL) {
        puts("truncating line");
        truncated++;
        while((c = getc(stream)) != EOF)
            if(c == NL)
                break;
    }
    return(count);
}  /* egets */


doread(lin, fname)
int lin;
char    *fname;
{
    FILE    *fp;
    int err;
    unsigned long   bytes;
    unsigned int    lines;
    static char str[MAXLINE];

    err = 0;
    nonascii = nullchar = truncated = 0;

    if (diag) printf("\"%s\" ",fname);
    if( (fp = fopen(fname, "r")) == NULL ) {
        puts(" isn't readable.");
        return( ERR );
    }
    setvbuf(fp, NULL, _IOFBF, BUFFER_SIZE);
    setCurLn( lin );
    for(lines = 0, bytes = 0;(err = egets(str,MAXLINE,fp)) > 0;) {
        bytes += err;
        if(ins(str) < 0) {
            err = MEM_FAIL;
            break;
        }
        lines++;
    }
    fclose(fp);
    if(err < 0)
        return(err);
    if (diag) {
        printf("%u lines %u bytes",lines,bytes);
        if(nonascii)
            printf(" [%d non-ascii]",nonascii);
        if(nullchar)
            printf(" [%d nul]",nullchar);
        if(truncated)
            printf(" [%d lines truncated]",truncated);
        putchar('\n');
    }
    return( err );
}  /* doread */


int dowrite(from, to, fname, apflg)
int from, to;
char    *fname;
int apflg;
{
    FILE    *fp;
    int lin, err;
    unsigned int    lines;
    unsigned long   bytes;
    char    *str;
    LINE    *lptr;

    err = 0;
    lines = bytes = 0;

    printf("\"%s\" ",fname);
    if((fp = fopen(fname,(apflg?"a":"w"))) == NULL) {
        puts(" can't be opened for writing!");
        return( ERR );
    }

    setvbuf(fp, NULL, _IOFBF, BUFFER_SIZE);
    lptr = getptr(from);
    for(lin = from; lin <= to; lin++) {
        str = lptr->l_buff;
        lines++;
        bytes += strlen(str) + 1;   /* str + '\n' */
        if(fputs(str, fp) == EOF) {
            puts("file write error");
            err++;
            break;
        }
        fputc('\n', fp);
        lptr = lptr->l_next;
    }
    printf("%u lines %lu bytes\n",lines,bytes);
    fclose(fp);
    return( err );
}  /* dowrite */


/*  find.c  */

find(pat, dir)
regexp  *pat;
int dir;
{
    int i, num;
    LINE    *lin;

    num = CurLn;
    lin = CurPtr;
    for(i=0; i<LastLn; i++ ) {
        if(regexec( pat, gettxtl( lin ) ))
            return(num);
        if( dir )
            num = nextln(num), lin = getnextptr(lin);
        else
            num = prevln(num), lin = getprevptr(lin);
    }
    return ( ERR );
}


/*  getfn.c */

char *getfn()
{
    static char file[256];
    char    *cp;

    if(*inptr == NL) {
        nofname=TRUE;
        strcpy(file, fname);
    } else {
        nofname=FALSE;
        Skip_White_Space;

        cp = file;
        while(*inptr && *inptr != NL && *inptr != SP && *inptr != HT)
            *cp++ = *inptr++;
        *cp = '\0';

        if(strlen(file) == 0) {
            puts("bad file name");
            return( NULL );
        }
    }

    if(strlen(file) == 0) {
        puts("no file name");
        return(NULL);
    }
    return( file );
}  /* getfn */


int getnum(first)
int first;
{
    regexp  *srchpat;
    int num;
    char    c;

    Skip_White_Space;

    if(*inptr >= '0' && *inptr <= '9') {    /* line number */
        for(num = 0; *inptr >= '0' && *inptr <= '9'; ++inptr) {
            num = (num * 10) + (*inptr - '0');
        }
        return num;
    }

    switch(c = *inptr) {
    case '.':
        inptr++;
        return (CurLn);

    case '$':
        inptr++;
        return (LastLn);

    case '/':
    case '?':
        srchpat = optpat();
        if(*inptr == c)
            *inptr++;
        return(find(srchpat,c == '/'?1:0));

    case '-':
    case '+':
        return(first ? CurLn : 1);

    case '\'':
        inptr++;
        if (*inptr < 'a' || *inptr > 'z')
            return(EOF);
        return mark[ *inptr++ - 'a' ];

    default:
        return ( first ? EOF : 1 ); /* unknown address */
    }
}  /* getnum */


/*  getone.c
 *  Parse a number (or arithmetic expression) off the command line.
 */
#define FIRST 1
#define NOTFIRST 0

int getone()
{
    int c, i, num;

    if((num = getnum(FIRST)) >= 0) {
        for (;;) {
            Skip_White_Space;
            if(*inptr != '+' && *inptr != '-')
                break;  /* exit infinite loop */

                        c = *inptr++;
            if((i = getnum(NOTFIRST)) < 0)
                return ( i );
            if(c == '+')
                num += i;
            else
                num -= i;
        }
    }
    return ( num>LastLn ? ERR : num );
}  /* getone */


getlst()
{
    int num;

    Line2 = 0;
    for(nlines = 0; (num = getone()) >= 0;)
    {
        Line1 = Line2;
        Line2 = num;
        nlines++;
        if(*inptr != ',' && *inptr != ';')
            break;
        if(*inptr == ';')
            setCurLn( num );
        inptr++;
    }
    nlines = min(nlines, 2);
    if(nlines == 0)
        Line2 = CurLn;
    if(nlines <= 1)
        Line1 = Line2;

    return ( (num == ERR) ? num : nlines );
}  /* getlst */


/*  getptr.c    */

LINE *getptr(num)
int num;
{
    LINE    *ptr;
    int j;

    if (2*num>LastLn && num<=LastLn) {  /* high line numbers */
        ptr = Line0.l_prev;
        for (j = LastLn; j>num; j--)
            ptr = ptr->l_prev;
    } else {                /* low line numbers */
        ptr = &Line0;
        for(j = 0; j < num; j++)
            ptr = ptr->l_next;
    }
    return(ptr);
}


/*  getrhs.c    */

int getrhs(sub)
char    *sub;
{
    char delim = *inptr++;
    char *outmax = sub + MAXPAT;
    if( delim == NL || *inptr == NL)    /* check for eol */
        return( ERR );
    while( *inptr != delim && *inptr != NL ) {
        if ( sub > outmax )
            return ERR;
        if ( *inptr == ESCAPE ) {
            switch ( *++inptr ) {
            case 'r':
                *sub++ = '\r';
                inptr++;
                break;
            case ESCAPE:
                *sub++ = ESCAPE;
                *sub++ = ESCAPE;
                inptr++;
            case 'n':
                *sub++ = '\n';
                inptr++;
                break;
            case 'b':
                *sub++ = '\b';
                inptr++;
                break;
            case '0': {
                int i=3;
                *sub = 0;
                do {
                    if (*++inptr<'0' || *inptr >'7')
                        break;
                    *sub = (*sub<<3) | (*inptr-'0');
                } while (--i!=0);
                sub++;
                } break;
            default:
                if ( *inptr != delim )
                    *sub++ = ESCAPE;
                *sub++ = *inptr;
                if ( *inptr != NL )
                    inptr++;
            }
        }
        else *sub++ = *inptr++;
    }
    *sub = '\0';

    inptr++;        /* skip over delimter */
    Skip_White_Space;
    if(*inptr == 'g') {
        *inptr++;
        return( 1 );
    }
    return( 0 );
}

/*  ins.c   */

ins(str)
char    *str;
{
    char    *cp;
    LINE    *new, *nxt;
    int len;

    do {
        for ( cp = str; *cp && *cp != NL; cp++ )
            ;
        len = cp - str;
        /* cp now points to end of first or only line */

        if((new = (LINE *)malloc(sizeof(LINE)+len)) == NULL)
            return( MEM_FAIL );     /* no memory */

        new->l_stat=0;
        strncpy(new->l_buff,str,len);   /* build new line */
        new->l_buff[len] = EOS;
        nxt = getnextptr(CurPtr);   /* get next line */
        relink(CurPtr, new, new, nxt);  /* add to linked list */
        relink(new, nxt, CurPtr, new);
        LastLn++;
        CurLn++;
        CurPtr = new;
        str = cp + 1;
    }
        while( *cp != EOS );
    return 1;
}


/*  join.c  */

int join(first, last)
int first, last;
{
    char buf[MAXLINE];
    char *cp=buf, *str;
    LINE *lin;
    int num;

    if (first<=0 || first>last || last>LastLn)
        return(ERR);
    if (first==last) {
        setCurLn( first );
        return 0;
    }
    lin = getptr(first);
    for (num=first; num<=last; num++) {
        str=gettxtl(lin);
        while ( *str ) {
            if (cp >= buf + MAXLINE-1 ) {
                puts("line too long");
                return(ERR);
            }
            *cp++ = *str++;
        }
        lin = getnextptr(lin);
    }
    *cp = EOS;
    del(first, last);
    if( ins(buf) < 0 )
        return MEM_FAIL;
    fchanged = TRUE;
    return 0;
}


/*  move.c
 *  Unlink the block of lines from Line1 to Line2, and relink them
 *  after line "num".
 */

int move(num)
int num;
{
    int range;
    LINE    *before, *first, *last, *after;

    if( Line1 <= num && num <= Line2 )
        return( ERR );
    range = Line2 - Line1 + 1;
    before = getptr(prevln(Line1));
    first = getptr(Line1);
    last = getptr(Line2);
    after = getptr(nextln(Line2));

    relink(before, after, before, after);
    LastLn -= range;    /* per AST's posted patch 2/2/88 */
    if (num > Line1)
        num -= range;

    before = getptr(num);
    after = getptr(nextln(num));
    relink(before, first, last, after);
    relink(last, after, before, first);
    LastLn += range;    /* per AST's posted patch 2/2/88 */
    setCurLn( num + range );
    return( 1 );
}


int transfer(num)
int num;
{
    int mid, lin, ntrans;

    if (Line1<=0 || Line1>Line2)
        return(ERR);

    mid= num<Line2 ? num : Line2;

    setCurLn( num );
    ntrans=0;

    for (lin=Line1; lin<=mid; lin++) {
        if( ins(gettxt(lin)) < 0 )
            return MEM_FAIL;
        ntrans++;
    }
    lin+=ntrans;
    Line2+=ntrans;

    for ( ; lin <= Line2; lin += 2 ) {
        if( ins(gettxt(lin)) < 0 )
            return MEM_FAIL;
        Line2++;
    }
    return(1);
}


/*  optpat.c    */

regexp *optpat(void)
{
    char    delim, str[MAXPAT], *cp;

    delim = *inptr++;
    cp = str;
    while(*inptr != delim && *inptr != NL) {
        if(*inptr == ESCAPE && inptr[1] != NL)
            *cp++ = *inptr++;
        *cp++ = *inptr++;
    }

    *cp = EOS;
    if(*str == EOS)
        return(oldpat);
    if(oldpat)
        free(oldpat);
    return oldpat = regcomp(str);
}

/* regerror.c */
void regerror( char *s )
{
    fprintf( stderr, "ed: %s\n", s );
}


set()
{
    char    word[16];
    int i;

    if(*(++inptr) != 't') {
        if(*inptr != SP && *inptr != HT && *inptr != NL)
            return(ERR);
    } else
        inptr++;

    if ( (*inptr == NL)
#ifdef __TURBOC__
        || (*inptr == CR)
#endif
       )
    {
        printf("ed version %d.%d\n", version/100, version%100);
        printf( "number %s, list %s\n",
            nflg?"ON":"OFF",
            lflg?"ON":"OFF");
        return(0);
    }

    Skip_White_Space;
    for(i = 0; *inptr != SP && *inptr != HT && *inptr != NL;)
        word[i++] = *inptr++;
    word[i] = EOS;
    for(t = tbl; t->t_str; t++) {
        if(strcmp(word,t->t_str) == 0) {
            *t->t_ptr = t->t_val;
            return(0);
        }
    }
    return SET_FAIL;
}

#ifndef relink
void relink(a, x, y, b)
LINE    *a, *x, *y, *b;
{
    x->l_prev = a;
    y->l_next = b;
}
#endif


void set_ed_buf(void)
{
    relink(&Line0, &Line0, &Line0, &Line0);
    CurLn = LastLn = 0;
    CurPtr = &Line0;
}


/*  subst.c */

int subst(pat, sub, gflg, pflag)
regexp  *pat;
char    *sub;
int gflg, pflag;
{
    int nchngd = 0;
    char    *txtptr;
    char    *new, buf[MAXLINE];
    int still_running = 1;
    LINE    *lastline = getptr( Line2 );

    if(Line1 <= 0)
        return( SUB_FAIL );
    nchngd = 0;     /* reset count of lines changed */

    for( setCurLn( prevln( Line1 ) ); still_running; ) {
        nextCurLn();
        new = buf;
        if ( CurPtr == lastline )
            still_running = 0;
        if ( regexec( pat, txtptr = gettxtl( CurPtr ) ) ) {
            do
                {
                /* Copy leading text */
                int diff = pat->startp[0] - txtptr;
                strncpy( new, txtptr, diff );
                new += diff;
                /* Do substitution */
                new = regsub( pat, sub, new );
                txtptr = pat->endp[0];
                }
            while( gflg && !pat->reganch && regexec( pat, txtptr ));

            /* Copy trailing chars */
            while( *txtptr ) *new++ = *txtptr++;

            if(new >= buf+MAXLINE)
                return( SUB_FAIL );
            *new++ = EOS;
            del(CurLn,CurLn);
            if( ins(buf) < 0 )
                return MEM_FAIL;
            nchngd++;
            if(pflag)
                doprnt(CurLn, CurLn);
        }
        }
    return (( nchngd == 0 && !gflg ) ? SUB_FAIL : nchngd);
}


/*  system.c    */
#ifndef __TURBOC__

#define SHELL   "/bin/sh"

system(c)
char *c; {
    int pid, status;

    switch (pid = fork()) {
    case -1:
        return -1;
    case 0:
        execl(SHELL, "sh", "-c", c, (char *) 0);
        exit(-1);
    default:
        while (wait(&status) != pid)
            ;
    }
    return status;
}
#endif  /* ifndef __TURBOC__ */


/*  docmd.c
 *  Perform the command specified in the input buffer, as pointed to
 *  by inptr.  Actually, this finds the command letter first.
 */

int docmd(glob)
int glob;
{
    static char rhs[MAXPAT];
    regexp  *subpat;
    int c, err, line3;
    int apflg, pflag, gflag;
    int nchng;
    char    *fptr;

    pflag = FALSE;
    Skip_White_Space;

    c = *inptr++;
    switch(c) {
    case NL:
        if( nlines == 0 && (Line2 = nextln(CurLn)) == 0 )
            return(ERR);
        setCurLn( Line2 );
        return (1);

    case '=':
        printf("%d\n",Line2);
        break;

    case 'a':
        if(*inptr != NL || nlines > 1)
            return(ERR);

        if(append(Line1, glob) < 0)
            return(ERR);
        fchanged = TRUE;
        break;

    case 'c':
        if(*inptr != NL)
            return(ERR);

        if(deflt(CurLn, CurLn) < 0)
            return(ERR);

        if(del(Line1, Line2) < 0)
            return(ERR);
        if(append(CurLn, glob) < 0)
            return(ERR);
        fchanged = TRUE;
        break;

    case 'd':
        if(*inptr != NL)
            return(ERR);

        if(deflt(CurLn, CurLn) < 0)
            return(ERR);

        if(del(Line1, Line2) < 0)
            return(ERR);
        if(nextln(CurLn) != 0)
            nextCurLn();
        fchanged = TRUE;
        break;

    case 'e':
        if(nlines > 0)
            return(ERR);
        if(fchanged)
            return CHANGED;
        /*FALL THROUGH*/
    case 'E':
        if(nlines > 0)
            return(ERR);

        if(*inptr != ' ' && *inptr != HT && *inptr != NL)
            return(ERR);

        if((fptr = getfn()) == NULL)
            return(ERR);

        clrbuf();
        if((err = doread(0, fptr)) < 0)
            return(err);

        strcpy(fname, fptr);
        fchanged = FALSE;
        break;

    case 'f':
        if(nlines > 0)
            return(ERR);

        if(*inptr != ' ' && *inptr != HT && *inptr != NL)
            return(ERR);

        if((fptr = getfn()) == NULL)
            return(ERR);

        if (nofname)
            printf("%s\n", fname);
        else
            strcpy(fname, fptr);
        break;

    case 'i':
        if(*inptr != NL || nlines > 1)
            return(ERR);

        if(append(prevln(Line1), glob) < 0)
            return(ERR);
        fchanged = TRUE;
        break;

    case 'j':
        if (*inptr != NL || deflt(CurLn, CurLn+1)<0)
            return(ERR);

        if (join(Line1, Line2) < 0)
            return(ERR);
        break;

    case 'k':
        Skip_White_Space;

        if (*inptr < 'a' || *inptr > 'z')
            return ERR;
        c= *inptr++;

        if(*inptr != ' ' && *inptr != HT && *inptr != NL)
            return(ERR);

        mark[c-'a'] = Line1;
        break;

    case 'l':
        if(*inptr != NL)
            return(ERR);
        if(deflt(CurLn,CurLn) < 0)
            return(ERR);
        if (dolst(Line1,Line2) < 0)
            return(ERR);
        break;

    case 'm':
        if((line3 = getone()) < 0)
            return(ERR);
        if(deflt(CurLn,CurLn) < 0)
            return(ERR);
        if(move(line3) < 0)
            return(ERR);
        fchanged = TRUE;
        break;

    case 'P':
    case 'p':
        if(*inptr != NL)
            return(ERR);
        if(deflt(CurLn,CurLn) < 0)
            return(ERR);
        if(doprnt(Line1,Line2) < 0)
            return(ERR);
        break;

    case 'q':
        if(fchanged)
            return CHANGED;
        /*FALL THROUGH*/
    case 'Q':
        if(*inptr == NL && nlines == 0 && !glob)
            return(EOF);
        else
            return(ERR);

    case 'r':
        if(nlines > 1)
            return(ERR);

        if(nlines == 0)         /* The original code tested */
            Line2 = LastLn;     /*  if(nlines = 0)      */
                        /* which looks wrong.  RAM  */

        if(*inptr != ' ' && *inptr != HT && *inptr != NL)
            return(ERR);

        if((fptr = getfn()) == NULL)
            return(ERR);

        if((err = doread(Line2, fptr)) < 0)
            return(err);
        fchanged = TRUE;
        break;

    case 's':
        if(*inptr == 'e')
            return(set());
        Skip_White_Space;
        if((subpat = optpat()) == NULL)
            return(ERR);
        if((gflag = getrhs(rhs)) < 0)
            return(ERR);
        if(*inptr == 'p')
            pflag++;
        if(deflt(CurLn, CurLn) < 0)
            return(ERR);
        if((nchng = subst(subpat, rhs, gflag, pflag)) < 0)
            return(ERR);
        if(nchng)
            fchanged = TRUE;
        break;

    case 't':
        if((line3 = getone()) < 0)
            return(ERR);
        if(deflt(CurLn,CurLn) < 0)
            return(ERR);
        if(transfer(line3) < 0)
            return(ERR);
        fchanged = TRUE;
        break;

    case 'W':
    case 'w':
        apflg = (c=='W');

        if(*inptr != ' ' && *inptr != HT && *inptr != NL)
            return(ERR);

        if((fptr = getfn()) == NULL)
            return(ERR);

        if(deflt(1, LastLn) < 0)
            return(ERR);
        if(dowrite(Line1, Line2, fptr, apflg) < 0)
            return(ERR);
        fchanged = FALSE;
        break;

    case 'x':
        if(*inptr == NL && nlines == 0 && !glob) {
            if((fptr = getfn()) == NULL)
                return(ERR);
            if(dowrite(1, LastLn, fptr, 0) >= 0)
                return(EOF);
        }
        return(ERR);

    case 'z':
        if(deflt(CurLn,CurLn) < 0)
            return(ERR);

        switch(*inptr) {
        case '-':
            if(doprnt(Line1-21,Line1) < 0)
                return(ERR);
            break;

        case '.':
            if(doprnt(Line1-11,Line1+10) < 0)
                return(ERR);
            break;

        case '+':
        case '\n':
            if(doprnt(Line1,Line1+21) < 0)
                return(ERR);
            break;
        }
        break;

    default:
        return(ERR);
    }
    return (0);
}  /* docmd */


/*  doglob.c    */
doglob()
{
    int lin, stat;
    char    *cmd;
    LINE    *ptr;

    cmd = inptr;

    for (;;) {
        ptr = getptr(1);
        for (lin=1; lin<=LastLn; lin++) {
            if (ptr->l_stat & LGLOB)
                break;
            ptr = getnextptr(ptr);
        }
        if (lin > LastLn)
            break;

        ptr->l_stat &= ~LGLOB;
        CurLn = lin; CurPtr = ptr;
        inptr = cmd;
        if((stat = getlst()) < 0)
            return(stat);
        if((stat = docmd(1)) < 0)
            return(stat);
    }
    return(CurLn);
}  /* doglob */


/*
 *  Software signal 2 or SIGINT is caught and the result is to resume
 *  the main loop at a command prompt.
 */
#include <setjmp.h>
jmp_buf env;

#ifndef __TURBOC__
intr()
{
    puts("intr()?");
    longjmp(env, 1);
}

#else

void Catcher(void)
{
    longjmp(env, 1);
}
#endif  /* !__TURBOC__ */


/*--------  main  ---------------------------------------------------------*/
#ifndef _Cdecl
# define _Cdecl
#endif

void _Cdecl main(argc,argv)
int argc;
char    **argv;
{
    int stat, i, doflush;

    set_ed_buf();
    doflush=isatty(1);

    if (argc>1 && argv[1][0]=='-' && argv[1][1]==0) {
        diag = 0;
        argc--;
        argv++;
    }
    if(argc > 1) {
        for(i = 1; i < argc; i++) {
            if(doread(0,argv[i])==0) {
                setCurLn( 1 );
                strcpy(fname, argv[i]);
                break;
            }
        }
    }

    for (;;) {

        setjmp(env);
        putchar(':');       /*  The command-line prompt  */

#ifndef __TURBOC__
        signal(2, intr);
#else   /* __TURBOC__ */
        signal(SIGINT, Catcher);
#endif  /* !__TURBOC__ */

        if (doflush)
            fflush(stdout);
        if (fgets(inlin, sizeof(inlin),stdin) == NULL) {
            puts("Null input.");
            break;
        }
        if(*inlin == '!') {
            for(inptr = inlin; *inptr != NL; inptr++)
                ;
            *inptr = EOS;
            system(inlin+1);
            continue;
        }
        inptr = inlin;
        if(getlst() >= 0)
            if((stat = ckglob()) != 0) {
                if(stat >= 0 && (stat = doglob()) >= 0) {
                    setCurLn( stat );
                    continue;
                }
            } else {
                if((stat = docmd(0)) >= 0) {
                    if(stat == 1)
                        doprnt(CurLn, CurLn);
                    continue;
                }
            }
        switch (stat) {
        case EOF:
            _cleanup(); exit(0);
        case FATAL:
            fputs("FATAL ERROR\n",stderr);
            _cleanup(); exit(1);
        case CHANGED:
            puts("File has been changed.");
            break;
        case SET_FAIL:
            puts("`set' command failed.");
            break;
        case SUB_FAIL:
            puts("string substitution failed.");
            break;
        case MEM_FAIL:
            puts("Out of memory: text may have been lost." );
            break;
        default:
            puts("Oops?");
            /*  Unrecognized or failed command (this  */
            /*  is SOOOO much better than "?" :-)     */
        }
    }
}  /* main */
/*________  end of source code  ____________________________________________*/
