/* tab2.h */
#ifndef  TAB2_H
#define  TAB2_H

#include <gtk/gtk.h>

void set_table_info(GPtrArray *results);
GtkWidget* tab2();

void clear_table_mine(const int size, GtkTreeStore *store);
#endif   

