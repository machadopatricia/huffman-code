#include <stdio.h>
#include <stdlib.h>

//alfabeto de 26 letras + quebra de linha + espaco + eof
#define alfabeto (29)

typedef struct Nodo_trie
{
    struct Nodo_trie * filho[2];
    struct Nodo_trie * pai;
    int rotulo;
    int freq;
    unsigned char conteudo;
} Nodo_trie;

//o tipo Letra carrega as informações do cabecalho
//um tamanho (primeiro byte do bloco) e um codigo (ultimos 4 bytes do bloco)
typedef struct Letra
{
    unsigned int tamanho;
    unsigned char *codigo_bin;
} Letra;

//o tipo Frequencia armazena no freq a quantidade de vezes que uma letra aparece
typedef struct Frequencia
{
    int freq;
    Nodo_trie *ptr;
} Frequencia;

//proximas 4 funcoes (sift_up, push, sift_down e pop)
//são todas referentes as funcoes de uma min-heap
void sift_up(int k, Frequencia *heap_freq)
{
    //enquanto k não é a raiz da arvore
    //e for menor que o pai
    while(k>1 && heap_freq[k].freq < heap_freq[k/2].freq)
    {
        //troca com o pai
        int aux1 = heap_freq[k].freq;
        Nodo_trie *aux2 = heap_freq[k].ptr;

        heap_freq[k].freq = heap_freq[k/2].freq;
        heap_freq[k].ptr = heap_freq[k/2].ptr;

        heap_freq[k/2].freq = aux1;
        heap_freq[k/2].ptr = aux2;

        k = k/2;
    }
}

void push(int *N, int x, Nodo_trie *pai, Frequencia *heap_freq)
{
    //insere valor da frequencia do pai (soma da frequencia dos filhos)
    //e arruma ponteiros
    heap_freq[++(*N)].freq = x;
    heap_freq[*N].ptr = pai;

    sift_up(*N, heap_freq);
}

void sift_down(int *N, int k, Frequencia *heap_freq)
{
    int filho_dir = (2*k) + 1;
    int filho_esq = 2*k;

    //enquanto k tem filhos
    //só checa o direito pois obrigatoriamente quando tiver o filho direito significa q tambem tem o esquerdo
    while((filho_dir <= (*N))
            //e heap[k] for maior que algum
            && (heap_freq[k].freq > heap_freq[filho_dir].freq || heap_freq[k].freq > heap_freq[filho_esq].freq))
    {
        int L = 0;

        //L é o menor filho de heap[k]
        if(heap_freq[filho_esq].freq < heap_freq[filho_dir].freq)
            L = filho_esq;
        else
            L = filho_dir;

        //troca
        int aux1 = heap_freq[k].freq;
        Nodo_trie *aux2 = heap_freq[k].ptr;

        heap_freq[k].freq = heap_freq[L].freq;
        heap_freq[k].ptr = heap_freq[L].ptr;

        heap_freq[L].freq = aux1;
        heap_freq[L].ptr = aux2;

        //atualiza posições
        k=L;
        filho_dir = (2*k) + 1;
        filho_esq = 2*k;
    }

    //quando a condicao acima nao for satisfeita, a heap ainda pode ter o filho esquerdo
    //entao checa se o a heap[filho esquerdo] eh menor, se for, troca
    if(filho_esq <= (*N) && (heap_freq[k].freq > heap_freq[filho_esq].freq))
    {
        int L = 0;

        //L é o menor filho de heap[k]
        L = filho_esq;

        //troca
        int aux1 = heap_freq[k].freq;
        Nodo_trie *aux2 = heap_freq[k].ptr;

        heap_freq[k].freq = heap_freq[L].freq;
        heap_freq[k].ptr = heap_freq[L].ptr;

        heap_freq[L].freq = aux1;
        heap_freq[L].ptr = aux2;
    }
}

Nodo_trie *pop(int *N, Frequencia *heap_freq)
{
    //troca a raiz com o ultimo elemento da folha e chama sift_down para arrumar maxheap
    int aux1 = heap_freq[1].freq;
    Nodo_trie *aux2 = heap_freq[1].ptr;

    heap_freq[1].freq = heap_freq[*N].freq;
    heap_freq[1].ptr = heap_freq[*N].ptr;

    heap_freq[*N].freq = aux1;
    heap_freq[*N].ptr = aux2;

    (*N)--;

    sift_down(N, 1, heap_freq);

    return aux2;
}

//transforma vetor qualquer em uma min-heap
void heapfy(int *N, Frequencia * heap_freq)
{
    for(int i=(*N); i>=1; i--)
        sift_down(N, i, heap_freq);
}

