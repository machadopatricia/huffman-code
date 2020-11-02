#include <stdio.h>
#include <stdlib.h>

//alfabeto de 26 letras + quebra de linha + espaco + eof
#define alfabeto (29)

typedef struct Nodo_trie
{
    struct Nodo_trie * filho[2];
    struct Nodo_trie * pai;
    int flag;
    unsigned char conteudo;
}Nodo_trie;

//Letra armazena em 'tamanho' o valor corresponde ao primeiro de 5 blocos do cabecalho
//proximos 4 blocos sao armazenados em 'codigo'
typedef struct Letra
{
    unsigned int tamanho;
    unsigned int codigo;
}Letra;

//cria novo nodo do tipo trie
//inicializa nulo para todos os filhos
//define flag para não ser um nodo terminal
Nodo_trie * cria_nodo()
{
    Nodo_trie * novo = (Nodo_trie*)malloc(sizeof(Nodo_trie));

    for(int i=0; i<2; i++)
        novo->filho[i] = NULL;

    novo->pai = NULL;
    novo->flag = 0;
    novo->conteudo = 0;

    return novo;
}

//funcao insere letra na trie
void insere(Nodo_trie * r, int tamanho, int codigo, unsigned char simbolo)
{
    //ajusta valor recebido para inserir bit a bit
    unsigned int aux = (codigo >> (tamanho-1)) & 1;

    //se o nodo for terminal, "marca flag" e encerra a função
    if(tamanho == 0)
    {
        //123 pois valor entra na funcao ja com +97
        //excecoes para espaco, quebra de linha e EOF
        if(simbolo >= 123)
        {
            if(simbolo == 123)
                simbolo = ' ';
            else if(simbolo == 124)
                simbolo = '\n';
            else if(simbolo == 125)
                simbolo = 20;
        }

        r->flag = 1;
        r->conteudo = simbolo;
        return;
    }

    //senão, se o nodo apontar para nulo, cria um nodo para a letra
    //e chama recursivo para proxima letra
    else if(r->filho[aux] == NULL)
    {
        r->filho[aux] = cria_nodo();
        r->filho[aux]->pai = r;
    }

    //chama recursivo com tamanho menor, ate chegar ao terminal
    insere(r->filho[aux], tamanho-1, codigo, simbolo);
}

int main(int argc, char *argv[])
{
    //cria ponteiros para arquivos de entrada e saida
    FILE *arquivo_in = fopen(argv[1], "rb+");
    FILE *arquivo_out = fopen(argv[2], "w");

    //excecao para caso caminho/arquivo não existam
    if(arquivo_in == NULL)
    {
        puts("O arquivo não existe!");
        return 0;
    }

    //vetor de Letras vai armazenar informacoes do cabecalho
    Letra *simbolos = (Letra*)malloc(alfabeto * sizeof(Letra));
    unsigned char *buffer_byte = (unsigned char*)malloc(sizeof(unsigned char));

    //percorre todo o cabecalho do arquivo txt de entrada
    for(int i=0; i<alfabeto; i++)
    {
        //simbolos na posicao i guarda o primeiro dos 5 blocos - tamanho
        fread(&simbolos[i].tamanho, sizeof(unsigned char), 1, arquivo_in);

        //sequencia dos proximos quatro freads
        //salva um byte no buffer, armazena no simbolos.codigo e
        //faz shift 8x para a esquerda ja que os bits sao gravados mais a direita
        fread(buffer_byte, sizeof(unsigned char), 1, arquivo_in);
        simbolos[i].codigo = 0 | *buffer_byte;
        simbolos[i].codigo = simbolos[i].codigo << 8;

        fread(buffer_byte, sizeof(unsigned char), 1, arquivo_in);
        simbolos[i].codigo = simbolos[i].codigo | *buffer_byte;
        simbolos[i].codigo = simbolos[i].codigo << 8;

        fread(buffer_byte, sizeof(unsigned char), 1, arquivo_in);
        simbolos[i].codigo = simbolos[i].codigo | *buffer_byte;
        simbolos[i].codigo = simbolos[i].codigo << 8;

        //ultimo fread nao tem necessidade de shift ja que o ultimo byte "entra" no ultimo dos 4 bytes do unsigned int
        fread(buffer_byte, sizeof(unsigned char), 1, arquivo_in);
        simbolos[i].codigo = simbolos[i].codigo | *buffer_byte;

        //faz shift para a direita com relacao ao tamanho
        //para eliminar zeros nao desejados a direita
        simbolos[i].codigo = simbolos[i].codigo >> (32-(simbolos[i].tamanho));
    }

    //imprime codigo de huffman utilizado para a descompactação
    for(int i=0; i<alfabeto; i++)
    {
        if(i < 26)
            printf("%c: ", i+97);
        else if(i == 26)
            printf(" : ");
        else if(i == 27)
            printf("\\n: ");
        else if(i == 28)
            printf("EOF: ");

        for(int j=simbolos[i].tamanho-1; j>=0; j--)
            printf("%d", (simbolos[i].codigo >> j) & 1);

        printf("\n");
    }

    Nodo_trie *r = cria_nodo();

    //insere letra por letra na trie
    for(int i=0; i<alfabeto; i++)
        insere(r, (simbolos[i].tamanho), simbolos[i].codigo, i+97);

    //escrevendo no arquivo de saida descompactado
    unsigned int * v = (unsigned int*)malloc(8 * sizeof(unsigned int));

    //inciializa vetor com zeros
    for(int i=0; i<8; i++)
        v[i] = 0;

    Nodo_trie *r_aux = r;
    int parada = 0;

    for(int i=0; parada != 1; i++)
    {
        //volta a ler byte por byte do arquivo de entrada (parte depois do cabecalho)
        fread(buffer_byte, sizeof(unsigned char), 1, arquivo_in);

        //vetor de 8 posicoes unsigned int roda 8x separando um byte em 8 bits
        //e armazenando cada bit (0 ou 1) em cada posicao
        for(int k=1; k<=8; k++)
            v[k-1] = ((0 | *buffer_byte) >> (8-k)) & 1;

        for(int j=0; j<8; j++)
        {
            //nodo auxiliar da raiz da trie
            r_aux = r_aux->filho[v[j]];

            //se o nodo for terminal, escreve a letra no arquivo de saida
            if(r_aux->flag == 1)
            {
                //se o nodo for terminal e o conteudo for o simbolo que corresponde ao EOF,
                //altera o valor da variavel parada para 1 e para a comparacao
                //isso faz com que na proxima iteracao, o laco for pare de rodar
                if(r_aux->conteudo == 20)
                {
                    parada = 1;
                    break;
                }

                //escrevendo a letra no arquivo de saida
                fwrite(&r_aux->conteudo, sizeof(unsigned char), 1, arquivo_out);
                //continua o laço retornando ao comeco da trie para nova leitura
                r_aux = r;
            }
        }
    }

    //liberando espaco alocado
    fclose(arquivo_in);
    fclose(arquivo_out);
    free(buffer_byte);
    free(r);
    free(r_aux);
    free(v);
    free(simbolos);

    return(0);
}
