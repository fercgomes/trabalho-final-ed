/*
	CONVERSOR DE TEXTO PARA CODIGO MORSE
	TRABALHO FINAL DA DISCIPLINA DE ESTRUTURA DE DADOS
	AUTORES:
		Fernando Correa Gomes (00274317)
		Daniel de Souza Novaes (00290193)

	UFRGS 2017
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <wctype.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>

#include "bst.h"
#include "splay.h"
#include "draw.c"

#define MORSEMAX 10
#define BUFFERSIZE 256

/* Define qual interface de linha de comando sera usada */
#define SUPER_CMD

/* Para escolha de implementacao */
#define ABP 0 
#define SPLAY 1

/* Flag para debugger */
static int debug_flag = 0;

/* Constroi uma ABP ou SPLAY a partir de um arquivo de texto com a tabela Morse */
tNode* tree_constructor(const char* filename, int implem_flag){
	FILE* table;
	char line[BUFFERSIZE];
	char *word, *string_alloc, *temp;
	
	int ascii, i;
	size_t morse_size;
	
	/* Ponteiro para funcao que insere elementos na arvore */
	tNode* (*insert) (tNode*, int, char*);
	
	/* Se implementacao eh ABP, seta funcao que insere */
	if(implem_flag == ABP) insert = &BST_insert;
	else insert = &SPLAY_insert;

	/* Inicializa a arvore */
	tNode* newTree = BST_create();

	table = fopen(filename, "r");  
	if(table == NULL){
		fprintf(stderr, "Erro ao abrir o arquivo %s.", filename);
		return NULL; // Retorna uma arvore vazia
	} else {
		/* Abriu o arquivo com a tabela morse */
		/* Busca uma linha de texto */
		while(fgets(line, BUFFERSIZE, table)){
			/* Primeiro token: ascii */
			word = strtok(line, "\n\r\t");		// Garante portabilidade para LF e CR+LF
			ascii = word[0];	

			/* Segundo token: string com o codigo Morse */
			word = strtok(NULL, "\n\r\t");
			
			/* Aloca espaco para string e copia string do arquivo */
			/* OBS:
				O espaco alocado para a string que representa o
				codigo Morse fica alocado independentemente 
				e nao depende da alocacao da arvore em si.
			*/

			/* Calcula tamanho necessario para string com o codigo Morse.
			   Caso a funcao nao retorne um valor valido, usa MORSEMAX como
			   default. Adiciona espaco para \0 tambem.*/
			morse_size = strlen(word);
			if(!morse_size) morse_size = MORSEMAX;
			else morse_size++;

			/* A string fica alocada dinamicamente. O ponteiro eh salvo na arvore */
			string_alloc = (char*)malloc(morse_size * sizeof(char));
			strcpy(string_alloc, word);	
	
			/* Insere caracter na arvore */
			/* OBS:
				A funcao insere depende da implementacao
				escolhida.
			*/
			newTree = insert(newTree, ascii, string_alloc);	

			/* Se debug_flag = 1, salva o estado da arvore em um arquivo de texto */
			if(debug_flag) save_tree_state(newTree, ascii, string_alloc, 0);
		}	
		fclose(table);
		return newTree;
	}
}

/* Converter caracters acentuados para equivalentes sem acentos.
   Retorna todos maiusculos. */
char wchar_to_char(wchar_t character) {
	/* Strings mapeadas */
	const wchar_t* origin =    L"ÁÀÃÂÄÉÈÊËÍÌÎĨÏÓÒÔÕÖÚÙÛŨÜÇÑ";
	const char* target =           "AAAAAEEEEIIIIIOOOOOUUUUUCN";
	int i = 0;

	/* Todos os caracteres retornados sao maiusculos */
	character = towupper(character);

	/* Busca caracter na string de origem (com acentos) */
	while( origin[i] != 0 && character != origin[i] )
		i++;

	/* Se encontrou um caracter correspondente */
	if( origin[i] != 0 )
		/* Busca o caracter sem acento correspondente */
		character = target[i];

	return character;
} 

