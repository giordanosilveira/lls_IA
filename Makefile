# Nome do executável
TARGET = reverse_sat

# Compilador
CC = gcc

# Flags do compilador
CFLAGS = -Wall -Wextra -g

# Arquivos fonte
SRC = main.c

# Arquivos objeto (gerados a partir dos arquivos fonte)
OBJ = $(SRC:.c=.o)

# Regra principal: cria o executável
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Regra para compilar arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
