changequote(`<%', `%>')dnl
define(<%SECTION_HEADER%>, <%ifelse($2,<%END%>,<%divert(-1)%>,<%divert(1)%>)%>)dnl
define(<%SECTION_HEADER_PRIVATE%>, <%ifelse($2,<%END%>,<%divert(-1)%>,<%divert(2)%>)%>)dnl
define(<%SECTION_IMPLEMENTATION%>, <%ifelse($2,<%END%>,<%divert(-1)%>,<%divert(3)%>)%>)dnl
define(<%SECTION_PREFIX%>, <%ifelse($2,<%END%>,<%divert(-1)%>,<%divert(4)%>)%>)dnl
define(<%SECTION%>, <%ifelse($#,0,<%errprint(<%ERROR: must provide section name\n%>)m4exit(<%1%>)%>,<%SECTION_$1($@)%>)%>)dnl
define(<%include_header_inner%>,<%esyscmd(<%sed "s|^//\s*SECTION(|SECTION(|" $1%>)%>)dnl
define(<%include_header%>,<%include_header_inner(include_/$1)%>)dnl
divert(-1)dnl
include_header(include.h)
include_header(core.h)
divert(0)dnl
#ifndef HH__
#define HH__
undivert(1)
//
//
//

//
//
//

//
//
//

//
//
//
undivert(2)
#ifdef HH_IMPLEMENTATION
undivert(3)dnl
#endif // HH_IMPLEMENTATION
#endif // HH__
#ifndef HH__STRIP_PREFIXES
#define HH__STRIP_PREFIXES
#ifdef HH_STRIP_PREFIXES
undivert(4)dnl
#endif // HH_STRIP_PREFIXES
#endif // not HH__STRIP_PREFIXES
