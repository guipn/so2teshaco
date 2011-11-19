#include "memory.h"
#include "str.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define INIT_LINE_SIZE 512


/**
 * $name free_strings;
 * $proto void free_strings(char **buffer);
 *
 * Chama free em todos os elementos de buffer, ate encontrar
 * NULL. Nao chama free em buffer.
 */

void free_strings(char **buff)
{
#ifdef debug
    printf("\n(%s): Deleting string buffer %p.\n", __func__, (void *) buff);
#endif
    if (!buff)
	return;

    int i;
    for (i = 0; buff[i]; i++)
    {
#ifdef debug
	printf("\n\t(%s): Deleting %s (%p)...", __func__, buff[i], (void *) buff[i]);
#endif
	free(buff[i]);
    }
    
    free(buff);
}


/**
 * $name string_buffer_length;
 * $proto size_t string_buffer_length(char **buffer);
 *
 * Conta o numero de ponteiros != NULL no array cujo 
 * primeiro elemento eh **buffer. O ultimo elemento do array
 * deve ser NULL (buffer deve apontar para o primeiro elemento de um
 * string buffer valido).
 */

size_t string_buffer_length(char **buffer)
{
    size_t i;

    for (i = 0; buffer[i]; i++);

    return i;
}


/**
 * $name read_line;
 * $proto char *read_line(FILE *stream);
 *
 * Responsavel por ler uma linha de texto de stdin. Retorna
 * o endereco do primeiro caracter da linha lida.
 *
 * Se, ao tentar ler algum caracter, houver erro, mostra uma
 * mensagem e aborta a execucao do programa.
 */

char *read_line(FILE *stream)
{
    char *ret         = _malloc(INIT_LINE_SIZE);
    unsigned int i    = 0, 
		 size = INIT_LINE_SIZE;
    int input         = 0;

    while ((input = fgetc(stream)) != '\n' && input != EOF)
    {
	if (i + 1 >= size)
	{
	    size *= 2;
	    ret = _realloc(ret, size);
	}
	ret[ i++ ] = input;
    }

    ret[i] = '\0';
    return _realloc(ret, i + 1); 
}


/**
 * $name split_on_str;
 * $proto char **split_on_str(char *target, char *separator);
 *
 * Retorna o endereco do endereco do primeiro elemento de uma array
 * de ponteiros para os primeiros caracteres das strings resultantes da
 * quebra de target a cada vez que separator eh encontrado (as ocorrencias
 * nao sao inclusas nos resultados).
 */

char **split_on_str(char *target, char *separator)
{
    char **result = _malloc((strlen(target) + 2) * sizeof *result);
    result[0] = NULL;
    size_t seplen = strlen(separator);

    int outer, i = 0, last = 0;
    for (outer = 0; target[outer]; outer++)
    {
	if (strncmp(target + outer, separator, seplen) == 0)
	{
	    size_t tokenlen = outer - last;
	    result[i] = _malloc(tokenlen + 1);
	    strncpy(result[i], target + last, tokenlen);
	    result[i][tokenlen] = '\0';
	    outer += seplen - 1;
	    last = outer + 1;
	    i++;
	}
    }

    if (target[last] != '\0')
    {
	size_t tokenlen = strlen(target + last);
	result[i] = _malloc(tokenlen + 1);
	strcpy(result[i], target + last);
	result[i][tokenlen] = '\0';
	i++;
    }

    result[i] = NULL;
    return realloc(result, (i + 1) * sizeof *result);
}



/**
 * $name split;
 * $proto char **split(char *line, char *sep);
 *
 * Retorna um ponteiro para o primeiro elemento de uma array
 * de ponteiros para os primeiros elementos de strings. O ultimo
 * elemento do array cujo endereco do primeiro elemento eh retornado
 * tem valor NULL.
 */

char **split(char *line, char *sep)
{
    char **ret = _malloc(((strlen(line) / 2) + 2) * sizeof (char *)), 
	 *each = strtok(line, sep);
    
    unsigned int i = 0;

    while (each)
    {
	unsigned int len = strlen(each);
	ret[i] = _malloc(len + 1);
	strcpy(ret[i], each);
	each = strtok(NULL, sep);
	i++;
    }
    
    ret[i] = NULL;
    return _realloc(ret, (i + 1) * sizeof (char *));
}


/**
 * $name concat;
 * $proto char *concat(unsigned int count, char *first, ...);
 *
 * Concatena as strings cujos primeiros caracteres tem enderecos 
 * fornecidos pelos argumentos a partir do segundo. O primeiro argumento
 * deve informar o numero de strings a serem concatenadas. Se o numero
 * informado for maior que o presente, o comportamento eh indefinido.
 */

char *concat(unsigned int count, char *first, ...)
{
    va_list ap;
    char *ptrs[count + 1];
    unsigned int i = 1;

    ptrs[0] = first;

    va_start(ap, first);

    while (i < count)
	ptrs[i++] = va_arg(ap, char *);

    va_end(ap);

    ptrs[i] = NULL;
    return join(ptrs, "");
}


/**
 * $name join;
 * $proto char *join(char *strs[], char *glue);
 *
 * Retorna o endereco do primeiro caracter da string resultante da
 * concatenacao de todas as strings cujos primeiros caracteres tem
 * seus enderecos contidos em strs. O ultimo elemento de strs deve ser
 * NULL. A string armazenada em glue eh inserida entre cada 2 strings de
 * strs.
 */

char *join(char *strs[], char *glue)
{
    if (!strs || !*strs)
	return NULL;

    char *result = NULL;
    
    size_t gluelen = strlen(glue),
    lenres = 0,
    offset = 0,
    i;

    for (i = 0; strs[i]; i++)
    {
	size_t newstrlen = strlen(strs[i]);

	lenres += newstrlen + gluelen + 1;
	result = _realloc(result, lenres);

	strcpy(result + offset, strs[i]);

	offset += newstrlen;
	strcpy(result + offset, glue);

	offset += gluelen;
    }

    result = _realloc(result, lenres - gluelen);
    result[ offset - gluelen ] = '\0';

    return result;
}


/**
 * $name parse_PATH;
 * $proto char **parse_PATH(void);
 *
 * Retorna o endereço do primeiro elemento de uma array de 
 * strings, cada uma representando uma entrada da variavel de
 * ambiente PATH.
 */

char **parse_PATH(void)
{
   return split(getenv("PATH"), ":");
}


/**
 * $name get_command_tokens;
 * $proto char **get_command_tokens(char *cmd, int *foreground);
 *
 * Dado um unico comando, determina se ele deve ser rodado em background
 * ou foreground, armazenando o resultado da avaliacao em <em>foreground</em>
 * e retornando cada 'token' como um elemento de um array cujo ultimo elemento
 * eh NULL. Exemplo:
 *
 * int fg;
 * char **tokens = get_command_tokens("&wget http://www.google.com", &fg);
 *
 * O exemplo acima retorna { "wget", "http://www.google.com" }, e atribui 1 a fg.
 */

char **get_command_tokens(char *cmd, int *foreground)
{	
    char **ret = NULL;

    if (*cmd == '&')
    {
	*foreground = 0;
	ret         = split(cmd + 1, " ");
    }
    else
    {
	*foreground = 1;
	ret         = split(cmd, " ");
    }

    return ret;
}

