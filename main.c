#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Definição de constantes usadas no código
#define NUMBER_OF_LINE_TO_UNSAT 14  // Linha na saída que indica "Unsatisfiable"
#define NUMBER_OF_LINE_TO_RESULT 15 // Linha onde começam os resultados processados

// Função para montar o caminho para o arquivo de padrão baseado no nome fornecido
void get_path_pattern(char *src_board, char *path) {
    sprintf(src_board, "patterns/%s", path); // Concatena o diretório "patterns/" ao nome do arquivo
}

// Função para montar o caminho para o arquivo do resolvedor SAT
void get_path_sat_solver_file(char *src_sat_solver_file, const char *path) {
    sprintf(src_sat_solver_file, "temp/%s", path); // Concatena o diretório "temp/" ao nome do arquivo
}

// Processa os argumentos passados via linha de comando
void get_options(int argc, char *argv[], int *check_board, char *src_board) {
    int opt;
    // Analisa os argumentos fornecidos
    while ((opt = getopt(argc, argv, "ci:")) != -1) {
        switch (opt) {
            case 'i': // Opção para especificar o arquivo de entrada
                get_path_pattern(src_board, optarg); // Monta o caminho para o arquivo
                printf("-i (caminho do arquivo): %s\n", src_board);
                break;
            case 'c': // Ativa o modo de verificação do tabuleiro
                (*check_board) = 1;
                printf("-c (conferir tabuleiro)\n");
                break;
            default: // Mensagem de uso em caso de erro nos argumentos
                fprintf(stderr, "Uso: %s [-i caminho_tabuleiro ]\n", argv[0]);
                return;
        }
    }
}

// Abre um arquivo no modo leitura e verifica possíveis erros
FILE *open_file(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    return file;
}

// Aloca memória para uma matriz bidimensional de inteiros
int **allocate_memory_to_board(int rows, int columns) {
    int **board;

    // Aloca memória para os ponteiros das linhas
    board = malloc(rows * sizeof(int *));
    if (board == NULL) {
        perror("Erro ao alocar memória para os ponteiros das linhas");
        return NULL;
    }

    // Aloca um bloco contínuo de memória para os elementos
    board[0] = malloc(rows * columns * sizeof(int));
    if (board[0] == NULL) {
        perror("Erro ao alocar memória para os elementos");
        free(board);
        return NULL;
    }

    // Ajusta os ponteiros das linhas
    for (int i = 1; i < rows; i++) {
        board[i] = board[0] + i * columns;
    }
    return board;
}

// Imprime uma matriz no formato legível
void print_board(const char *string, int **board, int rows, int columns) {
    fprintf(stdout, "\n%s:\n", string);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            fprintf(stdout, "%d ", board[i][j]);
        }
        fprintf(stdout, "\n");
    }
}

// Lê uma matriz de um arquivo e calcula a quantidade de células vivas
int **read_file(FILE *file, int *qty_one, int *rows, int *columns) {
    if (fscanf(file, "%d %d", rows, columns) != 2) {
        fprintf(stderr, "Erro ao ler as dimensões da matriz\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    int **board = allocate_memory_to_board(*rows, *columns);
    if (!board) {
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Lê os valores da matriz
    for (int i = 0; i < *rows; i++) {
        for (int j = 0; j < *columns; j++) {
            int cell;
            if (fscanf(file, "%d", &cell) != 1) {
                fprintf(stderr, "Erro ao ler os valores da matriz\n");
                free(board[0]);
                free(board);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            if (cell) (*qty_one) += 1; // Incrementa o contador de células vivas
            board[i][j] = cell;
        }
    }

    return board;
}

// Gera um arquivo para o resolvedor SAT com os dados do tabuleiro
FILE *create_sat_solver_file(int **t1_board, char *src_sat_solver_file, int rows, int columns) {
    get_path_sat_solver_file(src_sat_solver_file, "sat_solver_file.txt");

    FILE *sat_solver_file = fopen(src_sat_solver_file, "w");
    if (sat_solver_file == NULL) {
        perror("Erro ao abrir o arquivo para escrita");
        return NULL;
    }

    // Escreve os caracteres '*,' no arquivo
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            fprintf(sat_solver_file, "*");
            if (j < columns - 1) fprintf(sat_solver_file, ",");
        }
        fprintf(sat_solver_file, "\n");
    }

    // Linha em branco
    fprintf(sat_solver_file, "\n");

    // Escreve os valores da matriz no arquivo
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            fprintf(sat_solver_file, "%d", t1_board[i][j]);
            if (j < columns - 1) fprintf(sat_solver_file, ",");
        }
        fprintf(sat_solver_file, "\n");
    }

    return sat_solver_file;
}

