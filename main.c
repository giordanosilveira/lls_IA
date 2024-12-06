#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMBER_OF_LINE_TO_UNSAT 15
#define NUMBER_OF_LINE_TO_RESULT 16

void get_path_pattern(char *src_board, char *path) {
    sprintf(src_board, "patterns/%s", path);
}

void get_path_sat_solver_file(char *src_sat_solver_file, const char *path) {
    sprintf(src_sat_solver_file, "temp/%s", path);
}

void get_options(int argc, char *argv[], char *src_board) {
    int opt;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
            case 'i':
                get_path_pattern(src_board, optarg);
                printf("-i (caminho do arquivo): %s\n", src_board);
                break;
            default: /* '?' */
                fprintf(stderr, "Uso: %s [-i caminho_tabuleiro ]\n", argv[0]);
                return ;
        }
    }

}

FILE *open_file(char *path){
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    return file;

}

int **allocate_memory_to_board(int rows, int columns){
    
    int **board;

    // Aloca o vetor de ponteiros para as linhas
    board = malloc(rows * sizeof(int *));
    if (board == NULL) {
        perror("Erro ao alocar memória para os ponteiros das linhas");
        return NULL;
    }

    // aloca um vetor com todos os elementos da matriz
    board[0] = malloc (rows * columns * sizeof (int));
    if (board[0] == NULL) {
        perror("Erro ao alocar memória para os ponteiros das colunas");
        free(board);
        return NULL;
    }

    // Ajusta os ponteiros das linhas
    for (int i = 1; i < rows; i++) {
        board[i] = board[0] + i * columns;
    }

    return board;
}

void print_board(int **board, int rows, int columns) {

    fprintf(stdout, "\nTabuleiro:\n");
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j){
            fprintf(stdout, "%d ", board[i][j]);
        }
        fprintf(stdout, "\n");
    }

}

int **read_file(FILE *file, int *rows, int *columns) {
    
    // Lê as dimensões da matriz
    if (fscanf(file, "%d %d", rows, columns) != 2) {
        fprintf(stderr, "Erro ao ler as dimensões da matriz\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    int **board = allocate_memory_to_board(*rows, *columns);
    if (! board) {
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Lê os valores do arquivo e os armazena no bloco alocado
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
            board[i][j] = cell;
        }
    }

    return board;
}

FILE *create_sat_solver_file(int **t1_board, char *src_sat_solver_file, int rows, int columns) {
    get_path_sat_solver_file(src_sat_solver_file, "sat_solver_file.txt");

    FILE *sat_solver_file = fopen(src_sat_solver_file, "w");
    if (sat_solver_file == NULL) {
        perror("Erro ao abrir o arquivo para escrita");
        return NULL;
    }

    // Escrevendo os caracteres '*,' no arquivo
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            fprintf(sat_solver_file, "*");
            if (j < columns - 1) { // Adiciona ',' apenas entre os elementos
                fprintf(sat_solver_file, ",");
            }
        }
        fprintf(sat_solver_file, "\n"); // Pula para a próxima linha
    }

    // Linha em branco
    fprintf(sat_solver_file, "\n");

    // Escrevendo os valores da matriz no arquivo
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            fprintf(sat_solver_file, "%d", t1_board[i][j]);
            if (j < columns - 1) { // Adiciona ',' apenas entre os elementos
                fprintf(sat_solver_file, ",");
            }
        }
        fprintf(sat_solver_file, "\n"); // Pula para a próxima linha
    }

    return sat_solver_file;
    
}

int **get_output_result(FILE *popen_result, int rows, int columns) {
    char popen_buffer[256];

    int i = 1;
    // Lê a saída do comando linha por linha
    while (fgets(popen_buffer, sizeof(popen_buffer), popen_result) != NULL) {
        if (i == NUMBER_OF_LINE_TO_UNSAT) {
            if (strcmp(popen_buffer, "Unsatisfiable") == 0) {
                fprintf(stderr, "O tabuleiro é UNSAT\n");
                return NULL;
            }
        }

        if (i == NUMBER_OF_LINE_TO_RESULT) {

        }

        printf("%s", buffer); // Imprime cada linha
    }
    int **board = allocate_memory_to_board(rows, columns);

}

int main(int argc, char *argv[]){

    
    char src_board[64];
    char src_sat_solver_file[64];
    int **t1_board;
    int **t0_board;
    int rows, columns;
    FILE *file_t1_board;
    FILE *popen_result;
    FILE *sat_solver_file;

    get_options(argc, argv, src_board);
    file_t1_board = open_file(src_board);
    
    t1_board = read_file(file_t1_board, &rows, &columns);
    print_board(t1_board, rows, columns);
    fclose(file_t1_board);
    
    sat_solver_file = create_sat_solver_file(t1_board, src_sat_solver_file, rows, columns);
    if (! sat_solver_file) {
        free(t1_board[0]);
        free(t1_board);
        exit(EXIT_FAILURE);
    }
    fclose(file_t1_board);
    popen_result = popen("./logic-life-search/lls temp/sat_solver_file.txt -p", "r");

    t0_board = get_output_result(popen_result);


    pclose(popen_result);


    return 0;
}