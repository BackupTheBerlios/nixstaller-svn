/*
    Copyright (C) 2006 by Rick Helmus (rhelmus_AT_gmail.com)
    Copyright (C) 1994, 2003 Free Software Foundation, Inc.

    This file is part of Nixstaller.

    Nixstaller is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Nixstaller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nixstaller; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Linking cdk statically or dynamically with other modules is making a combined work based on cdk. Thus, the terms and
    conditions of the GNU General Public License cover the whole combination.

    In addition, as a special exception, the copyright holders of cdk give you permission to combine cdk program with free
    software programs or libraries that are released under the GNU LGPL and with code included in the standard release of
    DEF under the XYZ license (or modified versions of such code, with unchanged license). You may copy and distribute
    such a system following the terms of the GNU GPL for cdk and the licenses of the other code concerned, provided that
    you include the source code of that other code when and as the GNU GPL requires distribution of source code.

    Note that people who make modified versions of cdk are not obligated to grant this special exception for their modified
    versions; it is their choice whether to do so. The GNU General Public License gives permission to release a modified
    version without this exception; this exception also makes it possible to release a modified version which carries forward
    this exception.
*/

/* File copied(and modified) from 'libiberty' from the GCC project. */

#ifndef HAVE_VASPRINTF

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef TEST
int global_total_width;
#endif

/*

@deftypefn Extension int vasprintf (char **@var{resptr}, const char *@var{format}, va_list @var{args})

Like @code{vsprintf}, but instead of passing a pointer to a buffer,
you pass a pointer to a pointer.  This function will compute the size
of the buffer needed, allocate memory with @code{malloc}, and store a
pointer to the allocated memory in @code{*@var{resptr}}.  The value
returned is the same as @code{vsprintf} would return.  If memory could
not be allocated, minus one is returned and @code{NULL} is stored in
@code{*@var{resptr}}.

@end deftypefn

*/

static int
int_vasprintf (result, format, args)
     char **result;
     const char *format;
     va_list args;
{
  const char *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = strlen (format) + 1;
  va_list ap;

#ifdef va_copy
  va_copy (ap, args);
#else
  memcpy ((PTR) &ap, (PTR) &args, sizeof (va_list));
#endif

  while (*p != '\0')
    {
      if (*p++ == '%')
	{
	  while (strchr ("-+ #0", *p))
	    ++p;
	  if (*p == '*')
	    {
	      ++p;
	      total_width += abs (va_arg (ap, int));
	    }
	  else
	    total_width += strtoul (p, (char **) &p, 10);
	  if (*p == '.')
	    {
	      ++p;
	      if (*p == '*')
		{
		  ++p;
		  total_width += abs (va_arg (ap, int));
		}
	      else
	      total_width += strtoul (p, (char **) &p, 10);
	    }
	  while (strchr ("hlL", *p))
	    ++p;
	  /* Should be big enough for any format specifier except %s and floats.  */
	  total_width += 30;
	  switch (*p)
	    {
	    case 'd':
	    case 'i':
	    case 'o':
	    case 'u':
	    case 'x':
	    case 'X':
	    case 'c':
	      (void) va_arg (ap, int);
	      break;
	    case 'f':
	    case 'e':
	    case 'E':
	    case 'g':
	    case 'G':
	      (void) va_arg (ap, double);
	      /* Since an ieee double can have an exponent of 307, we'll
		 make the buffer wide enough to cover the gross case. */
	      total_width += 307;
	      break;
	    case 's':
	      total_width += strlen (va_arg (ap, char *));
	      break;
	    case 'p':
	    case 'n':
	      (void) va_arg (ap, char *);
	      break;
	    }
	  p++;
	}
    }
#ifdef va_copy
  va_end (ap);
#endif
#ifdef TEST
  global_total_width = total_width;
#endif
  *result = (char *) malloc (total_width);
  if (*result != NULL)
    return vsprintf (*result, format, args);
  else
    return -1;
}

int
vasprintf (result, format, args)
     char **result;
     const char *format;
     va_list args;
{
  return int_vasprintf (result, format, args);
}

#ifdef TEST
static void ATTRIBUTE_PRINTF_1
checkit VPARAMS ((const char *format, ...))
{
  char *result;
  VA_OPEN (args, format);
  VA_FIXEDARG (args, const char *, format);
  vasprintf (&result, format, args);
  VA_CLOSE (args);

  if (strlen (result) < (size_t) global_total_width)
    printf ("PASS: ");
  else
    printf ("FAIL: ");
  printf ("%d %s\n", global_total_width, result);

  free (result);
}

extern int main PARAMS ((void));

int
main ()
{
  checkit ("%d", 0x12345678);
  checkit ("%200d", 5);
  checkit ("%.300d", 6);
  checkit ("%100.150d", 7);
  checkit ("%s", "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
777777777777777777333333333333366666666666622222222222777777777777733333");
  checkit ("%f%s%d%s", 1.0, "foo", 77, "asdjffffffffffffffiiiiiiiiiiixxxxx");

  return 0;
}
#endif /* TEST */

#endif /* HAVE_VASPRINTF */