/* Converte um arquivo de texto para codigo Morse.
   Retorna 1 em caso de erro e 0 em caso de sucesso . */
int txtToMorse(const char* morsetable, const char* input_file, const char* output_file, int implem_flag){
	/* Contagem de tempo */
	clock_t start, end, elapsed;
	
	/* Streams de entrada e saida */
	FILE *input_stream, *output_stream;

	int i, white_space_flag = 0;

	/* Contadores de comparacoes e caracteres convertidos */
	int search_count = 0, char_count = 0;

	char normalized_char,	/* Salva caracter normalizado */
	     *morse_found;	/* Salva ponteiro para codigo Morse */

   	wchar_t input_character;	/* Salva long char lido do stream de entrada */

   	/* Constroi a arvore binaria de pesquisa a partir do arquivo de texto com a tabela Morse */
	tNode* morseTable = tree_constructor("TabelaMorse.txt", implem_flag);

	/* Abre arquivo com texto a ser traduzido */
	input_stream = fopen(input_file, "r");
	if (input_stream == NULL){
		fprintf(stderr, "Erro ao abrir o arquivo %s.\n", input_file);
		return 1;
	} else {
		/* Abre arquivo destino */	
		output_stream = fopen(output_file, "w");
		if(!output_stream){
			fprintf(stderr, "Erro ao abrir o arquivo %s.\n", output_file);
			fclose(input_stream);
			return 1;
		} else {
			/* Inicia leitura do arquivo */ 
			start = clock();

            		while(fscanf(input_stream, "%lc", &input_character) == 1){
				/* Recebe um long char do stream de entrada */
				if(input_character == ' '){
					/* Espaco em branco delimita a palavra */
					if(!white_space_flag)
					fprintf(output_stream, "/ ");
					/* Liga a flag que sinaliza que o ultimo caracter era um espaco em branco */
					white_space_flag = 1;
				} else {
					/* Converte caracteres acentuados para todos maiusculos */
					normalized_char = wchar_to_char(input_character);

					/* INSERE NA ARVORE */ 
					/* Busca o ponteiro para a string que contem o codigo Morse
					para a letra informada */
					if (implem_flag == SPLAY)
						morse_found = SPLAY_search(&morseTable, normalized_char, &search_count);
					else
						morse_found = BST_search(morseTable, normalized_char, &search_count);

					/* Salva o estado da arvore apos a insercao, caso a flag de debug estiver ligada */        
					if(debug_flag)
						save_tree_state(morseTable, normalized_char, morse_found, 1);

					/* Se encontrou um codigo Morse, salva no arquivo */
					if(morse_found){
						char_count++;
						fprintf(output_stream, "%s ", morse_found);
					}

					/* Reseta flag de espaco em branco */
					white_space_flag = 0;
				}
            		}
		}

		/* Termina relogio */
		end = clock();
		elapsed = 1000 * (end - start) / (CLOCKS_PER_SEC);

		/* Imprimir comparacoes */
		printf("+--------------------------------------------------------------------------------+\n");
		printf(">> Convertendo %s...\n", input_file);
		printf(">> Arquivo %s gerado com sucesso.\n", output_file);
		printf("+--------------------------------------------------------------------------------+\n");
		printf("* Implementacao utilizada:\t\t\t");
		if(implem_flag) printf("SPLAY\n");
		else printf("ABP\n");
			    printf("* Altura da arvore utilizada:\t\t\t%d\n", BST_height(morseTable));
		printf("* Comparacoes realizadas em consultas:\t\t%d\n", search_count);
		printf("* Caracteres convertidos para Morse:\t\t%d\n", char_count);
		printf("* Tempo gasto no processamento:\t\t\t%ld ms\n", elapsed);
		if(debug_flag) printf("DEBUGGER ativado. Operacoes na arvore salvas em debug.txt\n");
		else printf("DEBBUGER desativado.\n");
		printf("+--------------------------------------------------------------------------------+\n");
    	}

	/* Desaloca a arvore utilizada */
	BST_delete(morseTable);
	/* Fecha os arquivos utilizados */
	fclose(input_stream);
	fclose(output_stream);
	return 0;
}