int **get_output_result(FILE *popen_result, int *one_qty, int rows, int columns) {
    char popen_buffer[256];
    int rows_result, columns_result;
    int i = 1;

    // Aloca memória para o tabuleiro resultante
    int **board = allocate_memory_to_board(rows, columns);

    // Lê a saída do comando linha por linha
    while (fgets(popen_buffer, sizeof(popen_buffer), popen_result) != NULL) {
        if (i == NUMBER_OF_LINE_TO_UNSAT) {
            // Verifica se o resultado é "Unsatisfiable"
            if (strcmp(popen_buffer, "Unsatisfiable") == 0) {
                free(board[0]);
                free(board);
                fprintf(stderr, "O tabuleiro é UNSAT\n");
                return NULL;
            }
        } else if (i == NUMBER_OF_LINE_TO_RESULT) {
            // Processa as dimensões da nova matriz
            if (sscanf(popen_buffer, "x = %d, y = %d", &columns_result, &rows_result) != 2) {
                free(board[0]);
                free(board);
                fprintf(stderr, "Erro ao processar as dimensões da nova matriz\n");
                return NULL;
            }   
        } else if (i > NUMBER_OF_LINE_TO_RESULT) {
            // Processa as linhas restantes
            for (int i = 1; i < rows_result - 1; i++) {
                if (fgets(popen_buffer, sizeof(popen_buffer), popen_result) == NULL) {
                    perror("Erro ao ler a linha da matriz");
                    free(board[0]);
                    free(board);
                    return NULL;
                }
                
                // Processa cada coluna útil
                for (int j = 0; j < columns_result; j++) {
                    if (popen_buffer[j] == '$' || popen_buffer[j] == '!') {
                        break; // Ignora caracteres especiais
                    }

                    // Ignora a primeira e última coluna
                    if (j == 0 || j == columns_result - 1) continue;
                    
                    int cell_value = popen_buffer[j] == 'b' ? 0 : 1;

                    if (cell_value) (*one_qty) += 1;

                    board[i - 1][j - 1] = cell_value;
                }
                memset(popen_buffer, '\0', sizeof(popen_buffer));
            }
            return board;
        }
        memset(popen_buffer, '\0', sizeof(popen_buffer));
        i++;
    }
    return board;
}

int my_strcmp(char *s1, char *s2) {
    // Compara duas strings e verifica se s1 começa com s2
    int i = 0;
    while (s2[i] != '\0' && s2[i] == s1[i]) {
        i++;
    }
    if (s2[i] == '\0') return 1;
    return 0;
}

int is_sat(FILE *result) {
    // Verifica se o tabuleiro é satisfatório
    int i = 0;
    char popen_buffer[256];
    char *response = fgets(popen_buffer, sizeof(popen_buffer), result);
    while (response != NULL && i != NUMBER_OF_LINE_TO_UNSAT) {
        i++;
        response = fgets(popen_buffer, sizeof(popen_buffer), result);
    }
    if (result == NULL) {
        return 0; 
    }
    if (my_strcmp(popen_buffer, "Unsatisfiable")) return 0;
    return 1;
}

int find_mim_max_limit(int *max_cell_alive, int one_qty_t0) {
    // Determina os limites mínimos e máximos de células vivas
    FILE *popen_result;
    int min = one_qty_t0 / 2, max = one_qty_t0;
    char command_buffer[128];
    sprintf(command_buffer, "./logic-life-search-master/lls temp/sat_solver_file.txt -p \"<=%d\"", min);
    popen_result = popen(command_buffer, "r");
    while (is_sat(popen_result)) {
        pclose(popen_result);
        max = min;
        min = max / 2;
        sprintf(command_buffer, "./logic-life-search-master/lls temp/sat_solver_file.txt -p \"<=%d\"", min);
        popen_result = popen(command_buffer, "r");
    }
    (*max_cell_alive) = max;
    return min;
}

int** __minimize(int *min_cell_alive, int max_cell_alive, int rows, int columns) {
    // Minimize a quantidade de células vivas no tabuleiro
    FILE *popen_result;
    int min, max = max_cell_alive, lixo;
    int last_min_value = *min_cell_alive;
    int **board;

    min = (*min_cell_alive) + max;
    if (min % 2 == 1) min += 1;
    min /= 2;

    char command_buffer[128];
    sprintf(command_buffer, "./logic-life-search-master/lls temp/sat_solver_file.txt -p \"<=%d\"", min);
    while (max > min) {
        popen_result = popen(command_buffer, "r");

        if (is_sat(popen_result)) {
            max = min;
            min = last_min_value + min;
            if (min % 2 == 1) min += 1;
            min /= 2;
        } else {
            last_min_value = min;
            min = max + min;
            if (min % 2 == 1) min += 1;
            min /= 2;
        }
        pclose(popen_result);
        sprintf(command_buffer, "./logic-life-search-master/lls temp/sat_solver_file.txt -p \"<=%d\"", min);
    }
    fprintf(stderr, "\nValor mínimo encontrado - %d\n", min);
    sprintf(command_buffer, "./logic-life-search-master/lls temp/sat_solver_file.txt -p \"<=%d\"", min);
    popen_result = popen(command_buffer, "r");
    board = get_output_result(popen_result, &lixo, rows, columns);
    pclose(popen_result);
    (*min_cell_alive) = min;
    return board;
}

