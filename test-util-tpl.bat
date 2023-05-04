@set TESTEXE=.out\msvc2019\x64\Debug\umba-enum-gen.exe

@if not exist test_res mkdir tests
@if not exist test_res mkdir test_res

%TESTEXE% --options=0 --options=type-decl,enum-class,flags --enum-name-style=PascalStyle --enum-values-style=CamelStyle --enum-serialize-style=HyphenStyle --indent-increment=4 --namespace=test/ns  --enum-name=MyCoolEnum "--enum-definition=invalid; begin-some; next; final"

