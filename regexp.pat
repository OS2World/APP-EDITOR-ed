*** regexp.org
--- regexp.h
**************
*** 15,20
  	char program[1];	/* Unwarranted chumminess with compiler. */
  } regexp;
  
  extern regexp *regcomp();
  extern int regexec();
  extern void regsub();
--- 15,26 -----
  	char program[1];	/* Unwarranted chumminess with compiler. */
  } regexp;
  
+ #ifdef __STDC__
+ extern regexp *regcomp( char * );
+ extern int regexec( regexp*, char* );
+ extern char *regsub( regexp*, char*, char* );
+ extern void regerror( char* );
+ #else
  extern regexp *regcomp();
  extern int regexec();
  extern char *regsub();
**************
*** 17,21
  
  extern regexp *regcomp();
  extern int regexec();
! extern void regsub();
  extern void regerror();
--- 23,28 -----
  #else
  extern regexp *regcomp();
  extern int regexec();
! extern char *regsub();
  extern void regerror();
  #endif
**************
*** 19,21
  extern int regexec();
  extern void regsub();
  extern void regerror();
--- 25,28 -----
  extern int regexec();
  extern char *regsub();
  extern void regerror();
+ #endif
*** regsub.org
--- regsub.c
**************
*** 18,23
   *
   *	3. Altered versions must be plainly marked as such, and must not
   *		be misrepresented as being the original software.
   */
  #include <stdio.h>
  #include <regexp.h>
--- 18,27 -----
   *
   *	3. Altered versions must be plainly marked as such, and must not
   *		be misrepresented as being the original software.
+  *
+  * This version modified by Ian Phillipps to return pointer to terminating
+  * NUL on substitution string. [ Temp mail address ex-igp@camcon.co.uk ]
+  *
   */
  #include <stdio.h>
  #include <regexp.h>
**************
*** 32,38
  /*
   - regsub - perform substitutions after a regexp match
   */
! void
  regsub(prog, source, dest)
  regexp *prog;
  char *source;
--- 36,42 -----
  /*
   - regsub - perform substitutions after a regexp match
   */
! char *
  regsub(prog, source, dest)
  regexp *prog;
  char *source;
**************
*** 47,53
  
  	if (prog == NULL || source == NULL || dest == NULL) {
  		regerror("NULL parm to regsub");
! 		return;
  	}
  	if (UCHARAT(prog->program) != MAGIC) {
  		regerror("damaged regexp fed to regsub");
--- 51,57 -----
  
  	if (prog == NULL || source == NULL || dest == NULL) {
  		regerror("NULL parm to regsub");
! 		return NULL;
  	}
  	if (UCHARAT(prog->program) != MAGIC) {
  		regerror("damaged regexp fed to regsub");
**************
*** 51,57
  	}
  	if (UCHARAT(prog->program) != MAGIC) {
  		regerror("damaged regexp fed to regsub");
! 		return;
  	}
  
  	src = source;
--- 55,61 -----
  	}
  	if (UCHARAT(prog->program) != MAGIC) {
  		regerror("damaged regexp fed to regsub");
! 		return NULL;
  	}
  
  	src = source;
**************
*** 74,80
  			dst += len;
  			if (len != 0 && *(dst-1) == '\0') {	/* strncpy hit NUL. */
  				regerror("damaged match string");
! 				return;
  			}
  		}
  	}
--- 78,84 -----
  			dst += len;
  			if (len != 0 && *(dst-1) == '\0') {	/* strncpy hit NUL. */
  				regerror("damaged match string");
! 				return NULL;
  			}
  		}
  	}
**************
*** 78,82
  			}
  		}
  	}
! 	*dst++ = '\0';
  }
--- 82,87 -----
  			}
  		}
  	}
! 	*dst = '\0';
! 	return dst;
  }
