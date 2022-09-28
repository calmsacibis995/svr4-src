/^->/ {
	if(ndef)
		printf("\t0\n};\n\n")
	printf("static char *%s[] = {\n", $2)
	ndef++
	next
}
/^#/ {next}
$0 != "" {printf("\t\"%s\",\n", $0); next}
END {printf("\t0\n};\n")}