//cria novo nodo do tipo trie
//inicializa nulo para todos os filhos
Nodo_trie *cria_nodo()
{
    Nodo_trie *novo = (Nodo_trie*)malloc(sizeof(Nodo_trie));

    for(int i=0; i<2; i++)
        novo->filho[i] = NULL;

    novo->pai = NULL;
    novo->rotulo = 0;
    novo->conteudo = 0;
    novo->freq = 0;

    return novo;
}

//cria pais e atualiza ponteiros
//nodo1 e nodo2 são os 2 nodos que, no momento, tem menor frequencia
Nodo_trie *insere_pai(Nodo_trie *nodo1, Nodo_trie *nodo2)
{
    Nodo_trie *novo_pai = cria_nodo();

    //nodos recebem um pai
    nodo1->pai = novo_pai;
    nodo2->pai = novo_pai;

    //da rotulo os filhos
    nodo1->rotulo = 0;
    nodo2->rotulo = 1;

    //pai recebe os filhos
    novo_pai->filho[0] = nodo1;
    novo_pai->filho[1] = nodo2;

    //atualiza a frequencia do pai para ser a soma da frequencia dos filhos
    //unicos nodos com conteudo são os terminais (os 29 caracteres)
    novo_pai->freq = (nodo1->freq) + (nodo2->freq);
    novo_pai->conteudo = 0;

    return novo_pai;
}

//na "ida" da recursao vai do nodo terminal ate a raiz, trocando para os pais
//quando chega no caso base, o tamanho da letra eh a altura em que o nodo terminal estava
//preenche vetor com os codigos de huffman na volta da recursao
void salva_letras(Nodo_trie *raiz, Letra *letras, int i, int cont)
{
    if(raiz->pai == NULL)
    {
        letras[i].codigo_bin = (unsigned char*)malloc(cont * sizeof(unsigned char));
        letras[i].tamanho = cont;
        cont = 0;

        return;
    }

    salva_letras(raiz->pai, letras, i, cont+1);
    letras[i].codigo_bin[cont] = raiz->rotulo;
    cont++;
}

//percorre toda a trie e quando um nodo for terminal,
//chama a salva_letras para inserir o codigo de huffman num vetor
void percorre_trie(Nodo_trie *raiz, Letra *letras)
{
    if(raiz == NULL)
        return;

    if(raiz->conteudo != 0)
    {
        int i = 0;
        int cont = 0;

        if(raiz->conteudo == ' ')
            i = 26;
        else if(raiz->conteudo == '\n')
            i = 27;
        else if(raiz->conteudo == 20)
            i = 28;
        else
            i = raiz->conteudo - 97;

        salva_letras(raiz, letras, i, cont);
    }

    for(int i=0; i<2; i++)
    {
        if(raiz->filho[i] != NULL)
            percorre_trie(raiz->filho[i], letras);
    }
}