int **minimize_t0_board(int *one_min_qty_t0, int one_qty_t0, int rows, int columns) {
    // Reduz o número de células vivas no tabuleiro inicial
    int **board;
    int min_cell_alive, max_cell_alive;

    min_cell_alive = find_mim_max_limit(&max_cell_alive, one_qty_t0);
    fprintf(stderr, "\nLimite mínimo %d - Limite máximo %d\n", min_cell_alive, max_cell_alive);
    board = __minimize(&min_cell_alive, max_cell_alive, rows, columns);
    (*one_min_qty_t0) = min_cell_alive;
    return board;
}

int count_neighbors(int **t0, int rows, int columns, int x, int y) {
    // Conta o número de vizinhos vivos de uma célula
    int neighbors = 0;
    int dx[] = {-1, -1, -1,  0, 0, 1, 1, 1};
    int dy[] = {-1,  0,  1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && ny >= 0 && nx < rows && ny < columns) {
            neighbors += t0[nx][ny];
        }
    }
    return neighbors;
}

int is_t0_predecessor(int **t0, int **t1, int rows, int columns) {
    // Verifica se o tabuleiro t0 é o predecessor do tabuleiro t1
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            int vizinhosVivos = count_neighbors(t0, rows, columns, i, j);
            
            // Regras do Jogo da Vida
            if (t0[i][j] == 1 && (vizinhosVivos == 2 || vizinhosVivos == 3)) {
                if (t1[i][j] != 1) return 0;
            } else if (t0[i][j] == 0 && vizinhosVivos == 3) {
                if (t1[i][j] != 1) return 0;
            } else {
                if (t1[i][j] != 0) return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[]){

    
    char src_board[64];
    char src_sat_solver_file[64];
    int **t1_board;
    int **t0_board;
    int **t0_min_board;
    int rows, columns, check_board = 0;
    int one_qty_t0 = 0, one_min_qty_t0 = 0, one_qty_t1 = 0;
    time_t inicio, fim;
    double tempoGasto;


    FILE *file_t1_board;
    FILE *popen_result;
    FILE *sat_solver_file;

    get_options(argc, argv, &check_board, src_board);
    file_t1_board = open_file(src_board);
    
    t1_board = read_file(file_t1_board, &one_qty_t1, &rows, &columns);
    print_board("Tabuleiro em t1", t1_board, rows, columns);
    fprintf(stdout, "Quantidade de 1's no tabuleio t1: %d\n", one_qty_t1);
    fclose(file_t1_board);
    
    sat_solver_file = create_sat_solver_file(t1_board, src_sat_solver_file, rows, columns);
    if (! sat_solver_file) {
        free(t1_board[0]);
        free(t1_board);
        exit(EXIT_FAILURE);
    }
    fclose(sat_solver_file);
    popen_result = popen("./logic-life-search-master/lls temp/sat_solver_file.txt", "r");

    t0_board = get_output_result(popen_result, &one_qty_t0, rows, columns);
    if (! t0_board) {
        free(t1_board[0]);
        free(t1_board);
        pclose(popen_result);
    }

    
    print_board("Tabuleiro em t0", t0_board, rows, columns);
    fprintf(stdout, "Quantidade de 1's no tabuleio t0: %d\n", one_qty_t0);
    
    if(check_board) {
        if (is_t0_predecessor(t0_board, t1_board, rows, columns)) fprintf(stdout, "\nT0 é antecessor de t1\n");
        else fprintf(stdout, "\nT0 não é antecessor de t1\n");
    }

    inicio = time(NULL);
    t0_min_board = minimize_t0_board(&one_min_qty_t0, one_qty_t0, rows, columns);
    fim = time(NULL);

    tempoGasto = difftime(fim, inicio);
    
    print_board("Tabuleiro em t0 mínimo", t0_min_board, rows, columns);
    fprintf(stdout, "Quantidade mínima de 1's no tabuleio t0: %d\n", one_min_qty_t0);
    fprintf(stdout, "Tempo em segundos para achar a quantidade mínima: %.6f\n", tempoGasto);
    
    if(check_board) {
        if (is_t0_predecessor(t0_min_board, t1_board, rows, columns)) fprintf(stdout, "\nT0 é o antecessor mínimo de t1\n");
        else fprintf(stdout, "\nT0 não é antecessor de t1\n");
    }

    free(t1_board[0]);
    free(t1_board);
    free(t0_min_board[0]);
    free(t0_min_board);
    free(t0_board[0]);
    free(t0_board);
    pclose(popen_result);


    return 0;
}