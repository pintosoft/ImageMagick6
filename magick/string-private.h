/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private string methods.
*/
#ifndef MAGICKCORE_STRING_PRIVATE_H
#define MAGICKCORE_STRING_PRIVATE_H

#include "magick/locale_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static inline double SiPrefixToDoubleInterval(const char *string,
  const double interval)
{
  char
    *q;

  double
    value;

  value=InterpretSiPrefixValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline char *StringLocateSubstring(const char *haystack,
  const char *needle)
{
#if defined(MAGICKCORE_HAVE_STRCASESTR)
  return((char *) strcasestr(haystack,needle));
#else
  {
    size_t
      length_needle,
      length_haystack;

    register ssize_t
      i;

    if (!haystack || !needle)
      return(NULL);
    length_needle=strlen(needle);
    length_haystack=strlen(haystack)-length_needle+1;
    for (i=0; i < length_haystack; i++)
    {
      register size_t
        j;

      for (j=0; j < length_needle; j++)
      {
        unsigned char c1 = haystack[i+j];
        unsigned char c2 = needle[j];
        if (toupper(c1) != toupper(c2))
          goto next;
      }
      return((char *) haystack+i);
      next:
       ;
    }
    return((char *) NULL);
  }
#endif
}

static inline double StringToDouble(const char *magick_restrict string,
  char **magick_restrict sentinal)
{
  return(InterpretLocaleValue(string,sentinal));
}

static inline double StringToDoubleInterval(const char *string,
  const double interval)
{
  char
    *q;

  double
    value;

  value=InterpretLocaleValue(string,&q);
  if (*q == '%')
    value*=interval/100.0;
  return(value);
}

static inline int StringToInteger(const char *magick_restrict value)
{
  return((int) strtol(value,(char **) NULL,10));
}

static inline long StringToLong(const char *magick_restrict value)
{
  return(strtol(value,(char **) NULL,10));
}

static inline unsigned long StringToUnsignedLong(
  const char *magick_restrict value)
{
  return(strtoul(value,(char **) NULL,10));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