int main(int argc, char *argv[])
{
    FILE *arquivo_in = fopen(argv[1], "r");

    //excecao para caminho/arquivo errado
    if(arquivo_in == NULL)
    {
        puts("O arquivo não existe!");
        return 0;
    }

    //cria vetor para armazenar as frequencias
    int *vetor_frequencias = (int*)malloc(alfabeto * sizeof(int));

    //inicializando nodos
    //todo arquivo contem somente 1 EOF (posicao 28)
    for(int i=0; i<29; i++)
        vetor_frequencias[i] = 0;
    vetor_frequencias[28] = 1;

    int tam = 0;
    for(tam; fscanf(arquivo_in, "%*c")!=EOF; tam++);
    fseek(arquivo_in, 0, SEEK_SET);

    unsigned char *aux_byte = (unsigned char*)malloc(sizeof(unsigned char));

    //DEFININDO FREQUENCIAS
    //variavel char auxiliar armazena byte por byte do arquivo
    //checa qual é o caractere e incrementa a frequencia dele
    for(int i=0; i<tam; i++)
    {
        fread(aux_byte, sizeof(unsigned char), 1, arquivo_in);

        if(*aux_byte == ' ')
            vetor_frequencias[26] = vetor_frequencias[26] + 1;
        else if(*aux_byte == '\n')
            vetor_frequencias[27] = vetor_frequencias[27] + 1;
        else
            vetor_frequencias[*aux_byte-97] = vetor_frequencias[*aux_byte-97] + 1;
    }

    //imprime as frequencias de cada caractere
    for(int i=0; i<26; i++)
        printf("%c: %d\n", i+97, vetor_frequencias[i]);
    printf(" : %d\n", vetor_frequencias[26]);
    printf("\\n: %d\n", vetor_frequencias[27]);
    printf("EOF: %d\n---\n", vetor_frequencias[28]);

    //criando a trie e a heap
    Nodo_trie **nodo = (Nodo_trie**)malloc(alfabeto * sizeof(Nodo_trie*));
    Frequencia * heap_freq = (Frequencia*)malloc((alfabeto + 1) * sizeof(Frequencia));

    //heap é 1 indexada
    heap_freq[0].freq = -1;

    //ponteiro pra N armazena o tamanho da heap - dinamico
    int *N = (int*)malloc(sizeof(int));
    *N = alfabeto;

    //para todas as posicoes do alfabeto
    for(int i=0; i<alfabeto; i++)
    {
        //aloca nodos e atribui as frequencias de cada caractere
        nodo[i] = cria_nodo();
        nodo[i]->freq = vetor_frequencias[i];

        //atualiza os conteudos para representar caracteres correspondentes
        nodo[i]->conteudo = i+97;

        //copia frequencias para freq
        heap_freq[i+1].freq = vetor_frequencias[i];
        //ptr armazena endereco para nodo, assim a relacao entre frequencia e caractere nao eh perdida
        heap_freq[i+1].ptr = nodo[i];
    }

    //conteudos de espaco, quebra de linha e EOF sao corrigidos para os caracteres corretos fora do loop
    nodo[26]->conteudo = ' ';//32
    nodo[27]->conteudo = '\n';//10
    nodo[28]->conteudo = 20;

    //'transforma' vetor de frequencias numa heap
    heapfy(N, heap_freq);

    //pai eh um nodo auxiliar que recebe os enderecos da funcao insere_pai
    Nodo_trie *pai = cria_nodo();

    //enquanto a heap possuir 2 valores para receber pai, insere os pais na heap
    while((*N) > 1)
    {
        pai = insere_pai(pop(N, heap_freq), pop(N, heap_freq));
        push(N, pai->freq, pai, heap_freq);
    }

    //percorre a trie salvando os codigos de huffman de cada letra num vetor de letras
    Letra *letras = (Letra*)malloc(alfabeto * sizeof(Letra));
    percorre_trie(pai, letras);

    //inverte os codigos binarios para serem livres de prefixo
    //armazena o codigo binario numa variavel auxiliar e depois
    //percorre a variavel auxiliar ao contrario, reescrevendo o conteudo original
    for(int i=0; i<alfabeto; i++)
    {
        char *inverte_aux = (char*)malloc(letras[i].tamanho * sizeof(char));

        for(int j=0; j<letras[i].tamanho; j++)
            inverte_aux[j] = letras[i].codigo_bin[j];

        int k = 0;
        for(int j=letras[i].tamanho-1; j>=0; j--)
        {
            letras[i].codigo_bin[k] = inverte_aux[j];
            k++;
        }

        free(inverte_aux);
    }

    //imprime o codigo de cada letra na saida padrao
    for(int i=0; i<alfabeto; i++)
    {
        if(i == 26)
            printf(" : ");
        else if(i == 27)
            printf("\\n: ");
        else if(i == 28)
            printf("EOF: ");
        else
            printf("%c: ", i+97);

        for(int j=0; j<letras[i].tamanho; j++)
            printf("%d", letras[i].codigo_bin[j]);

        printf("\n");
    }

    FILE *arquivo_out = fopen(argv[2], "w");

    //comeca a escrever no arquivo o cabecalho
    for(int i=0; i<alfabeto; i++)
    {
        unsigned int guarda_int = 0;
        unsigned int guarda_bit = 0;

        //escreve o tamanho no primeiro dos cinco blocos de cada letra
        fwrite(&letras[i].tamanho, sizeof(unsigned char), 1, arquivo_out);

        //guarda_bit'eh um inteiro que armazena 1 ou 0 na posicao correta entre os 32 bits
        //quando laco abacar, guarda_int tera todos os bits do codigo da letra com zeros a direita
        for(int j=0; j<letras[i].tamanho; j++)
        {
            guarda_bit = (0 | letras[i].codigo_bin[j]) << (31-j);
            guarda_int = guarda_int | guarda_bit;
        }

        //segundo bloco armazena o primeiro bloco de guarda_int
        *aux_byte = guarda_int >> 24;
        fwrite(aux_byte, sizeof(unsigned char), 1, arquivo_out);

        //terceiro bloco armazena o segundo bloco de guarda_int
        *aux_byte = guarda_int >> 16;
        fwrite(aux_byte, sizeof(unsigned char), 1, arquivo_out);

        //quarto bloco armazena o terceiro bloco de guarda_int
        *aux_byte = guarda_int >> 8;
        fwrite(aux_byte, sizeof(unsigned char), 1, arquivo_out);

        //quinto bloco armazena o ultimo bloco de guarda_int
        *aux_byte = guarda_int;
        fwrite(aux_byte, sizeof(unsigned char), 1, arquivo_out);
    }

    int cont = 0;
    unsigned char *buffer = (unsigned char*)malloc(sizeof(unsigned char));
    *buffer = 0;

    //volta o ponteiro do arquivo de entrada para o comeco
    fseek(arquivo_in, 0, SEEK_SET);

    //percorre o aquivo de entrada todo para imprimir cada letra compactada no arquivo de saida
    for(int i=0; i<tam; i++)
    {
        //aux_byte armazena um caractere por vez do arquivo de entrada
        fread(aux_byte, sizeof(unsigned char), 1, arquivo_in);

        //se o caractere for um espaço
        if(*aux_byte == ' ')
        {
            //roda o loop quantas vezes o tamanho do caractere tiver
            for(int j=0; j<letras[26].tamanho; j++)
            {
                //contador incrementa bits ate chegar a 1 byte
                cont++;

                //se o contador for menor que 8, o buffer ainda nao esta cheio
                //entao insere o bit e insere uma posicao a direita
                if(cont < 8)
                {
                    *buffer = *buffer | (letras[26].codigo_bin[j]);
                    *buffer = *buffer << 1;
                }
                //se o contador estiver cheio
                //escreve no arquivo e reinicia buffer e contador
                else
                {
                    *buffer = *buffer | (letras[26].codigo_bin[j]);
                    fwrite(buffer, sizeof(unsigned char), 1, arquivo_out);
                    *buffer = 0;
                    cont = 0;
                }
            }
        }

        //mesma condicao e passos acima para caso o caractere for uma quebra de linnha
        else if(*aux_byte == '\n')
        {
            for(int j=0; j<letras[27].tamanho; j++)
            {
                cont++;

                if(cont < 8)
                {
                    *buffer = *buffer | (letras[27].codigo_bin[j]);
                    *buffer = *buffer << 1;
                }
                else
                {
                    *buffer = *buffer | (letras[27].codigo_bin[j]);
                    fwrite(buffer, sizeof(unsigned char), 1, arquivo_out);
                    *buffer = 0;
                    cont = 0;
                }
            }
        }
        else
        {
            //mesma condicao acima para se o caractere for uma letra do alfabeto
            for(int j=0; j<letras[(*aux_byte) - 97].tamanho; j++)
            {
                cont++;

                if(cont < 8)
                {
                    *buffer = *buffer | (letras[(*aux_byte)-97].codigo_bin[j]);
                    *buffer = *buffer << 1;
                }
                else
                {
                    *buffer = *buffer | (letras[(*aux_byte)-97].codigo_bin[j]);
                    fwrite(buffer, sizeof(unsigned char), 1, arquivo_out);
                    *buffer = 0;
                    cont = 0;
                }
            }
        }
    }

    //quando a leitura do arquivo acabar, insere o EOF
    for(int j=0; j<=letras[28].tamanho; j++)
    {
        cont++;

        //quando o EOF acabar, escreve no arquivo de saida com zeros a direita
        if(j == letras[28].tamanho)
        {
            *buffer = *buffer << (8-cont);

            //se o buffer estiver vazio, para
            if(cont == 1)
                break;

            fwrite(buffer, sizeof(unsigned char), 1, arquivo_out);
            break;
        }

        //mesmas condicoes e passos do outro for acima
        else if(cont < 8)
        {
            *buffer = *buffer | (letras[28].codigo_bin[j]);
            *buffer = *buffer << 1;
        }
        else if(cont == 8)
        {
            *buffer = *buffer | (letras[28].codigo_bin[j]);
            fwrite(buffer, sizeof(unsigned char), 1, arquivo_out);
            *buffer = 0;
            cont = 0;
        }
    }

    //liberando espaço alocado na memoria
    fclose(arquivo_in);
    fclose(arquivo_out);
    free(vetor_frequencias);
    free(aux_byte);
    free(heap_freq);
    free(pai);
    free(letras);
    free(buffer);
    free(N);

    for(int i=0; i<alfabeto; i++)
        free(nodo[i]);

    free(nodo);

    return(0);
}
