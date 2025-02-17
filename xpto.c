#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <limits.h>

// MAX char table (ASCII)
#define MAX 256

// Boyers-Moore-Hospool-Sunday algorithm for string matching
// Modificado para aceitar um offset e retornar índices relativos ao offset
int bmhs(char *string, int n, char *substr, int m, int offset) {
    int d[MAX];
    int i, j, k;

    // pre-processing
    for (j = 0; j < MAX; j++)
        d[j] = m + 1;

    for (j = 0; j < m; j++)
        d[(int) substr[j]] = m - j;

    // searching
    i = m - 1;
    while (i < n) {
        k = i;
        j = m - 1;
        while ((j >= 0) && (string[k] == substr[j])) {
            j--;
            k--;
        }
        if (j < 0)
            return offset + k + 1;  // Retorna a posição global
        i = i + d[(int) string[i + 1]];
    }

    return -1;
}

FILE *fdatabase, *fquery, *fout;

void openfiles() {
    fdatabase = fopen("dna.in", "r+");
    if (fdatabase == NULL) {
        perror("dna.in");
        exit(EXIT_FAILURE);
    }

    fquery = fopen("query.in", "r");
    if (fquery == NULL) {
        perror("query.in");
        exit(EXIT_FAILURE);
    }

    fout = fopen("dna.out", "w");
    if (fout == NULL) {
        perror("fout");
        exit(EXIT_FAILURE);
    }
}

void closefiles() {
    fflush(fdatabase);
    fclose(fdatabase);

    fflush(fquery);
    fclose(fquery);

    fflush(fout);
    fclose(fout);
}

void remove_eol(char *line) {
    int i = strlen(line) - 1;
    while (line[i] == '\n' || line[i] == '\r') {
        line[i] = 0;
        i--;
    }
}

char *bases;
char *str;

int main(void) {
    bases = (char*) malloc(sizeof(char) * 1000001);
    if (bases == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    str = (char*) malloc(sizeof(char) * 1000001);
    if (str == NULL) {
        perror("malloc str");
        exit(EXIT_FAILURE);
    }

    openfiles();

    char desc_dna[100], desc_query[100];
    char line[100];
    int i, found, result;

    fgets(desc_query, 100, fquery);
    remove_eol(desc_query);
    while (!feof(fquery)) {
        fprintf(fout, "%s\n", desc_query);
        // read query string
        fgets(line, 100, fquery);
        remove_eol(line);
        str[0] = 0;
        i = 0;
        do {
            strcat(str + i, line);
            if (fgets(line, 100, fquery) == NULL)
                break;
            remove_eol(line);
            i += 80;
        } while (line[0] != '>');
        strcpy(desc_query, line);

        // read database and search
        found = 0;
        fseek(fdatabase, 0, SEEK_SET);
        fgets(line, 100, fdatabase);
        remove_eol(line);
        while (!feof(fdatabase)) {
            strcpy(desc_dna, line);
            bases[0] = 0;
            i = 0;
            fgets(line, 100, fdatabase);
            remove_eol(line);
            do {
                strcat(bases + i, line);
                if (fgets(line, 100, fdatabase) == NULL)
                    break;
                remove_eol(line);
                i += 80;
            } while (line[0] != '>');

            int bases_len = strlen(bases);
            int pattern_len = strlen(str);
            
            int num_threads;
            #pragma omp parallel
            {
                #pragma omp master
                num_threads = omp_get_num_threads();
            }
            
            int global_min_pos = INT_MAX;
            int chunk_size = bases_len / num_threads;
            int overlap = pattern_len - 1;
            
            #pragma omp parallel
            {
                int thread_id = omp_get_thread_num();
                int start_pos = thread_id * chunk_size;
                int end_pos;
                
                // Último thread pega até o final para evitar problemas de arredondamento
                if (thread_id == num_threads - 1) {
                    end_pos = bases_len;
                } else {
                    end_pos = start_pos + chunk_size + overlap;
                    if (end_pos > bases_len)
                        end_pos = bases_len;
                }
                
                int search_len = end_pos - start_pos;
                if (search_len >= pattern_len) {
                    int local_result = bmhs(bases + start_pos, search_len, str, pattern_len, start_pos);
                    if (local_result > 0) {
                        #pragma omp critical
                        {
                            if (local_result < global_min_pos) {
                                global_min_pos = local_result;
                            }
                        }
                    }
                }
            }
            
            if (global_min_pos < INT_MAX) {
                fprintf(fout, "%s\n%d\n", desc_dna, global_min_pos);
                found++;
            }
        }

        if (!found)
            fprintf(fout, "NOT FOUND\n");
    }

    closefiles();

    free(str);
    free(bases);

    return EXIT_SUCCESS;
}
