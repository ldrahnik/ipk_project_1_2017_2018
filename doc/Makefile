doc.dvi: dokumentace.tex Makefile local.bib
	latex $<
	bibtex dokumentace
	latex $<
	latex $<

%.ps: %.dvi
	dvips -f -t a4 $< > $@

%.pdf: %.ps
	ps2pdf14 -sPAPERSIZE=letter $<

clean:
	rm -f *.aux *.dvi *.log *.blg *.bbl *.pdf *.ps