/* Mensagem de ajuda para o usuario */
int helpMessage(){
	printf("=================================================================================\n");
	printf("* TEXT TO MORSE CONVERTER\n");
	printf("* O objetivo dessa aplicacao e' mostrar a eficiencia de diferentes implementacoes\n");
	printf("* de arvores binarias de pesquisa. Estao atualmente implementadas a ABP, e a arvore Splay.\n");
	printf("* USO DO PROGRAMA:\n");
	printf("* Argumentos obrigatorios:-t -i -o\n");
	printf("* -t: arquivo de texto com a Tabela Morse\n");
	printf("* -i: arquivo de texto que sera convertido para codigo Morse\n");
	printf("* -o: arquivo de texto onde sera salvo o codigo Morse\n");
	printf("* EXEMPLO: >./txtToMorse -t tabela.txt -i entrada.txt -o saida.txt\n");
	printf("* Flags opcionais: -s -d\n");
	printf("* -s: Implementacao em arvore Splay\n");
	printf("* -d: modo debug, salva as operacoes na arvore em um arquivo de texto\n");
	printf("* -h: ajuda\n");
	printf("=================================================================================\n");
	return 0;
}

/* Programa principal */
int main(int argc, char *argv[]){
	setlocale(LC_ALL,""); 

	#ifdef SUPER_CMD
	const char *table_file = NULL,
		   *input_file = NULL,
		   *output_file = NULL;
	#endif

	/* Padrao: ABP */
	int implem = ABP;

	#ifdef SUPER_CMD
	int c;
	/* Busca flags da linha de comando */
	while ((c = getopt(argc, argv, "t:i:o:dsh")) != -1)
		switch (c){
			case 't':
				/* Busca nome do arquivo com a Tabela Morse */
				table_file = optarg;
				break;
			case 'i':
				/* Busca nome do arquivo de origem */
				input_file = optarg;
				break;
			case 'o':
				/* Busca nome do arquivo de destino */
				output_file = optarg;
				break;
			case 'd':
				/* Seta flag de debug */
				debug_flag = 1;
				break;
			case 's':
				/* Seta implementacao para SPLAY */
				implem = SPLAY;
				break;
			case 'h':
				/* Mostra mensagem de ajuda */
				return helpMessage();
		}
		
	/* Nao foram fornecidos arguments na chamada do programa */
	if(argc == 1) return helpMessage();

	/* Se todos os argumentos obrigatorios foram fornecidos, roda o programa */
	if(table_file && input_file && output_file)
		return txtToMorse(table_file, input_file, output_file, implem);
	else {
		printf("* Arquivos obrigatorios nao especificados: ");
		if(!table_file) printf("Tabela Morse ");
		if(!input_file) printf("Arquivo de entrada ");
		if(!output_file) printf("Arquivo de saida ");
		printf("\n");
		printf("* Uso correto:\n<programa> -t tabelamorse.txt -i entrada.txt -o saida.txt\n* Flags opcionais:\n* -s: Implementacao com arvore Splay (Default: ABP)\n* -d: Salva as operacoes em arvore em um arquivo de texto (Default: off)\n* -h: para mais informacoes\n");	// Mudar para stderr
	}

	#else
	/* Caso o codigo de cima nao seja aceito! */
	if(argc != 4){
		fprintf(stderr, "* Numero incorreto de argumentos.\nUSO: <programa> tabelamorse.txt entrada.txt saida.txt\n");
		return 1;
	} else return txtToMorse(argv[1], argv[2], argv[3], implem);
	#endif
}
