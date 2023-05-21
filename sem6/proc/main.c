#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>

#define MAX_THREADS 10

int num_threads = 1;
int *array;
int array_len = 0;
int threads_initialized = 0;

pthread_t threads[MAX_THREADS];
int thread_ids[MAX_THREADS];

int thread_wait = 1;

void *selection_sort(void *arg) {
    int thread_id = *(int *)arg;

    while (thread_wait) { }

    int start = thread_id * (array_len / num_threads);
    int end = start + (array_len / num_threads);

    if (thread_id == num_threads - 1) {
        end = array_len;
    }

    for (int i = start; i < end - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < end; j++) {
            if (array[j] < array[min_idx]) {
                min_idx = j;
            }
        }
        int temp = array[i];
        array[i] = array[min_idx];
        array[min_idx] = temp;
    }


    return NULL;
}

void create_threads() {
    thread_wait = 1;
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, selection_sort, &thread_ids[i]);
    }
}

void show_info(int start, int mid, int end) {
    printf("start mid end: %d, %d, %d\n", start, mid, end);
    printf("array: ");
    for (int i = 0; i < array_len; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(int* arr, int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
 
    /* create temp arrays */
    int L[n1], R[n2];
 
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there
    are any */
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there
    are any */
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void sort_array() {
    thread_wait = 0;

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 1; i < num_threads; i++) {
        int block_size = array_len / num_threads;
        int mid = i * block_size;
        int end = (i + 1) * block_size;

        if (i == num_threads - 1) {
            end = array_len;
        }

        merge(array, 0, mid - 1, end - 1);
    }
}

void on_sort_button_clicked(GtkButton *button, gpointer user_data) {
    create_threads();
    threads_initialized = 1;
}

void on_threads_changed(GtkSpinButton *spin_button, gpointer user_data) {
    num_threads = gtk_spin_button_get_value_as_int(spin_button);
}

void on_array_entry_changed(GtkEditable *editable, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
    gchar **tokens = g_strsplit(text, ",", -1);
    array_len = g_strv_length(tokens);
    array = realloc(array, array_len * sizeof(int));
    for (int i = 0; i < array_len; i++) {
        array[i] = atoi(tokens[i]);
    }
    g_strfreev(tokens);
}

void on_finish_button_clicked(GtkButton *button, gpointer user_data) {
    if (!threads_initialized) {
        return;
    }

    sort_array();
    threads_initialized = 0;

    GtkWidget *array_label = GTK_WIDGET(user_data);
    char *array_str = g_strdup_printf("[%d", array[0]);
    for (int i = 1; i < array_len; i++) {
        array_str = g_strdup_printf("%s, %d", array_str, array[i]);
    }
    array_str = g_strdup_printf("%s]", array_str);
    gtk_label_set_text(GTK_LABEL(array_label), array_str);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Multithreaded Selection Sort");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *array_entry = gtk_entry_new();
    GtkWidget *threads_spin = gtk_spin_button_new_with_range(1, MAX_THREADS, 1);
    GtkWidget *sort_button = gtk_button_new_with_label("Sort");
    GtkWidget *array_label = gtk_label_new(NULL);
    GtkWidget *finish_button = gtk_button_new_with_label("Finish");

    g_signal_connect(array_entry, "changed", G_CALLBACK(on_array_entry_changed), NULL);
    g_signal_connect(threads_spin, "value-changed", G_CALLBACK(on_threads_changed), NULL);
    g_signal_connect(sort_button, "clicked", G_CALLBACK(on_sort_button_clicked), NULL);
    g_signal_connect(finish_button, "clicked", G_CALLBACK(on_finish_button_clicked), array_label);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Array:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), array_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Threads:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), threads_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sort_button, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Sorted Array:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), finish_button, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), array_label, 0, 4, 2, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    free(array);

    return 0;
}
