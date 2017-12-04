cd ..
cp Testes/page_structure_teste.c temp.c
gcc -o page_structure_t temp.c log.c page_structure.c
cp page_structure_t Testes/page_structure_t
rm -f temp.c page_structure_t
cd Testes/
