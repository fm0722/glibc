/* Test strlen functions.
   Copyright (C) 1999-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jakub Jelinek <jakub@redhat.com>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define TEST_MAIN
#ifndef WIDE
# define TEST_NAME "strnlen"
#else
# define TEST_NAME "wcsnlen"
#endif /* !WIDE */
#include "test-string.h"

#ifndef WIDE
# define STRNLEN strnlen
# define CHAR char
# define BIG_CHAR CHAR_MAX
# define MIDDLE_CHAR 127
# define SIMPLE_STRNLEN simple_strnlen
#else
# include <wchar.h>
# define STRNLEN wcsnlen
# define CHAR wchar_t
# define BIG_CHAR WCHAR_MAX
# define MIDDLE_CHAR 1121
# define SIMPLE_STRNLEN simple_wcsnlen
#endif /* !WIDE */

typedef size_t (*proto_t) (const CHAR *, size_t);
size_t SIMPLE_STRNLEN (const CHAR *, size_t);

IMPL (SIMPLE_STRNLEN, 0)
IMPL (STRNLEN, 1)

size_t
SIMPLE_STRNLEN (const CHAR *s, size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen && s[i]; ++i);
  return i;
}

static void
do_one_test (impl_t *impl, const CHAR *s, size_t maxlen, size_t exp_len)
{
  size_t len = CALL (impl, s, maxlen);
  if (len != exp_len)
    {
      error (0, 0, "Wrong result in function %s %zd %zd", impl->name,
	     len, exp_len);
      ret = 1;
      return;
    }
}

static void
do_test (size_t align, size_t len, size_t maxlen, int max_char)
{
  size_t i;

  align &= 63;
  if ((align + len) * sizeof (CHAR) >= page_size)
    return;

  CHAR *buf = (CHAR *) (buf1);

  for (i = 0; i < len; ++i)
    buf[align + i] = 1 + 11111 * i % max_char;
  buf[align + len] = 0;

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, (CHAR *) (buf + align), maxlen, MIN (len, maxlen));
}

static void
do_random_tests (void)
{
  size_t i, j, n, align, len;
  CHAR *p = (CHAR *) (buf1 + page_size - 512 * sizeof (CHAR));

  for (n = 0; n < ITERATIONS; n++)
    {
      align = random () & 15;
      len = random () & 511;
      if (len + align > 510)
	len = 511 - align - (random () & 7);
      j = len + align + 64;
      if (j > 512)
	j = 512;

      for (i = 0; i < j; i++)
	{
	  if (i == len + align)
	    p[i] = 0;
	  else
	    {
	      p[i] = random () & 255;
	      if (i >= align && i < len + align && !p[i])
		p[i] = (random () & 127) + 1;
	    }
	}

      FOR_EACH_IMPL (impl, 1)
	{
	  if (len > 0
	      && CALL (impl, (CHAR *) (p + align), len - 1) != len - 1)
	    {
	      error (0, 0, "Iteration %zd (limited) - wrong result in function %s (%zd) %zd != %zd, p %p",
		     n, impl->name, align,
		     CALL (impl, (CHAR *) (p + align), len - 1), len - 1, p);
	      ret = 1;
	    }
	  if (CALL (impl, (CHAR *) (p + align), len) != len)
	    {
	      error (0, 0, "Iteration %zd (exact) - wrong result in function %s (%zd) %zd != %zd, p %p",
		     n, impl->name, align,
		     CALL (impl, (CHAR *) (p + align), len), len, p);
	      ret = 1;
	    }
	  if (CALL (impl, (CHAR *) (p + align), len + 1) != len)
	    {
	      error (0, 0, "Iteration %zd (long) - wrong result in function %s (%zd) %zd != %zd, p %p",
		     n, impl->name, align,
		     CALL (impl, (CHAR *) (p + align), len + 1), len, p);
	      ret = 1;
	    }
	}
    }
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%20s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 1; i < 8; ++i)
    {
      do_test (0, i, i - 1, MIDDLE_CHAR);
      do_test (0, i, i, MIDDLE_CHAR);
      do_test (0, i, i + 1, MIDDLE_CHAR);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, i, i - 1, MIDDLE_CHAR);
      do_test (i, i, i, MIDDLE_CHAR);
      do_test (i, i, i + 1, MIDDLE_CHAR);
    }

  for (i = 2; i <= 10; ++i)
    {
      do_test (0, 1 << i, 5000, MIDDLE_CHAR);
      do_test (1, 1 << i, 5000, MIDDLE_CHAR);
    }

  for (i = 1; i < 8; ++i)
    do_test (0, i, 5000, BIG_CHAR);

  for (i = 1; i < 8; ++i)
    do_test (i, i, 5000, BIG_CHAR);

  for (i = 2; i <= 10; ++i)
    {
      do_test (0, 1 << i, 5000, BIG_CHAR);
      do_test (1, 1 << i, 5000, BIG_CHAR);
    }

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
