TEX	 := $(wildcard *.tex)
FIGS_ODG := $(wildcard fig/*.odg)
FIGS_PDF := $(patsubst %.odg,%.pdf,$(FIGS_ODG))
GRAPHS	 := 
SVG_PDFS := $(patsubst %.svg,%.pdf,$(wildcard fig/*.svg))
SVG_PDFS_BW := $(patsubst %.svg,%-bw.pdf,$(wildcard fig/*.svg))

all:	main.pdf

fig: $(GRAPHS) $(SVG_PDFS) $(SVG_PDFS_BW)
.PHONY: fig

figclean:
	rm -f $(SVG_PDFS) $(SVG_PDFS_BW) $(GRAPHS)
.PHONY: figclean

main.pdf: $(TEX) $(wildcard *.bib) $(wildcard graph/out/*.pdf)
	pdflatex main
	bibtex -min-crossrefs=100 main
	pdflatex main
	pdflatex main

clean:
	rm -f *.aux, *.blg, *.log, *.bbl, main.pdf, *.out

$(FIGS_PDF): %.pdf: %.odg
	unoconv -f pdf --stdout $< > $@.tmp~
	pdfcrop $@.tmp~ $@
	rm -f $@.tmp~

fig/%.pdf: fig/%.svg
	inkscape -z --export-pdf=$@ --export-area-drawing $<

fig/%.pdf: fig/%.gp fig/%.dat fig/common.gp
	( gnuplot $< \
	  && acroread -toPostScript $@ \
	  && ps2pdf  $(patsubst fig/%.pdf,fig/%.ps,$@) \
		     $(patsubst fig/%.pdf,fig/.x.%.pdf,$@) \
	  && pdfcrop $(patsubst fig/%.pdf,fig/.x.%.pdf,$@) $@ \
	  && rm      $(patsubst fig/%.pdf,fig/%.ps,$@) \
		     $(patsubst fig/%.pdf,fig/.x.%.pdf,$@) \
	  && cp $@ .missing/$@ \
	  && echo NOTE: successfully re-built graph $@ ) \
	|| cp .missing/$@ $@

SPELLTEX := $(shell ./bin/get-tex-files.sh main.tex)
spell:
	@for i in $(SPELLTEX); do aspell --mode=tex \
					 --add-tex-command="newcommand pop" \
					 --add-tex-command="textcolor pp" \
					 -p ./aspell.words -c $$i; done
	@for i in $(SPELLTEX); do perl bin/double.pl $$i; done
	@bash bin/hyphens.sh $(SPELLTEX)
	@( head -1 aspell.words ; tail -n +2 aspell.words | sort ) > aspell.words~
	@mv aspell.words~ aspell.words


