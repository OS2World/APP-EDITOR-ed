abc	abc	y	&	abc
abc	xbc	n	-	-
abc	axc	n	-	-
abc	abx	n	-	-
abc	xabcy	y	&	abc
abc	ababc	y	&	abc
ab*c	abc	y	&	abc
ab*bc	abc	y	&	abc
ab*bc	abbc	y	&	abbc
ab*bc	abbbbc	y	&	abbbbc
ab+bc	abbc	y	&	abbc
ab+bc	abc	n	-	-
ab+bc	abq	n	-	-
ab+bc	abbbbc	y	&	abbbbc
ab?bc	abbc	y	&	abbc
ab?bc	abc	y	&	abc
ab?bc	abbbbc	n	-	-
ab?c	abc	y	&	abc
^abc$	abc	y	&	abc
^abc$	abcc	n	-	-
^abc	abcc	y	&	abc
^abc$	aabc	n	-	-
abc$	aabc	y	&	abc
^	abc	y	&
$	abc	y	&
a.c	abc	y	&	abc
a.c	axc	y	&	axc
a.*c	axyzc	y	&	axyzc
a.*c	axyzd	n	-	-
a[bc]d	abc	n	-	-
a[bc]d	abd	y	&	abd
a[b-d]e	abd	n	-	-
a[b-d]e	ace	y	&	ace
a[b-d]	aac	y	&	ac
a[-b]	a-	y	&	a-
a[b-]	a-	y	&	a-
[k]	ab	n	-	-
a[b-a]	-	c	-	-
a[]b	-	c	-	-
a[	-	c	-	-
a]	a]	y	&	a]
a[]]b	a]b	y	&	a]b
a[^bc]d	aed	y	&	aed
a[^bc]d	abd	n	-	-
a[^-b]c	adc	y	&	adc
a[^-b]c	a-c	n	-	-
a[^]b]c	a]c	n	-	-
a[^]b]c	adc	y	&	adc
ab|cd	abc	y	&	ab
ab|cd	abcd	y	&	ab
()ef	def	y	&-\1	ef-
()*	-	c	-	-
*a	-	c	-	-
^*	-	c	-	-
$*	-	c	-	-
(*)b	-	c	-	-
$b	b	n	-	-
a\	-	c	-	-
a\(b	a(b	y	&-\1	a(b-
a\(*b	ab	y	&	ab
a\(*b	a((b	y	&	a((b
a\\b	a\b	y	&	a\b
abc)	-	c	-	-
(abc	-	c	-	-
((a))	abc	y	&-\1-\2	a-a-a
(a)b(c)	abc	y	&-\1-\2	abc-a-c
a+b+c	aabbabc	y	&	abc
a**	-	c	-	-
a*?	-	c	-	-
(a*)*	-	c	-	-
(a*)+	-	c	-	-
(a|)*	-	c	-	-
(a*|b)*	-	c	-	-
(a+|b)*	ab	y	&-\1	ab-b
(a+|b)+	ab	y	&-\1	ab-b
(a+|b)?	ab	y	&-\1	a-a
[^ab]*	cde	y	&	cde
(^)*	-	c	-	-
(ab|)*	-	c	-	-
)(	-	c	-	-
	abc	y	&
abc		n	-	-
a*		y	&
abcd	abcd	y	&-\&-\\&	abcd-&-\abcd
a(bc)d	abcd	y	\1-\\1-\\\1	bc-\1-\bc
([abc])*d	abbbcd	y	&-\1	abbbcd-c
([abc])*bcd	abcd	y	&-\1	abcd-a
a|b|c|d|e	e	y	&	e
(a|b|c|d|e)f	ef	y	&-\1	ef-e
((a*|b))*	-	c	-	-
abcd*efg	abcdefg	y	&	abcdefg
ab*	xabyabbbz	y	&	ab
ab*	xayabbbz	y	&	a
(ab|cd)e	abcde	y	&-\1	cde-cd
[abhgefdc]ij	hij	y	&	hij
^(ab|cd)e	abcde	n	x\1y	xy
(abc|)ef	abcdef	y	&-\1	ef-
(a|b)c*d	abcd	y	&-\1	bcd-b
(ab|ab*)bc	abc	y	&-\1	abc-a
a([bc]*)c*	abc	y	&-\1	abc-bc
a([bc]*)(c*d)	abcd	y	&-\1-\2	abcd-bc-d
a([bc]+)(c*d)	abcd	y	&-\1-\2	abcd-bc-d
a([bc]*)(c+d)	abcd	y	&-\1-\2	abcd-b-cd
a[bcd]*dcdcde	adcdcde	y	&	adcdcde
a[bcd]+dcdcde	adcdcde	n	-	-
(ab|a)b*c	abc	y	&-\1	abc-ab
((a)(b)c)(d)	abcd	y	\1-\2-\3-\4	abc-a-b-d
[ -~]*	abc	y	&	abc
[ -~ -~]*	abc	y	&	abc
[ -~ -~ -~]*	abc	y	&	abc
[ -~ -~ -~ -~]*	abc	y	&	abc
[ -~ -~ -~ -~ -~]*	abc	y	&	abc
[ -~ -~ -~ -~ -~ -~]*	abc	y	&	abc
[ -~ -~ -~ -~ -~ -~ -~]*	abc	y	&	abc
[a-zA-Z_][a-zA-Z0-9_]*	alpha	y	&	alpha
^a(bc+|b[eh])g|.h$	abh	y	&-\1	bh-
(bc+d$|ef*g.|h?i(j|k))	effgz	y	&-\1-\2	effgz-effgz-
(bc+d$|ef*g.|h?i(j|k))	ij	y	&-\1-\2	ij-ij-j
(bc+d$|ef*g.|h?i(j|k))	effg	n	-	-
(bc+d$|ef*g.|h?i(j|k))	bcdd	n	-	-
(bc+d$|ef*g.|h?i(j|k))	reffgz	y	&-\1-\2	effgz-effgz-
((((((((((a))))))))))	-	c	-	-
(((((((((a)))))))))	a	y	&	a
multiple words of text	uh-uh	n	-	-
multiple words	multiple words, yeah	y	&	multiple words
(.*)c(.*)	abcde	y	&-\1-\2	abcde-ab-de
\((.*), (.*)\)	(a, b)	y	(\2, \1)	(b, a)
