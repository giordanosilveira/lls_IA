# Descrição Geral
O código implementa um programa para trabalhar com o Jogo da Vida de Conway, utilizando estratégias baseadas em SAT solvers para determinar a geração mínima necessária de células (t0) que produz um estado final dado (t1). Ele lê os tabuleiros a partir de arquivos, gera um arquivo de entrada para o SAT solver e analisa as saídas para verificar condições específicas e otimizar o número de células vivas no tabuleiro inicial.

## Funções Auxiliares de Manipulação de Arquivos e Caminhos

### void get_path_pattern(char *src_board, char *path)
Concatena o caminho base patterns/ com um nome de arquivo fornecido (path), armazenando no buffer src_board.

### void get_path_sat_solver_file(char *src_sat_solver_file, const char *path)
Similar à função anterior, mas concatena com temp/ para gerar o caminho de arquivos temporários.

### void get_options(int argc, char *argv[], int *check_board, char *src_board)
Analisa os argumentos da linha de comando:
-i: Especifica o arquivo de entrada do tabuleiro (src_board).
-c: Ativa a opção de verificação se t0 gera t1 (check_board).

## FILE *open_file(char *path)
Abre um arquivo para leitura.
Termina o programa em caso de erro.

## Funções de Manipulação de Tabuleiros

###  int **allocate_memory_to_board(int rows, int columns)
Aloca memória dinâmica para um tabuleiro de tamanho rows x columns como um bloco único para eficiência.
Retorna um ponteiro para a matriz bidimensional.

### int **read_file(FILE *file, int *qty_one, int *rows, int *columns)
Lê as dimensões (rows, columns) e os valores de um tabuleiro a partir de um arquivo.
Conta o número de células vivas (qty_one).

### void print_board(const char *string, int **board, int rows, int columns)
Imprime o tabuleiro (board) precedido por um título (string).

## Funções para Integração com SAT Solver

### FILE *create_sat_solver_file(int **t1_board, char *src_sat_solver_file, int rows, int columns)
Gera um arquivo temporário contendo:
Um cabeçalho de formatação.
O estado final do tabuleiro t1.
Esse arquivo serve como entrada para o SAT solver.

### int **get_output_result(FILE *popen_result, int *one_qty, int rows, int columns)
Processa a saída do SAT solver:
Verifica se o problema é "unsatisfiable".
Lê as dimensões e o conteúdo do tabuleiro resultante.

### int is_sat(FILE *result)
Verifica se o SAT solver retornou um resultado satisfatório.

## Funções para Otimização de Células Vivas

### int find_mim_max_limit(int *max_cell_alive, int one_qty_t0, int rows, int columns)
Determina os limites máximo (max_cell_alive) e mínimo inicial para o número de células vivas usando busca binária.

### int** __minimize(int *min_cell_alive, int max_cell_alive, int rows, int columns)
Utiliza busca binária refinada para encontrar o menor número de células vivas que ainda gera o estado t1.

### int **minimize_t0_board(int *one_min_qty_t0, int one_qty_t0, int rows, int columns)
Gera um tabuleiro inicial t0 com o número mínimo de células vivas.

## Funções Relacionadas ao Jogo da Vida

### int count_neighbors(int **t0, int rows, int columns, int x, int y)
Conta os vizinhos vivos ao redor da célula (x, y) em t0.

### int is_t0_predecessor(int **t0, int **t1, int rows, int columns)
Verifica se as regras do Jogo da Vida permitem que t0 gere t1.

## Função Principal
### main
    Passos principais:
    Lê os argumentos de entrada e configura o caminho do tabuleiro (get_options).
    Lê o tabuleiro t1 a partir de um arquivo.
    Gera o arquivo de entrada para o SAT solver (create_sat_solver_file).
    Obtém o estado inicial t0 que pode gerar t1 (get_output_result).
    Otimiza t0 para minimizar o número de células vivas (minimize_t0_board).
    Verifica se t0 e t0_min satisfazem as condições para gerar t1.

# Para executar:
1. Execute 'make'.
2. Coloque o arquivo.txt com seu tabuleiro no diretório patterns.
3. ./reverse_sat -c [opcional] -i [arquivo.txt]
    Não é necessário colocar o caminho inteiro. Somente o nome do seu arquivo.
4. pronto

