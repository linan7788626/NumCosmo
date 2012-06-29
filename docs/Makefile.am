## Process this file with automake to produce Makefile.in

# Files to process with asciidoc
XML_FILES=dependencies.xml \
          compiling.xml \
          introduction.xml \
          references.xml \
          theory.xml

# We require automake 1.6 at least.
AUTOMAKE_OPTIONS = 1.6

# This is a blank Makefile.am for using gtk-doc.
# Copy this to your project's API docs directory and modify the variables to
# suit your project. See the GTK+ Makefiles in gtk+/docs/reference for examples
# of using the various options.

# The name of the module, e.g. 'glib'.
DOC_MODULE=numcosmo

# Uncomment for versioned docs and specify the version of the module, e.g. '2'.
#DOC_MODULE_VERSION=2

# The top-level SGML file. You can change this if you want to.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# Directories containing the source code, relative to $(srcdir).
# gtk-doc will search all .c and .h files beneath these paths
# for inline comments documenting functions and macros.
# e.g. DOC_SOURCE_DIR=../../../gtk ../../../gdk
DOC_SOURCE_DIR=$(top_srcdir)/numcosmo \
	       $(top_srcdir)/numcosmo/data \
	       $(top_srcdir)/numcosmo/likelihood \
	       $(top_srcdir)/numcosmo/lss \
	       $(top_srcdir)/numcosmo/math \
	       $(top_srcdir)/numcosmo/model \
	       $(top_srcdir)/numcosmo/perturbations \
	       $(top_srcdir)/numcosmo/sphere \
	       $(top_srcdir)/numcosmo/thermodyn

DOC_BUILD_DIR=$(top_builddir)/numcosmo
DOC_TARGET_DIR=$(top_builddir)/docs

# Extra options to pass to gtkdoc-scangobj. Not normally needed.
SCANGOBJ_OPTIONS=

# Extra options to supply to gtkdoc-scan.
# e.g. SCAN_OPTIONS=--deprecated-guards="GTK_DISABLE_DEPRECATED"
SCAN_OPTIONS=--rebuild-types --rebuild-sections

# Extra options to supply to gtkdoc-mkdb.
# e.g. MKDB_OPTIONS=--xml-mode --output-format=xml
MKDB_OPTIONS=--sgml-mode --output-format=xml --name-space=nc

# Extra options to supply to gtkdoc-mktmpl
# e.g. MKTMPL_OPTIONS=--only-section-tmpl
MKTMPL_OPTIONS=

# Extra options to supply to gtkdoc-mkhtml
MKHTML_OPTIONS=

# Extra options to supply to gtkdoc-fixref. Not normally needed.
# e.g. FIXXREF_OPTIONS=--extra-dir=../gdk-pixbuf/html --extra-dir=../gdk/html
FIXXREF_OPTIONS=--extra-dir=$(GLIB_PREFIX)/share/gtk-doc/html

