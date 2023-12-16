all:
	latex main.tex
	latex main.tex
	dvips -t a4 main.dvi
	ps2pdf -sPAPERSIZE=a4 main.ps
	mv main.pdf dokumentace.pdf
clean:
	rm -rf *.aux *.dvi *.log *.out *.ps *.toc
