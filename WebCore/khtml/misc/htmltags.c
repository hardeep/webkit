/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -a -L ANSI-C -D -E -C -l -o -t -k '*' -NfindTag -Hhash_tag -Wwordlist_tag htmltags.gperf  */
/* This file is automatically generated from htmltags.in by maketags, do not edit */
/* Copyright 1999 Lars Knoll */
#include "htmltags.h"
struct tags {
    const char *name;
    int id;
};
/* maximum key range = 395, duplicates = 1 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_tag (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396,   0, 396, 396, 396,  30,
       40,  50,  55,  10,  20, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396,   0, 105, 115,
       35,   0,  65, 100,  10,  10,   0,  35,  45,  25,
       20,   0,  20,  25,   5,  30,   0,  60,  10,   5,
       20,  40, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396, 396, 396, 396, 396,
      396, 396, 396, 396, 396, 396
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#endif
const struct tags *
findTag (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 105,
      MIN_WORD_LENGTH = 1,
      MAX_WORD_LENGTH = 10,
      MIN_HASH_VALUE = 1,
      MAX_HASH_VALUE = 395
    };

  static const unsigned char lengthtable[] =
    {
       1,  2,  2,  4,  1,  2,  2,  3,  1,  2,  1,  2,  3,  4,
       1,  2,  8,  2,  2,  2,  2,  3,  4,  5,  2,  3,  5,  6,
       2,  3,  5,  1,  2,  3,  2,  3,  5,  2,  4,  4,  2,  3,
       4,  6,  4,  6,  5,  4,  5,  1,  2,  4,  6,  2,  3,  4,
       5,  7,  3,  5,  7,  3,  9,  4,  7,  8,  4,  3,  4,  5,
       7,  3,  6,  5,  8,  4,  5,  6,  6,  3,  5,  7,  3,  4,
       5,  6,  6,  7,  8,  6,  3,  5,  6,  6,  8,  7,  8,  3,
       4,  7,  6,  8,  8, 10, 10
    };
  static const struct tags wordlist_tag[] =
    {
      {"a", ID_A},
      {"tt", ID_TT},
      {"tr", ID_TR},
      {"area", ID_AREA},
      {"i", ID_I},
      {"th", ID_TH},
      {"hr", ID_HR},
      {"var", ID_VAR},
      {"p", ID_P},
      {"h5", ID_H5},
      {"q", ID_Q},
      {"em", ID_EM},
      {"pre", ID_PRE},
      {"meta", ID_META},
      {"s", ID_S},
      {"h6", ID_H6},
      {"textarea", ID_TEXTAREA},
      {"dt", ID_DT},
      {"td", ID_TD},
      {"h1", ID_H1},
      {"ol", ID_OL},
      {"map", ID_MAP},
      {"head", ID_HEAD},
      {"thead", ID_THEAD},
      {"h2", ID_H2},
      {"dir", ID_DIR},
      {"param", ID_PARAM},
      {"option", ID_OPTION},
      {"li", ID_LI},
      {"div", ID_DIV},
      {"title", ID_TITLE},
      {"u", ID_U},
      {"h3", ID_H3},
      {"ins", ID_INS},
      {"h4", ID_H4},
      {"xmp", ID_XMP},
      {"tfoot", ID_TFOOT},
      {"dd", ID_DD},
      {"span", ID_SPAN},
      {"samp", ID_SAMP},
      {"dl", ID_DL},
      {"del", ID_DEL},
      {"html", ID_HTML},
      {"strike", ID_STRIKE},
      {"font", ID_FONT},
      {"applet", ID_APPLET},
      {"layer", ID_LAYER},
      {"form", ID_FORM},
      {"frame", ID_FRAME},
      {"b", ID_B},
      {"ul", ID_UL},
      {"menu", ID_MENU},
      {"iframe", ID_IFRAME},
      {"br", ID_BR},
      {"sup", ID_SUP},
      {"link", ID_LINK},
      {"input", ID_INPUT},
      {"nolayer", ID_NOLAYER},
      {"wbr", ID_WBR},
      {"style", ID_STYLE},
      {"marquee", ID_MARQUEE},
      {"dfn", ID_DFN},
      {"plaintext", ID_PLAINTEXT},
      {"cite", ID_CITE},
      {"isindex", ID_ISINDEX},
      {"frameset", ID_FRAMESET},
      {"nobr", ID_NOBR},
      {"img", ID_IMG},
      {"base", ID_BASE},
      {"image", ID_IMG},
      {"address", ID_ADDRESS},
      {"bdo", ID_BDO},
      {"center", ID_CENTER},
      {"small", ID_SMALL},
      {"noframes", ID_NOFRAMES},
      {"code", ID_CODE},
      {"table", ID_TABLE},
      {"anchor", ID_A},
      {"strong", ID_STRONG},
      {"col", ID_COL},
      {"embed", ID_EMBED},
      {"caption", ID_CAPTION},
      {"kbd", ID_KBD},
      {"body", ID_BODY},
      {"tbody", ID_TBODY},
      {"script", ID_SCRIPT},
      {"button", ID_BUTTON},
      {"noembed", ID_NOEMBED},
      {"fieldset", ID_FIELDSET},
      {"select", ID_SELECT},
      {"sub", ID_SUB},
      {"label", ID_LABEL},
      {"keygen", ID_KEYGEN},
      {"legend", ID_LEGEND},
      {"noscript", ID_NOSCRIPT},
      {"acronym", ID_ACRONYM},
      {"optgroup", ID_OPTGROUP},
      {"big", ID_BIG},
      {"abbr", ID_ABBR},
      {"listing", ID_PRE},
      {"object", ID_OBJECT},
      {"basefont", ID_BASEFONT},
      {"colgroup", ID_COLGROUP},
      {"-konqblock", ID__KONQBLOCK},
      {"blockquote", ID_BLOCKQUOTE}
    };

  static const short lookup[] =
    {
        -1,    0,    1,   -1,   -1,   -1,   -1,    2,
        -1,    3,   -1,    4,    5,   -1,   -1,   -1,
        -1,    6,    7,   -1,   -1,    8,    9,   -1,
        -1,   -1,   10,   11,   12,   13,   -1,   14,
        15,   16,   -1,   -1,   -1, -144,  -88,   -2,
        -1,   -1,   19,   -1,   -1,   -1,   -1,   20,
        21,   22,   23,   -1,   24,   25,   -1,   26,
        27,   28,   29,   -1,   30,   31,   32,   33,
        -1,   -1,   -1,   34,   35,   -1,   36,   -1,
        37,   -1,   38,   -1,   -1,   -1,   -1,   39,
        -1,   -1,   40,   41,   42,   -1,   43,   -1,
        -1,   44,   -1,   45,   -1,   -1,   -1,   46,
        -1,   -1,   -1,   47,   48,   -1,   -1,   -1,
        -1,   -1,   49,   50,   -1,   51,   -1,   52,
        53,   54,   55,   56,   -1,   57,   58,   -1,
        59,   -1,   60,   61,   62,   -1,   -1,   -1,
        -1,   63,   -1,   -1,   64,   65,   66,   -1,
        -1,   -1,   67,   68,   69,   -1,   70,   71,
        -1,   -1,   72,   -1,   -1,   -1,   73,   -1,
        -1,   74,   75,   76,   77,   -1,   -1,   -1,
        -1,   78,   -1,   79,   -1,   -1,   -1,   -1,
        -1,   -1,   80,   -1,   81,   -1,   -1,   -1,
        -1,   -1,   82,   -1,   -1,   -1,   -1,   -1,
        83,   84,   85,   -1,   -1,   -1,   -1,   86,
        87,   88,   -1,   -1,   89,   -1,   90,   -1,
        91,   92,   -1,   -1,   -1,   -1,   93,   -1,
        94,   -1,   -1,   -1,   95,   96,   -1,   -1,
        -1,   -1,   97,   98,   -1,   -1,   99,   -1,
        -1,   -1,  100,   -1,  101,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,  102,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
        -1,   -1,   -1,   -1,   -1,   -1,  103,   -1,
        -1,   -1,   -1,  104
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash_tag (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              if (len == lengthtable[index])
                {
                  register const char *s = wordlist_tag[index].name;

                  if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                    return &wordlist_tag[index];
                }
            }
          else if (index < -TOTAL_KEYWORDS)
            {
              register int offset = - 1 - TOTAL_KEYWORDS - index;
              register const unsigned char *lengthptr = &lengthtable[TOTAL_KEYWORDS + lookup[offset]];
              register const struct tags *wordptr = &wordlist_tag[TOTAL_KEYWORDS + lookup[offset]];
              register const struct tags *wordendptr = wordptr + -lookup[offset + 1];

              while (wordptr < wordendptr)
                {
                  if (len == *lengthptr)
                    {
                      register const char *s = wordptr->name;

                      if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
                        return wordptr;
                    }
                  lengthptr++;
                  wordptr++;
                }
            }
        }
    }
  return 0;
}


static const char * const tagList[] = {
"",
    "A",
    "ABBR",
    "ACRONYM",
    "ADDRESS",
    "APPLET",
    "AREA",
    "B",
    "BASE",
    "BASEFONT",
    "BDO",
    "BIG",
    "BLOCKQUOTE",
    "BODY",
    "BR",
    "BUTTON",
    "CAPTION",
    "CENTER",
    "CITE",
    "CODE",
    "COL",
    "COLGROUP",
    "DD",
    "DEL",
    "DFN",
    "DIR",
    "DIV",
    "DL",
    "DT",
    "EM",
    "EMBED",
    "FIELDSET",
    "FONT",
    "FORM",
    "FRAME",
    "FRAMESET",
    "H1",
    "H2",
    "H3",
    "H4",
    "H5",
    "H6",
    "HEAD",
    "HR",
    "HTML",
    "I",
    "IFRAME",
    "IMG",
    "INPUT",
    "INS",
    "ISINDEX",
    "KBD",
    "KEYGEN",
    "-KONQBLOCK",
    "LABEL",
    "LAYER",
    "LEGEND",
    "LI",
    "LINK",
    "MAP",
    "MARQUEE",
    "MENU",
    "META",
    "NOBR",
    "NOEMBED",
    "NOFRAMES",
    "NOSCRIPT",
    "NOLAYER",
    "OBJECT",
    "OL",
    "OPTGROUP",
    "OPTION",
    "P",
    "PARAM",
    "PLAINTEXT",
    "PRE",
    "Q",
    "S",
    "SAMP",
    "SCRIPT",
    "SELECT",
    "SMALL",
    "SPAN",
    "STRIKE",
    "STRONG",
    "STYLE",
    "SUB",
    "SUP",
    "TABLE",
    "TBODY",
    "TD",
    "TEXTAREA",
    "TFOOT",
    "TH",
    "THEAD",
    "TITLE",
    "TR",
    "TT",
    "U",
    "UL",
    "VAR",
    "WBR",
    "XMP",
"TEXT",
"COMMENT",
    "/A",
    "/ABBR",
    "/ACRONYM",
    "/ADDRESS",
    "/APPLET",
    "/AREA",
    "/B",
    "/BASE",
    "/BASEFONT",
    "/BDO",
    "/BIG",
    "/BLOCKQUOTE",
    "/BODY",
    "/BR",
    "/BUTTON",
    "/CAPTION",
    "/CENTER",
    "/CITE",
    "/CODE",
    "/COL",
    "/COLGROUP",
    "/DD",
    "/DEL",
    "/DFN",
    "/DIR",
    "/DIV",
    "/DL",
    "/DT",
    "/EM",
    "/EMBED",
    "/FIELDSET",
    "/FONT",
    "/FORM",
    "/FRAME",
    "/FRAMESET",
    "/H1",
    "/H2",
    "/H3",
    "/H4",
    "/H5",
    "/H6",
    "/HEAD",
    "/HR",
    "/HTML",
    "/I",
    "/IFRAME",
    "/IMG",
    "/INPUT",
    "/INS",
    "/ISINDEX",
    "/KBD",
    "/KEYGEN",
    "/-KONQBLOCK",
    "/LABEL",
    "/LAYER",
    "/LEGEND",
    "/LI",
    "/LINK",
    "/MAP",
    "/MARQUEE",
    "/MENU",
    "/META",
    "/NOBR",
    "/NOEMBED",
    "/NOFRAMES",
    "/NOSCRIPT",
    "/NOLAYER",
    "/OBJECT",
    "/OL",
    "/OPTGROUP",
    "/OPTION",
    "/P",
    "/PARAM",
    "/PLAINTEXT",
    "/PRE",
    "/Q",
    "/S",
    "/SAMP",
    "/SCRIPT",
    "/SELECT",
    "/SMALL",
    "/SPAN",
    "/STRIKE",
    "/STRONG",
    "/STYLE",
    "/SUB",
    "/SUP",
    "/TABLE",
    "/TBODY",
    "/TD",
    "/TEXTAREA",
    "/TFOOT",
    "/TH",
    "/THEAD",
    "/TITLE",
    "/TR",
    "/TT",
    "/U",
    "/UL",
    "/VAR",
    "/WBR",
    "/XMP",
    0
};
DOMString getTagName(unsigned short id)
{
    if(id > ID_CLOSE_TAG*2) id = ID_CLOSE_TAG+1;
    return DOMString(tagList[id]);
};