# Used for dependencies. The docs will be rebuilt if any of these change.
# e.g. HFILE_GLOB=$(top_srcdir)/gtk/*.h
# e.g. CFILE_GLOB=$(top_srcdir)/gtk/*.c
HFILE_GLOB=$(top_srcdir)/numcosmo/*.h \
	   $(top_srcdir)/numcosmo/data/*.h \
	   $(top_srcdir)/numcosmo/likelihood/*.h \
	   $(top_srcdir)/numcosmo/lss/*.h \
	   $(top_srcdir)/numcosmo/math/*.h \
	   $(top_srcdir)/numcosmo/model/*.h \
	   $(top_srcdir)/numcosmo/perturbations/*.h \
	   $(top_srcdir)/numcosmo/sphere/*.h \
	   $(top_srcdir)/numcosmo/thermodyn/*.h

CFILE_GLOB=$(top_srcdir)/numcosmo/*.c \
	   $(top_srcdir)/numcosmo/data/*.c \
	   $(top_srcdir)/numcosmo/likelihood/*.c \
	   $(top_srcdir)/numcosmo/lss/*.c \
	   $(top_srcdir)/numcosmo/math/*.c \
	   $(top_srcdir)/numcosmo/model/*.c \
	   $(top_srcdir)/numcosmo/perturbations/*.c \
	   $(top_srcdir)/numcosmo/sphere/*.c \
	   $(top_srcdir)/numcosmo/thermodyn/*.c

# Extra header to include when scanning, which are not under DOC_SOURCE_DIR
# e.g. EXTRA_HFILES=$(top_srcdir}/contrib/extra.h
EXTRA_HFILES=

# Header files or dirs to ignore when scanning. Use base file/dir names
# e.g. IGNORE_HFILES=gtkdebug.h gtkintl.h private_code
IGNORE_HFILES=linear_internal.h config.h

# Images to copy into HTML directory.
# e.g. HTML_IMAGES=$(top_srcdir)/gtk/stock-icons/stock_about_24.png
HTML_IMAGES=

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
# e.g. content_files=running.sgml building.sgml changes-2.0.sgml
content_files=$(XML_FILES) references.bib

# SGML files where gtk-doc abbrevations (#GtkWidget) are expanded
# These files must be listed here *and* in content_files
# e.g. expand_content_files=running.sgml
expand_content_files=

# CFLAGS and LDFLAGS for compiling gtkdoc-scangobj with your library.
# Only needed if you are using gtkdoc-scangobj to dynamically query widget
# signals and properties.
# e.g. GTKDOC_CFLAGS=-I$(top_srcdir) -I$(top_builddir) $(GTK_DEBUG_FLAGS)
# e.g. GTKDOC_LIBS=$(top_builddir)/gtk/$(gtktargetlib)
GTKDOC_CFLAGS=$(GLIB_CFLAGS)
GTKDOC_LIBS=$(top_builddir)/numcosmo/libnumcosmo.la

# This includes the standard gtk-doc make rules, copied by gtkdocize.
include $(top_srcdir)/gtk-doc.make

# Other files to distribute
# e.g. EXTRA_DIST += version.xml.in
EXTRA_DIST += $(XML_FILES) references.bib

# Files not to distribute
# for --rebuild-types in $(SCAN_OPTIONS), e.g. $(DOC_MODULE).types
# for --rebuild-sections in $(SCAN_OPTIONS) e.g. $(DOC_MODULE)-sections.txt
DISTCLEANFILES = 

# Comment this out if you want 'make check' to test you doc status
# and run some sanity checks
if ENABLE_GTK_DOC
TESTS_ENVIRONMENT = cd $(srcdir) && \
  DOC_MODULE=$(DOC_MODULE) DOC_MAIN_SGML_FILE=$(DOC_MAIN_SGML_FILE) \
  SRCDIR=$(abs_srcdir) BUILDDIR=$(abs_builddir)
#TESTS = $(GTKDOC_CHECK)
endif

.asciidoc.xml:
	asciidoc -b docbook -d manpage -o $@ $<
	@if test ! "$(DOC_TARGET_DIR)/$(@F)" -ef "$(<D)/$(@F)"; then cp $(DOC_TARGET_DIR)/$(@F) $(<D)/$(@F); fi

.tex.xml:
	@echo "  DOC   Building XML from $(@F)"
	@latex --interaction=nonstopmode $< > $(@F).htlatex.log 2>&1
	@bibtex8 $*                         >> $(@F).htlatex.log 2>&1
	@latex --interaction=nonstopmode $< >> $(@F).htlatex.log 2>&1
	@latex --interaction=nonstopmode $< >> $(@F).htlatex.log 2>&1
	@htlatex $< "xhtml,docbook-mml" " -cunihtf" "-cdocbk" "--interaction=nonstopmode" >> $(@F).htlatex.log 2>&1
	@$(top_srcdir)/docs/xmltogtk.pl $(DOC_TARGET_DIR)/$(@F) > $(@F).xmltogtk.log 2>&1
	@if test ! "$(DOC_TARGET_DIR)/$(@F)" -ef "$(<D)/$(@F)"; then cp $(DOC_TARGET_DIR)/$(@F) $(<D)/$(@F); fi

-include $(top_srcdir)/git.mk