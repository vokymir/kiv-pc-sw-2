pdflatex --shell-escape doc.tex
echo "No bibtex used."
pdflatex --shell-escape doc.tex
pdflatex --shell-escape doc.tex
mv doc.pdf ../doc.pdf
